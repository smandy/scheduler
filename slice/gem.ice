#pragma once

#include "Ice/Identity.ice"

module scheduler {
    sequence<string> StringSeq;

    dictionary<string, string> StringStringDict;

    module EnumJobState {
    
        enum State {
            BLOCKED,
            DORMANT,
            STARTABLE,
            SCHEDULED,
            STARTED,
            STOPPED,
            FAILED,
            WAIVERED,
            COMPLETED
        };
    };

    // The state a worker thinks a job is in
    namespace  EnumWorkerJobState {
        enum State {
            STARTED,
            FAILED,
            CANCELLING,
            CANCELED
        }
    };

    struct JobUpdate {
        string id;
        int    pctComplete;
        string status;
    };

    sequence<JobUpdate> JobUpdateSeq;
    
    struct WorkerId {
        string id;
    };

    sequence<WorkerId> WorkerIdSeq;
    
    // A workers perspective on a job.
    // ALl he really knows if he's started it and if it's complete
    struct WorkerStateDescription {
        string jobId;
        WorkerId worker;
        ::scheduler::EnumWorkerJobState::State state;
    };
    
    // What a job declares its status to be
    // Richer object. The job can report status messgaes/completion etc.
    struct JobStateDescription {
        string jobId;
        long startTime;
        long updateTime;
        string statusMsg;
        int pctComplete;
    };
    
    sequence<JobStateDescription> JobStateDescriptionSeq;
    
    sequence<JobWorkerState> JobWorkerStateSeq;

    struct Job {
        string    id;
        StringSeq dependencies;
        int       priority;
        string    pwd;
        StringSeq cmdLine;
        StringStringDict env;
        string    batchId;
    };



    struct JobState {
        string jobId;
        JobState state = DORMANT;
        WorkerIdSeq currentWorker;
        JobWorkerStateSeq claimedJobStatus;
        
    };


    exception DuplicateJob {
        string id;
    };

    exception JobNotExist {
        string id;
    };
    
    exception JobNotStartable {
        string id;
    };

    sequence<Job> JobSeq;
    sequence<JobState> JobStateSeq;
    dictionary<string,Job> JobDict;

    struct Batch {
        JobSeq jobs;
    };

    struct Image {
        JobSeq      jobs;
        JobStateSeq states;
        string currentImage;
    };
    
    interface SchedulerServerListener {
        ["amd"] void onImage( Image image );
        ["amd"] void onUpdate( JobStateSeq jobs );
        ["amd"] void onImageReady(string imgId);
        ["amd"] void onReset();
    };

    exception JobNotFound {
        string id;
    };

    interface SchedulerServer {
        ["amd"] void submitBatch( Batch batch ) throws DuplicateJob;
        
        ["amd"] void startJob( string id ) throws JobNotExist, JobNotStartable;
        ["amd"] void stopJob( string id );
        
        ["amd"] void invalidate( string id);
        ["amd"] void reset();
        ["amd"] string dumpStatus();
        ["amd"] JobSeq getJobs();
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] Job getJob( string id ) throws JobNotExist;
        ["amd"] void addListener( SchedulerServerListener *listener);
        ["amd"] void addListenerWithIdent( Ice::Identity ident);
        ["amd"] void onWorkerStates( JobWorkerStateSeq xs);
        ["amd"] void imageReady(string imgId);
    };
};
