#include "Ice/Ice.h"
#include "IceStorm/IceStorm.h"
#include "scheduler.h"
#include <iostream>
#include <memory>
#include <set>
#include <spdlog/spdlog.h>

// Pointers for getting icestorm up
// Aha - the trick was exposing IceStorm/TopicManager as a well known object
// https://github.com/SintefRaufossManufacturing/icehms/blob/master/icecfg/icebox.xml
// typedef std::shared_ptr<Job> JobPtr;
// http://git.asterisk.org/gitweb/?p=asterisk-scf/release/techdemo.git;a=commitdiff_plain;h=89b836724135902a7d628cc08bd8ddd786b82240

using loggerType = std::shared_ptr<spdlog::logger>;

using namespace scheduler;

class SchedulerServerImpl : public SchedulerServer {
  std::vector<JobDTO> jobs;
  std::shared_ptr<IceStorm::TopicPrx> topic;
  std::map<JobId, std::vector<JobId>> dependants;
  std::set<std::shared_ptr<SchedulerServerListenerPrx>> listeners;
  std::string currentImage;

public:
  static loggerType log;
  SchedulerServerImpl(Ice::CommunicatorPtr communicator) {
    log = spdlog::get("log");
    log->info("Get Topic");
    auto topicManagerPrx = Ice::checkedCast<IceStorm::TopicManagerPrx>(
        communicator->propertyToProxy("IceStorm.TopicManager"));
    log->info("Get Subject");
    auto subject =
        communicator->getProperties()->getProperty("Scheduler.Topic");
    log->info("Subject is {}", subject);
    try {
      topic = topicManagerPrx->retrieve(subject);
    } catch (IceStorm::NoSuchTopic &) {
      topic = topicManagerPrx->create(subject);
    }
    listeners.insert(
        Ice::uncheckedCast<SchedulerServerListenerPrx>(topic->getPublisher()));
    log->info("Done");
  }

  inline JobDTO *find(const JobId id) {
    static thread_local JobDTO finder;
    Job *ret = nullptr;
    finder.job.id = id;
    auto x =
        std::lower_bound(begin(jobs), end(jobs), finder, [](auto &a, auto &b) {
          return a.job.id.id < b.job.id.id ? true
                                           : (a.job.id.batch < b.job.id.batch);
        });
    if (x != end(jobs) && x->job.id == id) {
      return &(*x);
    } else {
      throw JobNotExist{{id}};
    }
  }

  virtual void getJobAsync(JobId id, ::std::function<void(const JobDTO &)> cb,
                           ::std::function<void(::std::exception_ptr)> ex,
                           const ::Ice::Current &) {
    /// TODO - not found
    try {
      cb(*find(id));
    } catch (JobNotExist &jne) {
      ex(std::current_exception());
    };
  }

  void blockDependenciesOf(const JobId &id) {
    for (auto &depId : dependants[id]) {
      auto depJob = find(depId);
      depJob->state.state = EnumJobState::READY;
    }
  }

  virtual void
  imageReadyAsync(::std::string s, ::std::string batchId,
                  ::std::function<void()> cb,
                  ::std::function<void(::std::exception_ptr)> error,
                  const ::Ice::Current &) {
    currentImage = s;
    for (auto &listener : listeners) {
      listener->onImageReadyAsync(
          batchId, s, [s]() { log->info("Sent image ready {}", s); },
          [&](std::exception_ptr ex) {
            try {
              rethrow_exception(ex);
            } catch (const Ice::Exception &ex) {
              log->error("Bad listener {}", ex.what());
              auto it = listeners.find(listener);
              if (it != std::end(listeners)) {
                log->info("Expunging"); // TODO - detail
                listeners.erase(it);
              }
              log->info("Done expunge");
            };
          });
      cb();
    }
  }

  virtual void submitBatchAsync(Batch batch, ::std::function<void()> cb,
                                ::std::function<void(::std::exception_ptr)> ex,
                                const ::Ice::Current &) {
    log->info("Submitbatch");
    for (auto &job : batch.jobs) {
      if (!job.dependencies.empty()) {
        for (auto &depId : job.dependencies) {
          dependants[depId].push_back(job.id);
        }
      }
    }

    std::vector<JobDTO> dest;
    std::transform(begin(batch.jobs), end(batch.jobs), begin(dest),
                   [&](const auto &j) {
                     return JobDTO{j, {}};
                   });

    log->info("Inserting");
    jobs.insert(jobs.end(), begin(dest), end(dest));
    log->info("Sorting");
    std::sort(begin(jobs), end(jobs),
              [&](auto &a, auto &b) { return a.job.id < b.job.id; });
    for (auto &job : batch.jobs) {
      log->info("Looping");
      blockDependenciesOf(job.id);
    }

    // Ug - we've blocked our copies so we need to go and fetch
    // them back from the graph. Noticed when developing the web
    // gui too much stuff was reporting itself as 'startable' :-(
    JobDTOSeq ret;
    for (auto &j : batch.jobs) {
      log->info("Looking for ");
      ret.push_back(*find(j.id));
    };
    log->info("Have {} items to return", ret.size());
    for (auto &listener : listeners) {
      listener->onUpdateAsync(ret, []() { log->debug("Sent new jobs"); },
                              [&](const std::exception_ptr ex2) {
                                try {
                                  std::rethrow_exception(ex2);
                                } catch (const Ice::Exception &ex) {
                                  log->error("Bad listener : \n{}\n",
                                             ex.what());
                                  auto it = listeners.find(listener);
                                  if (it != std::end(listeners)) {
                                    log->error("Expunging");
                                    listeners.erase(it);
                                  }
                                };

                              });
    }
    cb();
  }

  virtual void startJobAsync(::scheduler::JobId id, ::std::function<void()> cb,
                             ::std::function<void(::std::exception_ptr)> error,
                             const ::Ice::Current &) {
    log->info("startJob {}", id.id);
    cb();
  }

  virtual void stopJobAsync(JobId id, ::std::function<void()> cb,
                            ::std::function<void(::std::exception_ptr)> error,
                            const ::Ice::Current &) {
    log->info("stopJob {}", id.id);
    cb();
  }

  virtual void
  invalidateJobAsync(scheduler::JobId id, std::function<void()> cb,
                     std::function<void(std::__exception_ptr::exception_ptr)>,
                     const Ice::Current &) {
    log->info("invalidate {}", id.id);
    cb();
  }

  virtual void resetAsync(::std::function<void()> cb,
                          ::std::function<void(::std::exception_ptr)> error,
                          const ::Ice::Current &) {
    jobs.clear();
    dependants.clear();
    currentImage = "";
    for (auto &listener : listeners) {
      listener->onResetAsync([]() {},
                             [&](const std::exception_ptr x) {
                               try {
                               } catch (const Ice::Exception &ex) {
                                 log->info("Bad listener");
                                 auto it = listeners.find(listener);
                                 if (it != std::end(listeners)) {
                                   log->info("Expunging");
                                   listeners.erase(it);
                                 }
                               }
                             });
    };
    cb();
  }

  virtual void
  getStartableJobAsync(WorkerId wid, ::std::function<void(const JobSeq &)> cb,
                       ::std::function<void(::std::exception_ptr)> error,
                       const ::Ice::Current &) {
    JobSeq ret;
    JobDTO *selected = nullptr;
    for (auto &dto : jobs) {
      if (dto.state.state == EnumJobState::READY) {
        if (!selected || dto.job.priority < selected->job.priority) {
          selected = &dto;
        }
      }
    }
    if (selected) {
      selected->state.state = EnumJobState::SCHEDULED;
      ret.push_back(selected->job);
    }
    cb(ret);
  }

  virtual void
  dumpStatusAsync(::std::function<void(const ::std::string &)>,
                  ::std::function<void(::std::exception_ptr)> error,
                  const ::Ice::Current &) {
    error(std::make_exception_ptr(std::invalid_argument{"Unimplmented"}));
  }

  virtual void
  getJobsAsync(std::function<void(const std::vector<scheduler::JobDTO> &)> cb,
               std::function<void(std::__exception_ptr::exception_ptr)> error,
               const Ice::Current &) {
    cb(jobs);
  }

  virtual void
  onWorkerUpdateAsync(WorkerUpdate wud, ::std::function<void()> cb,
                      ::std::function<void(::std::exception_ptr)> error,
                      const ::Ice::Current &) {
    std::vector<JobDTO> updated;
    for (auto &update : wud.updates) {
      auto dto = find(update.id);
      if (dto->state.state != update.state) {
        dto->state.state = update.state;
        updated.push_back(*dto);
      }
      for (auto dep : dependants[dto->job.id]) {
        log->info("Check startable");
        checkIsStartable(dep, updated);
      }
    }
    if (!updated.empty()) {
      for (auto &listener : listeners) {
        listener->onUpdateAsync(updated, []() { log->info("Sent new jobs"); },
                                [this, listener](const std::exception_ptr x) {
                                  try {
                                    rethrow_exception(x);
                                  } catch (const Ice::Exception &ex) {
                                    log->info("Bad listener");
                                    auto it = listeners.find(listener);
                                    if (it != end(listeners)) {
                                      listeners.erase(it);
                                    }
                                  };
                                });
      }
    }
    cb();
  }

  void checkIsStartable(const JobId &id, std::vector<JobDTO> &updatedSink) {
    auto dto = find(id);
    // TODO - check if ready is right.
    if (dto->state.state == EnumJobState::READY) {
      bool isStartable = true;
      for (auto &id : dto->job.dependencies) {
        if (causesBlockage(id)) {
          isStartable = false;
          break;
        };
      }
      log->info("job is {} startable={}", id.id, isStartable);
      if (isStartable) {
        // TODO - check state
        dto->state.state = EnumJobState::READY;
        updatedSink.push_back(*dto);
      }
    }
  }

  bool causesBlockage(const JobId &id) {
    auto dto = find(id);
    return dto->state.state != EnumJobState::COMPLETED;
  }
  virtual void
  addListenerWithIdentAsync(::Ice::Identity ident, ::std::function<void()> cb,
                            ::std::function<void(::std::exception_ptr)> error,
                            const ::Ice::Current &current) {
    log->info("Addlistener with ident {} {}", ident.name, ident.category);
    auto client = Ice::uncheckedCast<SchedulerServerListenerPrx>(
        current.con->createProxy(ident));
    Image img{jobs, currentImage};
    // std::cout << "Made image " << std::endl;
    client->onImageAsync(
        img,
        [this, client]() {
          log->debug("Success with image");
          listeners.insert(client);
        },
        [](const std::exception_ptr x) {
          try {
            std::rethrow_exception(x);
          } catch (const Ice::Exception &ex) {
            log->error("Failed to send to listener - sod him {}", ex.what());
          };
        });
    cb();
  }

  virtual void setStateAsync(::scheduler::JobId, ::scheduler::EnumJobState,
                             ::std::function<void()>,
                             ::std::function<void(::std::exception_ptr)>,
                             const ::Ice::Current &){
      // Not implemented
  }

  virtual void getImageAsync(::std::function<void(const ::scheduler::Image &)>,
                             ::std::function<void(::std::exception_ptr)>,
                             const ::Ice::Current &){
      // Not implemented
  }

  virtual void
  addListenerAsync(::std::shared_ptr<SchedulerServerListenerPrx> prx,
                   ::std::function<void()> cb,
                   ::std::function<void(::std::exception_ptr)> error,
                   const ::Ice::Current &) {
    log->info("Add listener {}", prx->ice_toString());
    static const IceStorm::QoS theQos;
    topic->subscribeAndGetPublisherAsync(
        theQos, prx, [this, cb](auto response) {
          // std::cout << " Got publisher " << response << std::endl;
          auto p = Ice::uncheckedCast<SchedulerServerListenerPrx>(response);
          Image img{jobs, currentImage};
          // std::cout << "Made image " << std::endl;
          p->onImageAsync(img, []() { log->debug("Success with image"); });
          // std::cout << "Called image " << cb << std::endl;
          // Hmmm ... should this be in side the lambda above maybe?
          cb();
          // std::cout << "Called response " << std::endl;
        });
    log->debug("End method");
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_async_mode(4096, spdlog::async_overflow_policy::block_retry,
                         nullptr, std::chrono::seconds(2));
  auto log =
      spdlog::rotating_logger_mt("log", "/tmp/scheduler", 1048576 * 5, 3);
  log->info("Make communicator");
  auto communicator = Ice::initialize(argc, argv);
  log->info("Make server");
  auto server = std::make_shared<SchedulerServerImpl>(communicator);
  log->info("Make adapter");
  auto adapter = communicator->createObjectAdapter("SchedulerServer");
  log->info("Add impl to adapter");
  // Liveness of server ensured by scope of main
  auto prx = adapter->add(server, Ice::stringToIdentity("server"));
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
}

loggerType SchedulerServerImpl::log{};
