#include "Gem.h"
#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <memory>
#include <unordered_map>
#include <stdexcept>

// Pointers for getting icestorm up
// Aha - the trick was exposing IceStorm/TopicManager as a well known object
// https://github.com/SintefRaufossManufacturing/icehms/blob/master/icecfg/icebox.xml
//typedef std::shared_ptr<Gem::Job> JobPtr;
// http://git.asterisk.org/gitweb/?p=asterisk-scf/release/techdemo.git;a=commitdiff_plain;h=89b836724135902a7d628cc08bd8ddd786b82240

class GraphNode;

using GraphNodePtr = std::shared_ptr<GraphNode>;

struct GraphNode {
    const std::string& id;
    
    GraphNode( const std::string &s) : id {s},
                                       dependencies {},
                                       dependents {} {};
    std::vector<GraphNodePtr> dependencies;
    std::vector<GraphNodePtr> dependents;
};

class Graph {
    std::map<std::string, GraphNodePtr> nodes;

public:
    std::string dump() {
        std::ostringstream oss;
        for(auto &kv : nodes) {
            oss << kv.first << std::endl;
            oss << "===========" << std::endl;
            {
                auto sep = "";
                oss << "dependents : ";
                for ( auto dep : kv.second->dependents) {
                    oss << sep << dep->id;
                    sep = ",";
                };
                oss << std::endl;
            }
            
            {
                auto sep = "";
                oss << "dependencies : ";
                for ( auto dep : kv.second->dependencies) {
                    oss << sep << dep->id;
                    sep = ",";
                };
                oss << std::endl;
            }
        };
        return oss.str();
    };
    
    void reset() {
        nodes.clear();
    };
    
    GraphNodePtr getNode(const std::string &id) {
        if ( nodes.find( id ) == end(nodes) ) {
            nodes.emplace( id, std::make_shared<GraphNode>(id));
        }
        return nodes[id];
    };
    
    void addJobs( const Gem::JobSeq &jobs) {
        for ( auto &job : jobs) {
            if ( !job.dependencies.empty()) {
                auto node = getNode( job.id );
                for ( auto &dependency : job.dependencies) {
                    auto depNode = getNode( dependency );
                    node->dependencies.push_back( depNode );
                    depNode->dependents.push_back( node );
                };
            };
        };
    };
};


class GemServerImpl : public Gem::GemServer {
    std::vector<Gem::Job> jobs;
    Gem::GemServerListenerPrx pubToListeners;
    IceStorm::TopicPrx topic;
    Graph graph;

public:
    GemServerImpl(Ice::CommunicatorPtr communicator) : jobs() {
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
            std::ostringstream err;
            err << "Logic error can't find " << id << std::endl;
            throw std::logic_error( err.str());
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

    void blockDependenciesOf(const std::string &id ) {
        auto node = graph.getNode( id );
        for ( auto& depNode : node->dependents ) {
            auto depJob = find( depNode->id );
            if ( depJob == nullptr) {
                std::cout << " Logic error can't find job " << node->id << std::endl;
            };
            depJob->state = Gem::JobState::BLOCKED;
        };
    };
    
    virtual void submitBatch_async(const ::Gem::AMD_GemServer_submitBatchPtr& cb,
                                   const ::Gem::Batch& batch,
                                   const ::Ice::Current& = ::Ice::Current()) {
        const auto currentSize = jobs.size();
        std::cout << "AddJobs" << std::endl;
        graph.addJobs( batch.jobs );
        std::cout << "Inserting" << std::endl;
        jobs.insert(jobs.end(), begin(batch.jobs), end(batch.jobs));
        std::cout << "Inserted" << std::endl;
        std::cout << "Sorting" << std::endl;
        std::sort( begin(jobs), end(jobs), [&]( auto &a, auto &b) {
                return a.id < b.id;
            });
        
        for( auto& job : batch.jobs) {
            std::cout << "Looping " << job.id << std::endl;
            blockDependenciesOf( job.id );
        };
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

    virtual void reset_async(const ::Gem::AMD_GemServer_resetPtr& cb, const ::Ice::Current& = ::Ice::Current()) {
        jobs.clear();
        graph.reset();
        cb->ice_response();
    };
    
    virtual void getStartableJob_async(const ::Gem::AMD_GemServer_getStartableJobPtr& cb,
                                       const ::Gem::WorkerId&,
                                       const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobSeq ret;
        Gem::Job *selected = nullptr;
        
        for (auto &&job : jobs) {
            if ( job.state == Gem::JobState::STARTABLE ) {
                if (selected == nullptr || job.priority < selected->priority) {
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

    virtual void dumpStatus_async(const ::Gem::AMD_GemServer_dumpStatusPtr& cb, const ::Ice::Current& = ::Ice::Current()) {
        auto s = graph.dump();
        std::cout << s << std::endl;
        cb->ice_response(s);
    };
    
    virtual void getJobs_async(const ::Gem::AMD_GemServer_getJobsPtr& cb, const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response( jobs );
    }

    virtual void onWorkerStates_async(const ::Gem::AMD_GemServer_onWorkerStatesPtr& cb, const ::Gem::JobWorkerStateSeq& jwss, const ::Ice::Current& = ::Ice::Current()) {
        std::vector<Gem::Job> updated;
        for( auto &jws : jwss) {
            auto job = find( jws.id );
            if ( job->state != jws.state) {
                job->state = jws.state;
                updated.push_back(*job);
            };
            auto node = graph.getNode( job->id );
            for ( auto dep : node->dependents) {
                std::cout << "Check startable " << dep->id << std::endl;
                checkIsStartable( dep->id, updated);
            };
        };
        if ( !updated.empty() ) {
            pubToListeners->begin_onUpdate( updated ,
                                            []() { std::cout << "Publushed update" << std::endl; } );
        }
        cb->ice_response();
    };

    void checkIsStartable( const std::string& id, std::vector<Gem::Job>& updatedSink ) {
        auto job = find( id );
        if (job->state == Gem::JobState::BLOCKED) {
            bool isStartable = true;
            for ( auto& id : job->dependencies) {
                if (causesBlockage(id)) {
                    isStartable = false;
                    break;
                };
            };
            std::cout << "job is " << id << " startable is " << (isStartable ? "true" : "false") << std::endl;
            if ( isStartable ) {
                job->state = Gem::JobState::STARTABLE;
                updatedSink.push_back( *job);
            };
        };
    };

    bool causesBlockage( const std::string id ) {
        auto job = find(id);
        return job->state != Gem::JobState::WAIVERED &&
            job->state != Gem::JobState::COMPLETED;
    };

    virtual void addListener_async(const ::Gem::AMD_GemServer_addListenerPtr& cb,
                                   const ::Gem::GemServerListenerPrx& prx,
                                   const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Add listener " << std::endl;
        static const IceStorm::QoS theQos;
        Gem::Image img { { begin(jobs), end(jobs) } };
        topic->begin_subscribeAndGetPublisher(theQos,
                                              prx,
                                              [&](const Ice::ObjectPrx &response) {
                                                  std::cout << " Got publisher " << response << std::endl;
                                                  auto p = Gem::GemServerListenerPrx::uncheckedCast(response);
                                                  std::cout << "Made image " << std::endl;
                                                  p->begin_onImage( img ,
                                                                    []() {
                                                                        std::cout << "Success with image" << std::endl;
                                                                    } );
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
    
