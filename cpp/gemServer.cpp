#include "gem.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <memory>
#include <unordered_map>
#include <iostream>

// Pointers for getting icestorm up
// Aha - the trick was exposing IceStorm/TopicManager as a well known object
// https://github.com/SintefRaufossManufacturing/icehms/blob/master/icecfg/icebox.xml
//typedef std::shared_ptr<Gem::Job> JobPtr;
// http://git.asterisk.org/gitweb/?p=asterisk-scf/release/techdemo.git;a=commitdiff_plain;h=89b836724135902a7d628cc08bd8ddd786b82240

class GemServerImpl : public Gem::GemServer {
    std::vector<Gem::Job> jobs;
    Gem::GemServerListenerPrx pubToListeners;
    IceStorm::TopicPrx topic;
    std::unordered_map<std::string, std::vector<std::string> > dependants;
    
public:
    GemServerImpl(Ice::CommunicatorPtr communicator) {
        std::cout << "Get Topic" << std::endl;
        auto topicPrx = IceStorm::TopicManagerPrx::checkedCast( communicator->propertyToProxy("IceStorm.TopicManager"));
        std::cout << "Get Subject" << std::endl;
        auto subject = communicator->getProperties()->getProperty("Gem.Topic");
        std::cout << "Subject is " << subject << std::endl;
        try {
            topic = topicPrx->retrieve(subject);
        } catch( IceStorm::NoSuchTopic &) {
            topic = topicPrx->create(subject);
        }
        pubToListeners = Gem::GemServerListenerPrx::uncheckedCast(topic->getPublisher());
        std::cout << "Done" << std::endl;
    }

    inline Gem::Job* find(const std::string& id) {
        static thread_local Gem::Job finder;
        Gem::Job *ret = nullptr;
        finder.id = id;
        auto x = std::lower_bound( begin(jobs),
                                   end(jobs),
                                   finder,
                                   []( auto& a, auto& b) {
                                       return a.id < b.id;
                                   });
        if (x != end(jobs) && x->id==id) {
            return &(*x);
        } else {
            throw Gem::JobNotFound(id);
        }
    }

    virtual void getJob_async(const ::Gem::AMD_GemServer_getJobPtr& cb,
                              const ::std::string& id,
                              const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response(*find(id));
    }

    void blockDependenciesOf(const std::string &id ) {
        for ( auto& depId : dependants[id]) {
            auto depJob = find(depId);
            depJob->state = Gem::JobState::BLOCKED;
        }
    }
    
    virtual void submitBatch_async(const ::Gem::AMD_GemServer_submitBatchPtr& cb,
                                   const ::Gem::Batch& batch,
                                   const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Submitbatch" << std::endl;
        for ( auto &job : batch.jobs) {
            if ( !job.dependencies.empty()) {
                for ( auto &depId : job.dependencies) {
                    dependants[depId].push_back( job.id );
                }
            }
        }
        std::cout << "Inserting" << std::endl;
        jobs.insert(jobs.end(), begin(batch.jobs), end(batch.jobs));
        std::cout << "Sorting" << std::endl;
        std::sort(begin(jobs), end(jobs), [&]( auto &a, auto &b) {
                return a.id < b.id;
            });
        for(auto& job : batch.jobs) {
            std::cout << "Looping " << job.id << std::endl;
            blockDependenciesOf( job.id );
        }
        pubToListeners->begin_onUpdate(batch.jobs,
                                        []() { std::cout << "Sent new jobs" << std::endl; } );
        cb->ice_response();
    }
    
    virtual void startJob_async(const ::Gem::AMD_GemServer_startJobPtr& cb,
                                const ::std::string& id,
                                const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "startJob" << std::endl;
        cb->ice_response();
    }
    
    virtual void stopJob_async(const ::Gem::AMD_GemServer_stopJobPtr& cb,
                               const ::std::string&,
                               const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "stopJob" << std::endl;
        cb->ice_response();
    }
    
    virtual void invalidate_async(const ::Gem::AMD_GemServer_invalidatePtr& cb,
                                  const ::std::string&,
                                  const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "invalidate" << std::endl;
        cb->ice_response();
    }

    virtual void reset_async(const ::Gem::AMD_GemServer_resetPtr& cb,
                             const ::Ice::Current& = ::Ice::Current()) {
        jobs.clear();
        dependants.clear();
        cb->ice_response();
    }
    
    virtual void getStartableJob_async(const ::Gem::AMD_GemServer_getStartableJobPtr& cb,
                                       const ::Gem::WorkerId&,
                                       const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobSeq ret;
        Gem::Job *selected = nullptr;
        for (auto &job : jobs) {
            if (job.state == Gem::JobState::STARTABLE ) {
                if (!selected || job.priority < selected->priority) {
                    selected = &job;
                }
            }
        }
        if (selected) {
            selected->state = Gem::JobState::SCHEDULED;
            ret.push_back(*selected);
        }
        cb->ice_response( ret );
    }

    virtual void dumpStatus_async(const ::Gem::AMD_GemServer_dumpStatusPtr& cb,
                                  const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response("Unimplemented");
    }
    
    virtual void getJobs_async(const ::Gem::AMD_GemServer_getJobsPtr& cb,
                               const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response( jobs );
    }

    virtual void onWorkerStates_async(const ::Gem::AMD_GemServer_onWorkerStatesPtr& cb,
                                      const ::Gem::JobWorkerStateSeq& jwss,
                                      const ::Ice::Current& = ::Ice::Current()) {
        std::vector<Gem::Job> updated;
        for(auto &jws : jwss) {
            auto job = find( jws.id );
            if ( job->state != jws.state) {
                job->state = jws.state;
                updated.push_back(*job);
            }
            for ( auto dep : dependants[job->id]) {
                std::cout << "Check startable " << dep << std::endl;
                checkIsStartable( dep, updated);
            }
        }
        
        if (!updated.empty()) {
            pubToListeners->begin_onUpdate( updated ,
                                            []() { std::cout << "Publushed update" << std::endl; } );
        }
        cb->ice_response();
    }

    void checkIsStartable( const std::string& id,
                           std::vector<Gem::Job>& updatedSink) {
        auto job = find( id );
        if (job->state == Gem::JobState::BLOCKED) {
            bool isStartable = true;
            for ( auto& id : job->dependencies) {
                if (causesBlockage(id)) {
                    isStartable = false;
                    break;
                };
            }
            std::cout << "job is " << id << " startable is " << (isStartable ? "true" : "false") << std::endl;
            if (isStartable) {
                job->state = Gem::JobState::STARTABLE;
                updatedSink.push_back( *job);
            }
        }
    }

    bool causesBlockage( const std::string id ) {
        auto job = find(id);
        return job->state != Gem::JobState::WAIVERED &&
            job->state != Gem::JobState::COMPLETED;
    }

    virtual void addListener_async(const ::Gem::AMD_GemServer_addListenerPtr& cb,
                                   const ::Gem::GemServerListenerPrx& prx,
                                   const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Add listener " << std::endl;
        static const IceStorm::QoS theQos;
        std::cout << "Add listener " << std::endl;
        topic->begin_subscribeAndGetPublisher(theQos,
                                              prx,
                                              [this,cb](const Ice::ObjectPrx &response) {
                                                  //std::cout << " Got publisher " << response << std::endl;
                                                  auto p = Gem::GemServerListenerPrx::uncheckedCast(response);
                                                  Gem::Image img { jobs };
                                                  //std::cout << "Made image " << std::endl;
                                                  p->begin_onImage( img ,
                                                                    []() {
                                                                        std::cout << "Success with image" << std::endl;
                                                                    });
                                                  //std::cout << "Called image " << cb << std::endl;
                                                  // Hmmm ... should this be in side the lambda above maybe?
                                                  cb->ice_response();
                                                  //std::cout << "Called response " << std::endl;
                                              } );
        std::cout << "End method" << std::endl;
    }
};

int main(int argc, char *argv[]) {
    std::cout << "Make communicator" << std::endl;
    auto communicator = Ice::initialize(argc, argv);
    std::cout << "Make server" << std::endl;
    auto server = std::make_unique<GemServerImpl>(communicator);
    std::cout << "Make adapter" << std::endl;
    auto adapter = communicator->createObjectAdapter("GemServer");
    std::cout << "Add impl to adapter" << std::endl;
    
    // Liveness ensured by scope of main
    auto prx = adapter->add(server.get(), communicator->stringToIdentity("server"));
    std::cout << "Activate adpater" << std::endl;
    adapter->activate();
    std::cout << "Wait for shutdown" << std::endl;
    communicator->waitForShutdown();
    std::cout << "Shutdown - exiting" << std::endl;
};
    
