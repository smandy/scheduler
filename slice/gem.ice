#pragma once

#include "Ice/Identity.ice"

module Gem {
    sequence<string> StringSeq;

    dictionary<string, string> StringStringDict;
    
    const string POJONAME = "POJO";
    const string POJO2NAME = "POJO2";

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
        ["amd"] void submitBatch( Batch batch );
        ["amd"] void startJob( string id );
        ["amd"] void stopJob( string id );
        ["amd"] void invalidate( string id);
        ["amd"] void reset();
        ["amd"] string dumpStatus();
        ["amd"] JobSeq getJobs();
        ["amd"] JobSeq getStartableJob( WorkerId worker );
        ["amd"] Job getJob( string id ) throws JobNotFound;
        ["amd"] void addListener( GemServerListener *listener);
        ["amd"] void addListenerWithIdent( Ice::Identity ident);
        ["amd"] void onWorkerStates( JobWorkerStateSeq xs);
        ["amd"] void imageReady(string imgId);
    };

    struct POJO {
        string type = POJONAME;
        int x;
        int y;
    };

    struct POJO2 {
        string type = POJO2NAME;
        string first;
        string last;
    };
};
