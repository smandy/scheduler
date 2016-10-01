#include "Gem.h"
#include "Ice/Ice.h"
#include <memory>
#include <unordered_map>

typedef std::shared_ptr<Gem::Job> JobPtr;

class GemServerImpl : public Gem::GemServer {
    std::map<std::string, JobPtr> jobs;
    virtual void submitBatch_async(const ::Gem::AMD_GemServer_submitBatchPtr& cb,
                                   const ::Gem::Batch& batch,
                                   const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Woot" << std::endl;
        for( auto &&x : batch.jobs) {
            jobs[x.id] = JobPtr(new Gem::Job(x));
        };
        cb->ice_response();
    };
    
    virtual void startJob_async(const ::Gem::AMD_GemServer_startJobPtr& cb,
                                const ::std::string& id,
                                const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Woot" << std::endl;
        cb->ice_response();
    };
    
    virtual void stopJob_async(const ::Gem::AMD_GemServer_stopJobPtr& cb,
                               const ::std::string&,
                               const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Woot" << std::endl;
        cb->ice_response();
    };
    
    virtual void invalidate_async(const ::Gem::AMD_GemServer_invalidatePtr& cb,
                                  const ::std::string&, const ::Ice::Current& = ::Ice::Current()) {
        std::cout << "Woot" << std::endl;
        cb->ice_response();
    };

    virtual void getStartableJob_async(const ::Gem::AMD_GemServer_getStartableJobPtr& cb, const ::Gem::WorkerId&, const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobSeq ret;
        for (auto &job : jobs ) {
            if ( job.second->state == Gem::JobState::STARTABLE) {
                job.second->state == Gem::JobState::STARTED;
                if (ret.empty()) {
                    ret.push_back(*job.second);
                } else {
                    if ( job.second->priority < ret[0].priority) {
                        ret[0] = *(job.second.get());
                    };
                }
            }
        };
        cb->ice_response( ret );
    };
    
    virtual void getJobs_async(const ::Gem::AMD_GemServer_getJobsPtr& cb, const ::Ice::Current& = ::Ice::Current()) {
        Gem::JobDict ret;
        for( auto &x : jobs) {
            ret[x.first] = *x.second;
        };
        cb->ice_response( ret );
    };
};

int main(int argc, char *argv[]) {
    std::cout << "Make communicator" << std::endl;
    auto communicator = Ice::initialize( argc, argv);
    std::cout << "Make server" << std::endl;
    auto server = std::make_shared<GemServerImpl>();
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
    
