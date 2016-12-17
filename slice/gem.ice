#pragma once

#include "Ice/Identity.ice"

module Gem {
    sequence<string> StringSeq;

    dictionary<string, string> StringStringDict;
    
    enum JobState {
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

    struct JobWorkerState {
        WorkerId worker;
        string id;
        JobState state;
    };

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

    exception DuplicateJob {
        string id;
    };

    exception JobNotExist {
        string id;
    };
    
    exception JobNotStartable {
        string id;
    };

    
    struct JobDescription {
        Job job;
        JobState state = DORMANT;
        WorkerIdSeq currentWorker;
    };

    sequence<Job> JobSeq;
    dictionary<string,Job> JobDict;

    struct Batch {
        JobSeq jobs;
    };

    struct Image {
        JobSeq jobs;
        string currentImage;
    };
    
    interface GemServerListener {
        ["amd"] void onImage( Image image );
        ["amd"] void onUpdate( JobSeq jobs );
        ["amd"] void onImageReady(string imgId);
        ["amd"] void onReset();
    };

    exception JobNotFound {
        string id;
    };

    interface GemServer {
        ["amd"] void submitBatch( Batch batch ) throws DuplicateJob;
        
        ["amd"] void startJob( string id ) throws JobNotExist, JobNotStartable;
        ["amd"] void stopJob( string id );
        
        ["amd"] void invalidate( string id);
        ["amd"] void reset();
        ["amd"] string dumpStatus();
        ["amd"] JobSeq getJobs();
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] Job getJob( string id ) throws JobNotExist;
        ["amd"] void addListener( GemServerListener *listener);
        ["amd"] void addListenerWithIdent( Ice::Identity ident);
        ["amd"] void onWorkerStates( JobWorkerStateSeq xs);
        ["amd"] void imageReady(string imgId);
    };
};
