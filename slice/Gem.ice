#pragma once

module Gem {
    sequence<string> StringSeq;
    
    dictionary<string, string> StringStringDict;

    enum JobState {
        BLOCKED,
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

    struct JobWorkerState {
        WorkerId worker;
        string id;
        JobState state;
    };

    sequence<JobWorkerState> JobWorkerStateSeq;

    struct Job {
        string    id;
        StringSeq dependencies;
        JobState  state = STARTABLE;
        int       priority;
        string    pwd;
        StringSeq cmdLine;
        StringStringDict env;
        string    batchId;
    };

    sequence<Job> JobSeq;
    dictionary<string,Job> JobDict;

    struct Batch {
        JobSeq jobs;
    };


    struct Image {
        JobSeq jobs;
    };
    
    interface GemServerListener {
        ["amd"] void onImage( Image image);
        ["amd"] void onUpdate( JobSeq jobs);
    };

    interface GemServer {
        ["amd"] void submitBatch( Batch batch );
        ["amd"] void startJob( string id );
        ["amd"] void stopJob( string id );
        ["amd"] void invalidate( string id);
        ["amd"] void reset();
        ["amd"] string dumpStatus();
        
        ["amd"] JobSeq getJobs();
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] JobSeq getJob( string id );

        ["amd"] void addListener( GemServerListener *listener);

        ["amd"] void onWorkerStates( JobWorkerStateSeq xs);
    };
};
