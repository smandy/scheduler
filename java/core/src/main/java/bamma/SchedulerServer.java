package bamma;

import Scheduler.*;
import Ice.Current;
import Ice.Identity;



public class SchedulerServer extends _SchedulerServerDisp {
    @Override
    public void submitBatch_async(AMD_SchedulerServer_submitBatch amd_schedulerServer_submitBatch, Batch batch, Current current) {

    }

    @Override
    public void startJob_async(AMD_SchedulerServer_startJob amd_schedulerServer_startJob, String s, Current current) {

    }

    @Override
    public void stopJob_async(AMD_SchedulerServer_stopJob amd_schedulerServer_stopJob, String s, Current current) {

    }

    @Override
    public void invalidate_async(AMD_SchedulerServer_invalidate amd_schedulerServer_invalidate, String s, Current current) {

    }

    @Override
    public void reset_async(AMD_SchedulerServer_reset amd_schedulerServer_reset, Current current) {

    }

    @Override
    public void dumpStatus_async(AMD_SchedulerServer_dumpStatus amd_schedulerServer_dumpStatus, Current current) {

    }

    @Override
    public void getJobs_async(AMD_SchedulerServer_getJobs amd_schedulerServer_getJobs, Current current) {

    }

    @Override
    public void getStartableJob_async(AMD_SchedulerServer_getStartableJob amd_schedulerServer_getStartableJob, WorkerId workerId, Current current) {
    }

    @Override
    public void getJob_async(AMD_SchedulerServer_getJob amd_schedulerServer_getJob, String s, Current current) throws JobNotExist {
    }

    @Override
    public void addListener_async(AMD_SchedulerServer_addListener amd_schedulerServer_addListener, SchedulerServerListenerPrx schedulerServerListenerPrx, Current current) {

    }

    @Override
    public void addListenerWithIdent_async(AMD_SchedulerServer_addListenerWithIdent amd_schedulerServer_addListenerWithIdent, Identity identity, Current current) {

    }

    @Override
    public void onWorkerStates_async(AMD_SchedulerServer_onWorkerStates amd_schedulerServer_onWorkerStates, JobWorkerState[] jobWorkerStates, Current current) {

    }

    @Override
    public void imageReady_async(AMD_SchedulerServer_imageReady amd_schedulerServer_imageReady, String s, Current current) {

    }
}
