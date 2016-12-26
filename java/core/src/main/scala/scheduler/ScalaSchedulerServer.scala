package scheduler

import java.util.concurrent.{Callable, ScheduledExecutorService}

import Ice._
import IceStorm.{NoSuchTopic, TopicManagerPrxHelper}

class ScalaSchedulerServer(val communicator : Communicator,
                     val executor: ScheduledExecutorService) extends _SchedulerServerDisp {
  var graph = Graph.empty

  var workers = Map[WorkerId, WorkerState]()

  val topic = {
    val topicPrx = TopicManagerPrxHelper.checkedCast(communicator.propertyToProxy("icestorm.topicmanager"))
    val subject = communicator.getProperties().getProperty("scheduler.topic")
    try {
      topicPrx.retrieve(subject)
    } catch {
      case (nst: NoSuchTopic) => topicPrx.create(subject)
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

  override def reset_async(cb: AMD_SchedulerServer_reset, current: Current): Unit = {
    println("Reset not implemented")
    cb.ice_response()
  }

  /*
  Create an array of jobs to add. We don't add them immediately
  in case we need to back out because of a detected cycle
   */
  def mapForJobs(newJobs : Array[Job]) = Map( newJobs.map( x => (x.id, new Node(x))) : _*)

  override def submitBatch_async(cb : AMD_SchedulerServer_submitBatch, batch: Batch, current: Current) = {
    runOnExecutor {
      val newGraph = Graph.forJobs(graph.jobs ++ mapForJobs( batch.jobs)) match {
        case Right(g) => {
          graph = g
          cb.ice_response()
        }
        case Left(jc) => cb.ice_exception(jc)
      }
    }
  }

  override def getStartableJob_async(amd_schedulerServer_getStartableJob: AMD_SchedulerServer_getStartableJob, workerId: WorkerId, current: Current): Unit = {
    runOnExecutor {
      graph.jobs.values.filter(_.isStartable).toArray.sortBy(_.priority).headOption match {
        case Some(x) => {
          x.jobState.currentWorker = Array(workerId)
          x.jobState.state = EnumJobState.State.SCHEDULED
          Array(x.job)
        }
        case None => Array.empty[Job]
      }
    }
  }

  override def dumpStatus_async(amd_schedulerServer_dumpStatus: AMD_SchedulerServer_dumpStatus,
                                current: Current): Unit = ???


  def withJobOnExecutor[T <: AMDCallback](jid : JobId, cb : T, f : (Node) => Unit ) = runOnExecutor {
    graph.jobs.get(jid) match {
      case Some(wj) => {
        f(wj)
      }
      case None => {
        cb.ice_exception(new JobNotExist(Array(jid)))
      }
    }
  }

  override def invalidateJob_async(cb: AMD_SchedulerServer_invalidateJob, id: JobId, __current: Current): Unit = {
    withJobOnExecutor(id, cb, (wj) => {
      val toKill = new java.util.HashSet[Node]()
      graph.invalidateImpl(wj, toKill)
      cb.ice_response()
    })
  }

  override def startJob_async(cb : AMD_SchedulerServer_startJob, jid : JobId, current: Current) : Unit = {
    withJobOnExecutor(jid, cb, (wj) =>
      if (wj.state == EnumJobState.State.DORMANT) {
        wj.state = EnumJobState.State.STARTABLE
        cb.ice_response()
      } else {
        cb.ice_exception(new JobNotStartable(jid))
      })
  }

  override def getJobs_async(cb: AMD_SchedulerServer_getJobs, current: Current): Unit = {
    cb.ice_response(graph.jobs.values.map(_.job).toArray[Job])
  }

  override def invalidate_async(amd_schedulerServer_invalidate: AMD_SchedulerServer_invalidate, jid : JobId, current: Current): Unit = ???

  override def addListener_async(cb : AMD_SchedulerServer_addListener,
                                 listener : SchedulerServerListenerPrx,
                                 current: Current): Unit = runOnExecutor {
    val qos = new java.util.HashMap[String,String]()
    topic.begin_subscribeAndGetPublisher(qos,
      listener,
      (rawPrx) => {
        println(s"Got proxy $rawPrx")
        val prx = SchedulerServerListenerPrxHelper.uncheckedCast(rawPrx)
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
    val tmpJobs   = graph.jobs.values.map(_.job).toArray[Job]
    val tmpStates = graph.jobs.values.map(_.jobState).toArray[JobState]
    new Image( tmpJobs, tmpStates, "")
  }

  override def addListenerWithIdent_async(amd_schedulerServer_addListenerWithIdent: AMD_SchedulerServer_addListenerWithIdent, identity: Identity, current: Current): Unit = ???

  override def stopJob_async(cb : AMD_SchedulerServer_stopJob,
                             jid : JobId,
                             current: Current) : Unit = {
    withJobOnExecutor(jid , cb, (wj) => {
      if (wj.state == EnumJobState.State.STARTED ) {
        /// XXAS - need to send cancel message to the worker
        wj.state = EnumJobState.State.CANCELLED
        cb.ice_response()
      } else {
        cb.ice_exception( new JobNotStartable(jid))
      }
    })
  }

  override def imageReady_async(amd_schedulerServer_imageReady: AMD_SchedulerServer_imageReady, batchId : String, s: String, current: Current): Unit = ???

  override def getJob_async(cb: AMD_SchedulerServer_getJob, jid : JobId, current: Current): Unit = {
    withJobOnExecutor(jid, cb, (wj) => {
      cb.ice_response(wj.job)
    })
  }

  override def onWorkerUpdate_async(__cb: AMD_SchedulerServer_onWorkerUpdate, x: WorkerUpdate, __current: Current): Unit = ???
}
