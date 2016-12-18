package bamma

import java.util.concurrent.{Callable, ScheduledExecutorService}

import Gem._
import Ice._
import IceStorm.{NoSuchTopic, TopicManagerPrxHelper, TopicManagerPrx}

class ScalaGemServer(val communicator : Communicator,
                     val executor: ScheduledExecutorService) extends _GemServerDisp {
  var jobs = Map[String, WrappedJob]()
  var workers = Map[WorkerId, WorkerState]()

  var listeners = Set[GemServerListenerPrx]()

  val topic = {
    val topicPrx = TopicManagerPrxHelper.checkedCast(communicator.propertyToProxy("icestorm.topicManager"))
    val subject = communicator.getProperties().getProperty("gem.topic")
    try {
      topicPrx.retrieve(topic)
    } catch {
      case (nst: NoSuchTopic) => topicPrx.create(topic)
    }
  }

  /// XXXAS - check worker states.
  def runOnExecutor(f: => Unit) = {
    val x : Runnable = () => { f }
    executor.submit( x )
  }

  def runOnExecutorAndWait[T](f: => T) : T = {
    val c : Callable[T] = () => f
    val fut = executor.submit(c)
    fut.get()
  }

  override def reset_async(amd_gemServer_reset: AMD_GemServer_reset, current: Current): Unit = ???

  override def onWorkerStates_async(amd_gemServer_onWorkerStates: AMD_GemServer_onWorkerStates, jobWorkerStates: Array[JobWorkerState], current: Current): Unit = ???

  override def submitBatch_async(amd_gemServer_submitBatch: AMD_GemServer_submitBatch, batch: Batch, current: Current) = {
    runOnExecutor {
      for {job <- batch.jobs} {
        JobHolder
        jobs += (job.id -> WrappedJob.forJob(job))
      }
    }
  }

  override def getStartableJob_async(amd_gemServer_getStartableJob: AMD_GemServer_getStartableJob, workerId: WorkerId, current: Current): Unit = {
    runOnExecutor {
      jobs.values.filter(_.isStartable).toArray.sortBy(_.priority).headOption match {
        case Some(x) => {
          x.jd.currentWorker = Array(workerId)
          x.jd.state = JobState.SCHEDULED
          Array(x.jd.job)
        }
        case None => Array.empty[JobDescription]
      }
    }
  }

  override def dumpStatus_async(amd_gemServer_dumpStatus: AMD_GemServer_dumpStatus,
                                current: Current): Unit = ???


  def withJobOnExecutor[T <: AMDCallback](s : String, cb : T, f : (WrappedJob) => Unit ) = runOnExecutor {
    jobs.get(s) match {
      case Some(wj) => {
        f(wj)
      }
      case None => {
        cb.ice_exception(new JobNotExist(s))
      }
    }
  }

  override def startJob_async(cb : AMD_GemServer_startJob, s: String, current: Current) : Unit = {
    withJobOnExecutor(s, cb, (wj) =>
      if (wj.state == JobState.DORMANT) {
        wj.state = JobState.SCHEDULED
        cb.ice_response()
      } else {
        cb.ice_exception(new JobNotStartable(s))
      })
  }

  override def getJobs_async(cb: AMD_GemServer_getJobs, current: Current): Unit = {
    cb.ice_response(jobs.values.map(_.jd.job).toArray[Job])
  }

  override def invalidate_async(amd_gemServer_invalidate: AMD_GemServer_invalidate, s: String, current: Current): Unit = ???

  //def r( f : => Unit) : Runnable = () => { f }

  override def addListener_async(cb : AMD_GemServer_addListener,
                                 listener : GemServerListenerPrx,
                                 current: Current): Unit = runOnExecutor {
    listeners += listener
    val qos = new java.util.HashMap[String,String]()
    topic.begin_subscribeAndGetPublisher(qos,
      listener,
      (rawPrx) => {
        println(s"Got proxy $rawPrx")
        val prx = GemServerListenerPrxHelper.uncheckedCast(rawPrx)
        val image = makeImage()
        prx.begin_onImage(
          image,
          () => println("Success"),
          (ex) => println("Failed"))
      },
      (ex : UserException) => {},
      (ex : Exception) => {} )
  }

  def makeImage() : Image = {
    val tmpJobs = jobs.values.map(_.jd.job).toArray[Job]
    new Image( tmpJobs, "")
  }

  override def addListenerWithIdent_async(amd_gemServer_addListenerWithIdent: AMD_GemServer_addListenerWithIdent, identity: Identity, current: Current): Unit = ???

  override def stopJob_async(cb: AMD_GemServer_stopJob,
                             s: String,
                             current: Current): Unit = {
    withJobOnExecutor(s, cb, (wj) => {
      if (wj.state == JobState.STARTED ) {
        wj.state = JobState.STOPPED
        cb.ice_response()
      } else {
        cb.ice_exception( new JobNotStartable(s))
      }
    })
  }

  override def imageReady_async(amd_gemServer_imageReady: AMD_GemServer_imageReady, s: String, current: Current): Unit = ???

  override def getJob_async(cb: AMD_GemServer_getJob, s: String, current: Current): Unit = {
    withJobOnExecutor(s, cb, (wj) => {
      => cb.ice_response(wj.jd.job)
    })
  }
}
