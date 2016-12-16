package bamma;

import Gem.AMD_GemServer_addListener;
import Gem.AMD_GemServer_addListenerWithIdent;
import Gem.AMD_GemServer_dumpStatus;
import Gem.AMD_GemServer_getJob;
import Gem.AMD_GemServer_getJobs;
import Gem.AMD_GemServer_getStartableJob;
import Gem.AMD_GemServer_imageReady;
import Gem.AMD_GemServer_invalidate;
import Gem.AMD_GemServer_onWorkerStates;
import Gem.AMD_GemServer_reset;
import Gem.AMD_GemServer_startJob;
import Gem.AMD_GemServer_stopJob;
import Gem.AMD_GemServer_submitBatch;
import Gem.Batch;
import Gem.GemServerListenerPrx;
import Gem.JobNotFound;
import Gem.JobWorkerState;
import Gem.WorkerId;
import Gem._GemServerDisp;
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
    public void getJob_async(AMD_GemServer_getJob amd_gemServer_getJob, String s, Current current) throws JobNotFound {

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
