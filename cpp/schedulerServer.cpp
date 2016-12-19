#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include <memory>
#include <set>
#include <iostream>
#include "scheduler.h"
#include <spdlog/spdlog.h>

// Pointers for getting icestorm up
// Aha - the trick was exposing IceStorm/TopicManager as a well known object
// https://github.com/SintefRaufossManufacturing/icehms/blob/master/icecfg/icebox.xml
//typedef std::shared_ptr<Scheduler::Job> JobPtr;
// http://git.asterisk.org/gitweb/?p=asterisk-scf/release/techdemo.git;a=commitdiff_plain;h=89b836724135902a7d628cc08bd8ddd786b82240

using loggerType = std::shared_ptr<spdlog::logger>;

class SchedulerServerImpl : public scheduler::SchedulerServer {
    std::vector<scheduler::Job> jobs;
    IceStorm::TopicPrx topic;
    std::unordered_map<scheduler::JobId, std::vector<scheduler::JobId> > dependants;
    std::set<scheduler::SchedulerServerListenerPrx> listeners;
    std::string currentImage;
public:
    static loggerType log;
    SchedulerServerImpl(Ice::CommunicatorPtr communicator) {
        log = spdlog::get("log");
        log->info("Get Topic");
        auto topicPrx = IceStorm::TopicManagerPrx::checkedCast( communicator->propertyToProxy("IceStorm.TopicManager"));
        log->info("Get Subject");
        auto subject = communicator->getProperties()->getProperty("Scheduler.Topic");
        log->info("Subject is {}",subject);
        try {
            topic = topicPrx->retrieve(subject);
        } catch( IceStorm::NoSuchTopic &) {
            topic = topicPrx->create(subject);
        }
        listeners.insert(scheduler::SchedulerServerListenerPrx::uncheckedCast(topic->getPublisher()));
        log->info("Done");
    }
    
    inline scheduler::Job* find(const scheduler::JobId id) {
        static thread_local scheduler::Job finder;
        scheduler::Job *ret = nullptr;
        finder.id = id;
        auto x = std::lower_bound( begin(jobs),
                                   end(jobs),
                                   finder,
                                   []( auto& a, auto& b) {
                                       return a.id.id < b.id.id ? true :
                                       (a.id.batch < b.id.batch);
                                   });
        if (x != end(jobs) && x->id==id) {
            return &(*x);
        } else {
            throw scheduler::JobNotExist(id);
        }
    }

    virtual void getJob_async(const ::scheduler::AMD_SchedulerServer_getJobPtr& cb,
                              const ::scheduler::JobId& id,
                              const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response(*find(id));
    }

    void blockDependenciesOf(const scheduler::JobId &id ) {
        for ( auto& depId : dependants[id]) {
            auto depJob = find(depId);
            depJob->state = scheduler::JobState::BLOCKED;
        }
    }


     virtual void imageReady_async(const ::scheduler::AMD_SchedulerServer_imageReadyPtr& cb,
                                   const ::std::string& s,
                                   const ::Ice::Current& = ::Ice::Current()) {

         currentImage = s;
         for (auto& listener : listeners) {
             listener->begin_onImageReady(s,
                                          [s]() {
                                              log->info("Sent image ready {}", s);
                                          },
                                          [&](const Ice::Exception& ex) {
                                              log->error("Bad listener {}", ex.what());
                                              auto it = listeners.find(listener);
                                              if ( it != std::end(listeners)) {
                                                  log->info("Expunging {}", *it);
                                                  listeners.erase( it );
                                              }
                                              log->info("Done expunge");
                                          });
         }
     };
    
    virtual void submitBatch_async(const ::scheduler::AMD_SchedulerServer_submitBatchPtr& cb,
                                   const ::scheduler::Batch& batch,
                                   const ::Ice::Current& = ::Ice::Current()) {
        log->info("Submitbatch");
        for ( auto &job : batch.jobs) {
            if ( !job.dependencies.empty()) {
                for ( auto &depId : job.dependencies) {
                    dependants[depId].push_back( job.id );
                }
            }
        }
        log->info("Inserting");
        jobs.insert(jobs.end(), begin(batch.jobs), end(batch.jobs));
        log->info("Sorting");
        std::sort(begin(jobs), end(jobs), [&]( auto &a, auto &b) {
                return a.id < b.id;
            });
        for(auto& job : batch.jobs) {
            log->info("Looping");
            blockDependenciesOf( job.id );
        }

        // Ug - we've blocked our copies so we need to go and fetch
        // them back from the graph. Noticed when developing the web
        // gui too much stuff was reporting itself as 'startable' :-(
        scheduler::JobSeq ret;
        for( auto& j : batch.jobs) {
            log->info("Looking for ");
            ret.push_back( *find(j.id) );
        };
        log->info("Have {} items to return" , ret.size());
        for (auto& listener : listeners) {
            listener->begin_onUpdate(ret,
                                     []() {
                                         log->debug("Sent new jobs");
                                     },
                                     [&](const Ice::Exception& ex) {
                                         log->error("Bad listener : \n{}\n", ex.what());
                                         auto it = listeners.find(listener);
                                         if ( it != std::end(listeners)) {
                                             log->error("Expunging");
                                             listeners.erase( it );
                                         }
                                     });
        }
        cb->ice_response();
    }
    
    virtual void startJob_async(const ::scheduler::AMD_SchedulerServer_startJobPtr& cb,
                                const ::std::string& id,
                                const ::Ice::Current& = ::Ice::Current()) {
        log->info("startJob {}, id");
        cb->ice_response();
    }
    
    virtual void stopJob_async(const ::scheduler::AMD_SchedulerServer_stopJobPtr& cb,
                               const ::std::string& id,
                               const ::Ice::Current& = ::Ice::Current()) {
        log->info("stopJob {}", id);
        cb->ice_response();
    }
    
    virtual void invalidate_async(const ::scheduler::AMD_SchedulerServer_invalidatePtr& cb,
                                  const ::std::string& id,
                                  const ::Ice::Current& = ::Ice::Current()) {
        log->info("invalidate {}", id);
        cb->ice_response();
    }

    virtual void reset_async(const ::scheduler::AMD_SchedulerServer_resetPtr& cb,
                             const ::Ice::Current& = ::Ice::Current()) {
        jobs.clear();
        dependants.clear();
        currentImage = "";
        for (auto& listener : listeners) {
            listener->begin_onReset([]() {},
                                    [&](const Ice::Exception& ex) {
                                        log->info("Bad listener");
                                        auto it = listeners.find(listener);
                                        if ( it != std::end(listeners)) {
                                            log->info("Expunging");
                                            listeners.erase( it );
                                        }
                                    });
        }
        cb->ice_response();
    }
    
    virtual void getStartableJob_async(const ::scheduler::AMD_SchedulerServer_getStartableJobPtr& cb,
                                       const ::scheduler::WorkerId&,
                                       const ::Ice::Current& = ::Ice::Current()) {
        scheduler::JobSeq ret;
        scheduler::Job *selected = nullptr;
        for (auto &job : jobs) {
            if (job.state == scheduler::JobState::STARTABLE ) {
                if (!selected || job.priority < selected->priority) {
                    selected = &job;
                }
            }
        }
        if (selected) {
            selected->state = scheduler::JobState::SCHEDULED;
            ret.push_back(*selected);
        }
        cb->ice_response( ret );
    }

    virtual void dumpStatus_async(const ::scheduler::AMD_SchedulerServer_dumpStatusPtr& cb,
                                  const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response("Unimplemented");
    }
    
    virtual void getJobs_async(const ::scheduler::AMD_SchedulerServer_getJobsPtr& cb,
                               const ::Ice::Current& = ::Ice::Current()) {
        cb->ice_response( jobs );
    }

    virtual void onWorkerUpdate_async(const ::scheduler::AMD_SchedulerServer_onWorkerUpdatePtr& cb,
                                      const ::scheduler::WorkerUpdate& wud,
                                      const ::Ice::Current& = ::Ice::Current()) {
        std::vector<scheduler::Job> updated;
        for(auto &update : wud.updates) {
            auto job = find( jws.id );
            if ( job->state != jws.state) {
                job->state = jws.state;
                updated.push_back(*job);
            }
            for ( auto dep : dependants[job->id]) {
                log->info("Check startable");
                checkIsStartable( dep, updated);
            }
        }
        
        if (!updated.empty()) {
            for ( auto& listener : listeners) {
                listener->begin_onUpdate(updated,
                                         []() {
                                             log->info("Sent new jobs");
                                         },
                                         [&](const Ice::Exception& ex) {
                                             log->info("Bad listener");
                                             auto it = listeners.find(listener);
                                             if ( it != end(listeners) ) {
                                                 listeners.erase( it );
                                             }
                                         });
            }
        }
        cb->ice_response();
    }

    void checkIsStartable( const std::string& id,
                           std::vector<scheduler::Job>& updatedSink) {
        auto job = find( id );
        if (job->state == scheduler::JobState::BLOCKED) {
            bool isStartable = true;
            for ( auto& id : job->dependencies) {
                if (causesBlockage(id)) {
                    isStartable = false;
                    break;
                };
            }
            log->info("job is {} startable={}" , id ,isStartable);
            if (isStartable) {
                job->state = scheduler::JobState::STARTABLE;
                updatedSink.push_back( *job);
            }
        }
    }

    bool causesBlockage( const std::string id ) {
        auto job = find(id);
        return job->state != scheduler::JobState::WAIVERED &&
            job->state != scheduler::JobState::COMPLETED;
    }

    virtual void addListenerWithIdent_async(const ::scheduler::AMD_SchedulerServer_addListenerWithIdentPtr& cb,
                                            const ::Ice::Identity& ident,
                                            const ::Ice::Current& current = ::Ice::Current()) {
        log->info("Addlistener with ident {} {}", ident.name, ident.category);
        ::scheduler::SchedulerServerListenerPrx client = scheduler::SchedulerServerListenerPrx::uncheckedCast(current.con->createProxy(ident));
        scheduler::Image img { jobs, currentImage };
        //std::cout << "Made image " << std::endl;
        client->begin_onImage( img ,
                               [this,client]() {
                                   log->debug("Success with image");
                                   listeners.insert(client);
                               },
                               [](const Ice::Exception& ex) {
                                   log->error("Failed to send to listener - sod him {}", ex.what());
                               });
        cb->ice_response();
    };
    
    virtual void addListener_async(const ::scheduler::AMD_SchedulerServer_addListenerPtr& cb,
                                   const ::scheduler::SchedulerServerListenerPrx& prx,
                                   const ::Ice::Current& = ::Ice::Current()) {
        log->info("Add listener {}", prx);
        static const IceStorm::QoS theQos;
        topic->begin_subscribeAndGetPublisher(theQos,
                                              prx,
                                              [this,cb](const Ice::ObjectPrx &response) {
                                                  //std::cout << " Got publisher " << response << std::endl;
                                                  auto p = scheduler::SchedulerServerListenerPrx::uncheckedCast(response);
                                                  scheduler::Image img { jobs, currentImage };
                                                  //std::cout << "Made image " << std::endl;
                                                  p->begin_onImage( img ,
                                                                    []() {
                                                                        log->debug("Success with image");
                                                                    });
                                                  //std::cout << "Called image " << cb << std::endl;
                                                  // Hmmm ... should this be in side the lambda above maybe?
                                                  cb->ice_response();
                                                  //std::cout << "Called response " << std::endl;
                                              } );
        log->debug("End method");
    }
};

int main(int argc, char *argv[]) {

    spdlog::set_async_mode(4096, spdlog::async_overflow_policy::block_retry,
                           nullptr,
                           std::chrono::seconds(2));
    auto log = spdlog::rotating_logger_mt("log", "/tmp/scheduler", 1048576 * 5, 3);
    log->info("Make communicator");
    auto communicator = Ice::initialize(argc, argv);
    log->info("Make server");
    auto server = std::make_unique<SchedulerServerImpl>(communicator);
    log->info("Make adapter");
    auto adapter = communicator->createObjectAdapter("SchedulerServer");
    log->info("Add impl to adapter");
    // Liveness of server ensured by scope of main
    auto prx = adapter->add(server.get(),
                            communicator->stringToIdentity("server"));
    log->info("Activate adpater");
    adapter->activate();
    log->info("Wait for shutdown");
    communicator->waitForShutdown();
    log->info("Shutdown - exiting");
    log->flush();
    adapter->deactivate();
    log->info("Adapter deactivated");
    communicator->destroy();
    log->info("Communicator destroyed. Game over");
    log->flush();
};
    
loggerType SchedulerServerImpl::log {};
