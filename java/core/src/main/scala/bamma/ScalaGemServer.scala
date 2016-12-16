package bamma

import java.util.concurrent.ScheduledExecutorService

import Gem._
import Ice.{Current, Identity}

object ScalaGemServer {
  import Gem.JobState._

  val startableStates = {
    Set( STARTABLE)
  }

  val stoppableStates = {
    Set(STARTED)
  }
}

class ScalaGemServer(val executor : ScheduledExecutorService) extends _GemServerDisp {
  var jobs = Map[String,JobDescription]()
  var workers = Map[WorkerId,WorkerState]()
  /// XXXAS - check worker states.

  override def reset_async(amd_gemServer_reset: AMD_GemServer_reset, current: Current): Unit = ???

  override def onWorkerStates_async(amd_gemServer_onWorkerStates: AMD_GemServer_onWorkerStates, jobWorkerStates: Array[JobWorkerState], current: Current): Unit = ???

  override def submitBatch_async(amd_gemServer_submitBatch: AMD_GemServer_submitBatch, batch: Batch, current: Current): Unit = {
    for { job <- batch.jobs } {
      jobs += (job.id -> WrappedJob.forJob(job) )
    }
  }

  override def getStartableJob_async(amd_gemServer_getStartableJob: AMD_GemServer_getStartableJob, workerId: WorkerId, current: Current): Unit = ???

  override def dumpStatus_async(amd_gemServer_dumpStatus: AMD_GemServer_dumpStatus, current: Current): Unit = ???

  override def startJob_async(amd_gemServer_startJob: AMD_GemServer_startJob, s: String, current: Current): Unit = ???

  override def getJobs_async(cb: AMD_GemServer_getJobs, current: Current): Unit = {
    cb.ice_response(jobs.values.toArray[Job])
  }

  override def invalidate_async(amd_gemServer_invalidate: AMD_GemServer_invalidate, s: String, current: Current): Unit = ???

  override def addListener_async(amd_gemServer_addListener: AMD_GemServer_addListener, gemServerListenerPrx: GemServerListenerPrx, current: Current): Unit = ???

  override def addListenerWithIdent_async(amd_gemServer_addListenerWithIdent: AMD_GemServer_addListenerWithIdent, identity: Identity, current: Current): Unit = ???

  override def stopJob_async(amd_gemServer_stopJob: AMD_GemServer_stopJob, s: String, current: Current): Unit = ???

  override def imageReady_async(amd_gemServer_imageReady: AMD_GemServer_imageReady, s: String, current: Current): Unit = ???

  override def getJob_async(cb : AMD_GemServer_getJob, s: String, current: Current): Unit = {
    jobs.get(s) match {
      case Some(jobWrapper) => cb.ice_response( jobWrapper.job )
      case None => cb.ice_exception( new JobNotExist(s))
    }
  }
}
