package bamma;

import Gem.*;
import Ice.Current;
import Ice.Identity;



public class GemServer extends _GemServerDisp {
    @Override
    public void submitBatch_async(AMD_GemServer_submitBatch amd_gemServer_submitBatch, Batch batch, Current current) {

    }

    @Override
    public void startJob_async(AMD_GemServer_startJob amd_gemServer_startJob, String s, Current current) {

    }

    @Override
    public void stopJob_async(AMD_GemServer_stopJob amd_gemServer_stopJob, String s, Current current) {

    }

    @Override
    public void invalidate_async(AMD_GemServer_invalidate amd_gemServer_invalidate, String s, Current current) {

    }

    @Override
    public void reset_async(AMD_GemServer_reset amd_gemServer_reset, Current current) {

    }

    @Override
    public void dumpStatus_async(AMD_GemServer_dumpStatus amd_gemServer_dumpStatus, Current current) {

    }

    @Override
    public void getJobs_async(AMD_GemServer_getJobs amd_gemServer_getJobs, Current current) {

    }

    @Override
    public void getStartableJob_async(AMD_GemServer_getStartableJob amd_gemServer_getStartableJob, WorkerId workerId, Current current) {
    }

    @Override
    public void getJob_async(AMD_GemServer_getJob amd_gemServer_getJob, String s, Current current) throws JobNotExist {
    }

    @Override
    public void addListener_async(AMD_GemServer_addListener amd_gemServer_addListener, GemServerListenerPrx gemServerListenerPrx, Current current) {

    }

    @Override
    public void addListenerWithIdent_async(AMD_GemServer_addListenerWithIdent amd_gemServer_addListenerWithIdent, Identity identity, Current current) {

    }

    @Override
    public void onWorkerStates_async(AMD_GemServer_onWorkerStates amd_gemServer_onWorkerStates, JobWorkerState[] jobWorkerStates, Current current) {

    }

    @Override
    public void imageReady_async(AMD_GemServer_imageReady amd_gemServer_imageReady, String s, Current current) {

    }
}
