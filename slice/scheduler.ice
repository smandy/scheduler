#pragma once

#include "Ice/Identity.ice"

[["cpp:include:unordered_map"]]

module scheduler {
    sequence<string> StringSeq;

    dictionary<string, string> StringStringDict;
    
    ["cpp:comparable"] struct JobId {
        string id;
        string batch;
    };

    sequence<JobId> JobIdSeq;

    ["cpp:type:std::unordered_map<JobId, JobId>"] dictionary<JobId,JobId> tmpJobId;

    enum EnumJobState {
        DORMANT,
        READY,
        SCHEDULED,
        STARTED,
        CANCELLING,
        CANCELLED,
        FAILED,
        COMPLETED
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
        EnumJobState state;
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
        JobIdSeq  dependencies;
        int       priority;
        string    pwd;
        StringSeq cmdLine;
        StringStringDict env;
    };

    /* The 'stateful part of a job detached from the immutable
       description of a job. Separation of concerns etc. */
    struct JobState {
        JobId id;
        EnumJobState state = DORMANT;
        
        WorkerIdSeq currentWorker;
        
        // Reported by the job itself
        JobStateDescriptionSeq jobStatus;
        
        // Reported by the worker
        WorkerStateDescriptionSeq workerStatus;
    };

    /* Convenience to use until we hava fancy version with job + state
       uncoupled */
    struct JobDTO {
        Job job;
        JobState state;
    };

    sequence<JobDTO> JobDTOSeq;

    exception DuplicateJob {
        JobIdSeq id;
    };

    exception JobCycleDetected {
        JobIdSeq badNodes;
    };

    exception JobNotExist {
        JobIdSeq id;
    };
    
    exception JobNotStartable {
        JobId id;
    };

    sequence<Job> JobSeq;
    sequence<JobState> JobStateSeq;
    dictionary<string,Job> JobDict;

    struct Batch {
        JobSeq jobs;
    };

    struct Image {
        JobDTOSeq jobs;
        string currentImage;
    };
    
    interface SchedulerServerListener {
        ["amd","ami"] void onImage( Image image );
        ["amd","ami"] void onUpdate( JobDTOSeq jobs );
        ["amd","ami"] void onImageReady(string batchId, string imgId);
        ["amd","ami"] void onReset();
    };

    interface SchedulerServer {
        ["amd"] void submitBatch( Batch batch ) throws DuplicateJob;
        
        ["amd"] void startJob( JobId id ) throws JobNotExist, JobNotStartable;
        ["amd"] void stopJob( JobId id );
        ["amd"] void invalidateJob( JobId id );
        
        ["amd"] void reset();
        
        ["amd"] string dumpStatus();
        ["amd"] JobDTOSeq getJobs();
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] Job getJob( JobId id ) throws JobNotExist;
        ["amd"] void addListener( SchedulerServerListener *listener);
        
        // For Javascript
        ["amd"] void addListenerWithIdent( Ice::Identity ident);
        
        ["amd"] void onWorkerUpdate(WorkerUpdate x);
        
        // Invoked from the graph server
        ["amd"] void imageReady(string batchId, string imgId);

        ["amd"] void setState( JobId id, EnumJobState state);

        ["amd"] Image getImage();

    };
};
