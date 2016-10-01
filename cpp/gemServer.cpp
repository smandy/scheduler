#include "Gem.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <memory>
#include <unordered_map>

// https://github.com/SintefRaufossManufacturing/icehms/blob/master/icecfg/icebox.xml
//typedef std::shared_ptr<Gem::Job> JobPtr;
// http://git.asterisk.org/gitweb/?p=asterisk-scf/release/techdemo.git;a=commitdiff_plain;h=89b836724135902a7d628cc08bd8ddd786b82240
class GemServerImpl : public Gem::GemServer {
    std::vector<Gem::Job> jobs;

    IceStorm::TopicPrx topic;
    Ice::CommunicatorPtr communicator;
    Gem::GemServerListenerPrx pubToListeners;
    
public:
    GemServerImpl(Ice::CommunicatorPtr _communicator) : jobs(),
                                                        communicator(_communicator) {
        std::cout << "Get Topic" << std::endl;
        auto topicPrx = IceStorm::TopicManagerPrx::checkedCast( communicator->propertyToProxy("IceStorm.TopicManager"));
        std::cout << "Get Subject" << std::endl;
        auto subject = communicator->getProperties()->getProperty("Gem.Topic");
        std::cout << "Subject is " << subject << std::endl;
        try {
            topic = topicPrx->retrieve( subject );
        } catch( IceStorm::NoSuchTopic &) {
            topic = topicPrx->create( subject );
        }
        pubToListeners = Gem::GemServerListenerPrx::uncheckedCast(
                                                                  topic->getPublisher() );
        std::cout << "Done" << std::endl;
    }
    
    inline Gem::Job* find( const std::string& id ) {
        static thread_local Gem::Job finder;
        Gem::Job *ret = nullptr;
        finder.id = id;
        auto x = std::lower_bound( begin(jobs), end(jobs), finder, [&]( auto& a, auto& b) {
                return a.id < b.id;
            });
        if (x != end(jobs) && x->id==id) {
            return &(*x);
        } else {
            return nullptr;
        };
    }

    virtual void getJob_async(const ::Gem::AMD_GemServer_getJobPtr& cb,
                              const ::std::string& id,
                              const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobSeq ret;
        Gem::Job* x = find(id);
        if (x !=nullptr) {
            ret.push_back(*x);
        }
        cb->ice_response(ret);
    }
    
    virtual void submitBatch_async(const ::Gem::AMD_GemServer_submitBatchPtr& cb,
                                   const ::Gem::Batch& batch,
                                   const ::Ice::Current& = ::Ice::Current()) {
        // TODO - duplicate ids.
        std::cout << "Woot" << std::endl;
        jobs.insert(jobs.end(), begin(batch.jobs), end(batch.jobs));
        std::sort( begin(jobs), end(jobs), [&]( auto &a,auto &b) {
                return a.id < b.id;
            });

        pubToListeners->begin_onUpdate( batch.jobs,
                                        []() { std::cout << "Send update to published" << std::endl; } );
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
                                  const ::std::string&, const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "invalidate" << std::endl;
        cb->ice_response();
    }

    virtual void getStartableJob_async(const ::Gem::AMD_GemServer_getStartableJobPtr& cb, const ::Gem::WorkerId&, const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobSeq ret;
        Gem::Job *selected = nullptr;
        for (auto &&job : jobs) {
            if ( job.state == Gem::JobState::STARTABLE ) {
                if (selected == nullptr || job.priority<selected->priority) {
                    selected = &job;
                };
            }
        };
        
        if (selected != nullptr) {
            selected->state = Gem::JobState::STARTED;
            ret.push_back(*selected);
        };
        cb->ice_response( ret );
    }
    
    virtual void getJobs_async(const ::Gem::AMD_GemServer_getJobsPtr& cb, const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response( jobs );
    }

    virtual void addListener_async(const ::Gem::AMD_GemServer_addListenerPtr& cb, const ::Gem::GemServerListenerPrx& prx, const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Add listener " << std::endl;
        static const IceStorm::QoS theQos;

        Gem::Image img { {begin(jobs), end(jobs) }};

        // auto prx2 = prx->ice_timeout(5000);
        // prx->begin_onImage( img ,
        //                     []() { std::cout << "Called directly" << std::endl; },
        //                     [](const Ice::Exception &ex) { std::cout << "Exception" << std::endl; } ,
        //                     [](bool b) { std::cout << "Sent " << b << std::endl; });
        topic->begin_subscribeAndGetPublisher( theQos, prx, [=]( const Ice::ObjectPrx &response) {
                std::cout << " Got publisher " << response << std::endl;
                auto p = Gem::GemServerListenerPrx::uncheckedCast(response);
                std::cout << "Made image " << std::endl;
                p->begin_onImage( img , []() { std::cout << "Success with image" << std::endl; } );
                cb->ice_response();
            } );
    }
};

int main(int argc, char *argv[]) {
    std::cout << "Make communicator" << std::endl;
    auto communicator = Ice::initialize( argc, argv);
    
    std::cout << "Make server" << std::endl;
    auto server = std::make_shared<GemServerImpl>(communicator);
    std::cout << "Make adapter" << std::endl;
    auto adapter = communicator->createObjectAdapter("GemServer");
    std::cout << "Add impl to adapter" << std::endl;
    auto prx = adapter->add( server.get(), communicator->stringToIdentity("server"));
    std::cout << "Activate adpater" << std::endl;
    adapter->activate();
    std::cout << "Wait for shutdown" << std::endl;
    communicator->waitForShutdown();
    std::cout << "Shutdown - exiting" << std::endl;
};
    
