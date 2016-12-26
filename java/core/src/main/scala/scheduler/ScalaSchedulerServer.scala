package scheduler

import java.util.concurrent.{Callable, ScheduledExecutorService}

import Ice._
import IceStorm.{NoSuchTopic, TopicManagerPrxHelper}

class ScalaSchedulerServer(val communicator : Communicator,
                     val executor: ScheduledExecutorService) extends _SchedulerServerDisp {
  var graph = Graph.empty

  var workers = Map[WorkerId, WorkerState]()

  val (topic, publisher) = {
    val topicPrx = TopicManagerPrxHelper.checkedCast(communicator.propertyToProxy("icestorm.topicmanager"))
    val subject = communicator.getProperties().getProperty("scheduler.topic")
    val topic = try {
      topicPrx.retrieve(subject)
    } catch {
      case (nst: NoSuchTopic) => topicPrx.create(subject)
    }
    (topic, SchedulerServerListenerPrxHelper.uncheckedCast(topic.getPublisher()))
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
    println("Reset")
    graph = Graph.empty
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
          checkStartableStates()
          cb.ice_response()
        }
        case Left(jc) => cb.ice_exception(jc)
      }
    }
  }

  def checkStartableStates(): Unit = {
    import EnumJobState.READY
    for {
      v <- graph.jobs.values
      if (!graph.dependencies.isDefinedAt(v) || graph.dependencies(v).forall(_.state == EnumJobState.COMPLETED))
    } {
      v.state = READY
    }
  }

  override def getStartableJob_async(cb : AMD_SchedulerServer_getStartableJob, workerId: WorkerId, current: Current): Unit = {
    runOnExecutor {
      graph.jobs.values.filter(_.isStartable).toArray.sortBy(_.priority).headOption match {
        case Some(x) => {
          x.jobState.currentWorker = Array(workerId)
          x.jobState.state = EnumJobState.SCHEDULED
          cb.ice_response(Array(x.job))
        }
        case None => cb.ice_response(Array.empty[Job])
      }
    }
  }

  override def dumpStatus_async(cb : AMD_SchedulerServer_dumpStatus,
                                current: Current): Unit = {
    val ret = graph.jobs.map( _.toString ).mkString("\n")
    cb.ice_response(ret)
  }

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
      if (wj.state == EnumJobState.DORMANT) {
        wj.state = EnumJobState.READY
        cb.ice_response()
      } else {
        cb.ice_exception(new JobNotStartable(jid))
      })
  }

  override def getJobs_async(cb: AMD_SchedulerServer_getJobs, current: Current): Unit = {
    cb.ice_response(graph.jobs.values.map(_.makeDTO()).toArray[JobDTO])
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
      import EnumJobState._
      if ( wj.state == STARTED) {
        wj.state = CANCELLING
      }
      cb.ice_response()
    })
  }

  def sendUpdate( n : Node): Unit = {



  }

  override def imageReady_async(amd_schedulerServer_imageReady: AMD_SchedulerServer_imageReady, batchId : String, s: String, current: Current): Unit = ???

  override def getJob_async(cb: AMD_SchedulerServer_getJob, jid : JobId, current: Current): Unit = {
    withJobOnExecutor(jid, cb, (wj) => {
      cb.ice_response(wj.job)
    })
  }

  override def onWorkerUpdate_async(cb : AMD_SchedulerServer_onWorkerUpdate, x: WorkerUpdate, __current: Current): Unit = {
    import EnumJobState._
    import JobStates.PimpState
    for {
      u <- x.updates
      wj <- graph.jobs.get(u.id)
    } {
      val newState = ( wj.state.isTerminal, wj.state, u.state) match {
        case (false, _, FAILED)    => Some(FAILED)
        case (false, _, CANCELLED)  => Some(CANCELLED)
        case (false, _, COMPLETED) => Some(COMPLETED)
        case (_, SCHEDULED, STARTED) => Some(STARTED)
        case _ => None
      }
      newState.foreach( (x) => {
        wj.state = x
        checkStartableStates()
      } )
    }
    cb.ice_response()
  }
}
