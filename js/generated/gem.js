// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
//
// Ice version 3.6.3
//
// <auto-generated>
//
// Generated from file `gem.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var __M = Ice.__M;
    var Slice = Ice.Slice;

    var Gem = __M.module("Gem");
    Slice.defineSequence(Gem, "StringSeqHelper", "Ice.StringHelper", false);
    Slice.defineDictionary(Gem, "StringStringDict", "StringStringDictHelper", "Ice.StringHelper", "Ice.StringHelper", false, undefined, undefined);

    Gem.JobState = Slice.defineEnum([
        ['BLOCKED', 0], ['STARTABLE', 1], ['SCHEDULED', 2], ['STARTED', 3], ['STOPPED', 4],
        ['FAILED', 5], ['WAIVERED', 6], ['COMPLETED', 7]]);

    Gem.JobUpdate = Slice.defineStruct(
        function(id, pctComplete, status)
        {
            this.id = id !== undefined ? id : "";
            this.pctComplete = pctComplete !== undefined ? pctComplete : 0;
            this.status = status !== undefined ? status : "";
        },
        true,
        function(__os)
        {
            __os.writeString(this.id);
            __os.writeInt(this.pctComplete);
            __os.writeString(this.status);
        },
        function(__is)
        {
            this.id = __is.readString();
            this.pctComplete = __is.readInt();
            this.status = __is.readString();
        },
        6, 
        false);
    Slice.defineSequence(Gem, "JobUpdateSeqHelper", "Gem.JobUpdate", false);

    Gem.WorkerId = Slice.defineStruct(
        function(id)
        {
            this.id = id !== undefined ? id : "";
        },
        true,
        function(__os)
        {
            __os.writeString(this.id);
        },
        function(__is)
        {
            this.id = __is.readString();
        },
        1, 
        false);

    Gem.JobWorkerState = Slice.defineStruct(
        function(worker, id, state)
        {
            this.worker = worker !== undefined ? worker : new Gem.WorkerId();
            this.id = id !== undefined ? id : "";
            this.state = state !== undefined ? state : Gem.JobState.BLOCKED;
        },
        true,
        function(__os)
        {
            Gem.WorkerId.write(__os, this.worker);
            __os.writeString(this.id);
            Gem.JobState.__write(__os, this.state);
        },
        function(__is)
        {
            this.worker = Gem.WorkerId.read(__is, this.worker);
            this.id = __is.readString();
            this.state = Gem.JobState.__read(__is);
        },
        3, 
        false);
    Slice.defineSequence(Gem, "JobWorkerStateSeqHelper", "Gem.JobWorkerState", false);

    Gem.Job = Slice.defineStruct(
        function(id, dependencies, state, priority, pwd, cmdLine, env, batchId)
        {
            this.id = id !== undefined ? id : "";
            this.dependencies = dependencies !== undefined ? dependencies : null;
            this.state = state !== undefined ? state : Gem.JobState.STARTABLE;
            this.priority = priority !== undefined ? priority : 0;
            this.pwd = pwd !== undefined ? pwd : "";
            this.cmdLine = cmdLine !== undefined ? cmdLine : null;
            this.env = env !== undefined ? env : null;
            this.batchId = batchId !== undefined ? batchId : "";
        },
        false,
        function(__os)
        {
            __os.writeString(this.id);
            Gem.StringSeqHelper.write(__os, this.dependencies);
            Gem.JobState.__write(__os, this.state);
            __os.writeInt(this.priority);
            __os.writeString(this.pwd);
            Gem.StringSeqHelper.write(__os, this.cmdLine);
            Gem.StringStringDictHelper.write(__os, this.env);
            __os.writeString(this.batchId);
        },
        function(__is)
        {
            this.id = __is.readString();
            this.dependencies = Gem.StringSeqHelper.read(__is);
            this.state = Gem.JobState.__read(__is);
            this.priority = __is.readInt();
            this.pwd = __is.readString();
            this.cmdLine = Gem.StringSeqHelper.read(__is);
            this.env = Gem.StringStringDictHelper.read(__is);
            this.batchId = __is.readString();
        },
        11, 
        false);
    Slice.defineSequence(Gem, "JobSeqHelper", "Gem.Job", false);
    Slice.defineDictionary(Gem, "JobDict", "JobDictHelper", "Ice.StringHelper", "Gem.Job", false, undefined, undefined, Ice.HashMap.compareEquals);

    Gem.Batch = Slice.defineStruct(
        function(jobs)
        {
            this.jobs = jobs !== undefined ? jobs : null;
        },
        false,
        function(__os)
        {
            Gem.JobSeqHelper.write(__os, this.jobs);
        },
        function(__is)
        {
            this.jobs = Gem.JobSeqHelper.read(__is);
        },
        1, 
        false);

    Gem.Image = Slice.defineStruct(
        function(jobs, currentImage)
        {
            this.jobs = jobs !== undefined ? jobs : null;
            this.currentImage = currentImage !== undefined ? currentImage : "";
        },
        false,
        function(__os)
        {
            Gem.JobSeqHelper.write(__os, this.jobs);
            __os.writeString(this.currentImage);
        },
        function(__is)
        {
            this.jobs = Gem.JobSeqHelper.read(__is);
            this.currentImage = __is.readString();
        },
        2, 
        false);

    Gem.GemServerListener = Slice.defineObject(
        undefined,
        Ice.Object, undefined, 0,
        [
            "::Gem::GemServerListener",
            "::Ice::Object"
        ],
        -1, undefined, undefined, false);

    Gem.GemServerListenerPrx = Slice.defineProxy(Ice.ObjectPrx, Gem.GemServerListener.ice_staticId, undefined);

    Slice.defineOperations(Gem.GemServerListener, Gem.GemServerListenerPrx,
    {
        "onImage": [, , , 1, , , [[Gem.Image]], , , , ],
        "onUpdate": [, , , 1, , , [["Gem.JobSeqHelper"]], , , , ],
        "onImageReady": [, , , 1, , , [[7]], , , , ],
        "onReset": [, , , 1, , , , , , , ]
    });

    Gem.JobNotFound = Slice.defineUserException(
        function(id, _cause)
        {
            Ice.UserException.call(this, _cause);
            this.id = id !== undefined ? id : "";
        },
        Ice.UserException,
        "Gem::JobNotFound",
        function(__os)
        {
            __os.writeString(this.id);
        },
        function(__is)
        {
            this.id = __is.readString();
        },
        false,
        false);

    Gem.GemServer = Slice.defineObject(
        undefined,
        Ice.Object, undefined, 0,
        [
            "::Gem::GemServer",
            "::Ice::Object"
        ],
        -1, undefined, undefined, false);

    Gem.GemServerPrx = Slice.defineProxy(Ice.ObjectPrx, Gem.GemServer.ice_staticId, undefined);

    Slice.defineOperations(Gem.GemServer, Gem.GemServerPrx,
    {
        "submitBatch": [, , , 1, , , [[Gem.Batch]], , , , ],
        "startJob": [, , , 1, , , [[7]], , , , ],
        "stopJob": [, , , 1, , , [[7]], , , , ],
        "invalidate": [, , , 1, , , [[7]], , , , ],
        "reset": [, , , 1, , , , , , , ],
        "dumpStatus": [, , , 1, , [7], , , , , ],
        "getJobs": [, , , 1, , ["Gem.JobSeqHelper"], , , , , ],
        "getStartableJob": [, , , 1, , ["Gem.JobSeqHelper"], [[Gem.WorkerId]], , , , ],
        "getJob": [, , , 1, , [Gem.Job], [[7]], , 
        [
            Gem.JobNotFound
        ], , ],
        "addListener": [, , , 1, , , [["Gem.GemServerListenerPrx"]], , , , ],
        "addListenerWithIdent": [, , , 1, , , [[Ice.Identity]], , , , ],
        "onWorkerStates": [, , , 1, , , [["Gem.JobWorkerStateSeqHelper"]], , , , ],
        "imageReady": [, , , 1, , , [[7]], , , , ]
    });
    exports.Gem = Gem;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
