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

    struct JobWorkerState {
        string id;
        JobState state;
    };

    struct Job {
        string    id;
        int       priority;
        JobState  state = STARTABLE;
        string    pwd;
        StringSeq cmdLine;
        StringStringDict env;
        StringSeq        dependencies;
        string    batchId;
    };

    sequence<Job> JobSeq;
    dictionary<string,Job> JobDict;

    struct Batch {
        JobSeq jobs;
    };

    struct WorkerId {
        string id;
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
        ["amd"] JobSeq getJobs();
        
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] JobSeq getJob( string id );

        ["amd"] void addListener( GemServerListener *listener);
    };
};
