#pragma once

#include "Ice/Identity.ice"

module scheduler {
    sequence<string> StringSeq;

    dictionary<string, string> StringStringDict;

    struct JobId {
        string id;
        string batch;
    };

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
    module EnumWorkerJobState {
        enum State {
            STARTED,
            FAILED,
            CANCELLING,
            CANCELED
        };
    };

    struct JobUpdate {
        JobId  id;
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
        JobId id;
        ::scheduler::EnumWorkerJobState::State state;
    };

    sequence<WorkerStateDescription> WorkerStateDescriptionSeq;

    struct WorkerUpdate {
        WorkerId id;
        WorkerStateDescriptionSeq updates;
    };
    
    // What a job declares its status to be
    // Richer object. The job can report status messgaes/completion etc.
    struct JobStateDescription {
        JobId id;
        long startTime;
        long updateTime;
        string statusMsg;
        int pctComplete;
    };
    
    sequence<JobStateDescription> JobStateDescriptionSeq;
    
    struct Job {
        JobId id;
        StringSeq dependencies;
        int       priority;
        string    pwd;
        StringSeq cmdLine;
        StringStringDict env;
    };


    /* The 'stateful part of a job detached from the immutable
       description of a job. Separation of concerns etc. */
    
    struct JobState {
        string jobId;
        EnumJobState::State state = EnumJobState::DORMANT;
        WorkerIdSeq currentWorker;
        JobStateDescriptionSeq jobStatus;
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
        ["amd"] void onWorkerUpdate( WorkerUpdate x);
        ["amd"] void imageReady(string imgId);
    };
};
