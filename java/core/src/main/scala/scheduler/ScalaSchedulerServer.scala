package scheduler

import java.util
import java.util.concurrent.{Callable, ScheduledExecutorService}

import Ice._
import IceStorm.{NoSuchTopic, TopicManagerPrxHelper}

class ScalaSchedulerServer(val communicator: Communicator,
                           val executor: ScheduledExecutorService) extends _SchedulerServerDisp {
  var graph = Graph.empty

  var workers = Map[WorkerId, WorkerState]()

  val (topic, publisher) = {
    val topicPrx = TopicManagerPrxHelper.checkedCast(communicator.propertyToProxy("icestorm.topicmanager"))
    val subject = communicator.getProperties.getProperty("scheduler.topic")
    val topic = try {
      topicPrx.retrieve(subject)
    } catch {
      case (nst: NoSuchTopic) => topicPrx.create(subject)
    }
    (topic, SchedulerServerListenerPrxHelper.uncheckedCast(topic.getPublisher()))
  }

  def checkInvariants() : Unit = {
    // Check dependencies + dependants self-consistent.
  }

  def runOnExecutor(f: => Unit) = {
    val x: Runnable = () => {
      checkInvariants()
      f
      checkInvariants()
    }
    executor.submit(x)
  }

  def runOnExecutorAndWait[T](f: => T): T = {
    val c: Callable[T] = () => f
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
  def mapForJobs(newJobs: Array[Job]) = Map(newJobs.map(x => (x.id, new Node(x))): _*)

  override def submitBatch_async(cb: AMD_SchedulerServer_submitBatch, batch: Batch, current: Current) =
    runOnExecutor {
      val newJobs = mapForJobs(batch.jobs)
      val updated = new util.ArrayList[JobDTO]()
      for {
        x <- newJobs.values
      } {
        updated.add(x.makeDTO())
      }
      Graph.forJobs(graph.jobs ++ newJobs) match {
        case Right(g) => {
          graph = g
          checkStartableStates(updated)
          cb.ice_response()
        }
        case Left(jc) => cb.ice_exception(jc)
      }
      publisher.begin_onUpdate(updated.toArray(Array.empty[JobDTO]))
    }

  /* Call from executor! */
  def checkStartableStates(updated: util.ArrayList[JobDTO]): Unit = {
    import EnumJobState.READY
    for {
      v <- graph.jobs.values
      if v.state == EnumJobState.DORMANT
      if !graph.dependencies.isDefinedAt(v) || graph.dependencies(v).forall(_.state == EnumJobState.COMPLETED)
    } {
      v.state = READY
      updated.add(v.makeDTO())
    }
  }

  override def getStartableJob_async(cb: AMD_SchedulerServer_getStartableJob, workerId: WorkerId, current: Current): Unit = {
    runOnExecutor {
      graph.jobs.values.filter(_.isStartable).toArray.sortBy(_.priority).headOption match {
        case Some(x) => {
          x.jobState.currentWorker = Array(workerId)
          x.jobState.state = EnumJobState.SCHEDULED
          x.jobState.currentWorker = Array(workerId)
          cb.ice_response(Array(x.job))
          publisher.begin_onUpdate(Array(x.makeDTO()))
        }
        case None => cb.ice_response(Array.empty[Job])
      }
    }
  }

  override def dumpStatus_async(cb: AMD_SchedulerServer_dumpStatus,
                                current: Current): Unit = {
    val ret = graph.jobs.map(_.toString).mkString("\n")
    cb.ice_response(ret)
  }

  def withJobOnExecutor[T <: AMDCallback](jid: JobId, cb: T, f: (Node) => Unit) = runOnExecutor {
    graph.jobs.get(jid) match {
      case Some(wj) => f(wj)
      case None => cb.ice_exception(new JobNotExist(Array(jid)))
    }
  }

  override def invalidateJob_async(cb: AMD_SchedulerServer_invalidateJob, id: JobId, __current: Current): Unit = {
    withJobOnExecutor(id, cb, (wj) => {
      val toKill = new java.util.ArrayList[Node]()
      graph.invalidateImpl(wj, toKill)
      cb.ice_response()
    })
  }

  override def startJob_async(cb: AMD_SchedulerServer_startJob, jid: JobId, current: Current): Unit = {
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

  override def addListener_async(cb: AMD_SchedulerServer_addListener,
                                 listener: SchedulerServerListenerPrx,
                                 current: Current): Unit = runOnExecutor {
    val qos = new java.util.HashMap[String, String]()
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
      (ex: UserException) => {},
      (ex: Exception) => {})
  }

  def makeImage(): Image = {
    val tmpJobs = graph.jobs.values.map(_.makeDTO()).toArray[JobDTO]
    new Image(tmpJobs,  "")
  }

  override def addListenerWithIdent_async(cb : AMD_SchedulerServer_addListenerWithIdent, identity: Identity, current: Current): Unit = {
    print("Add listener with ident")
    cb.ice_response()
  }

  override def stopJob_async(cb: AMD_SchedulerServer_stopJob,
                             jid: JobId,
                             current: Current): Unit = {
    withJobOnExecutor(jid, cb, (wj) => {
      import EnumJobState._
      if (wj.state == STARTED) {
        wj.state = CANCELLING
      }
      cb.ice_response()
    })
  }

  override def imageReady_async(cb : AMD_SchedulerServer_imageReady, batchId: String, s: String, current: Current): Unit = {
    println(s"Image readyy $batchId $s")
    cb.ice_response()
  }

  override def getJob_async(cb: AMD_SchedulerServer_getJob, jid: JobId, current: Current): Unit = {
    withJobOnExecutor(jid, cb, (wj) => {
      cb.ice_response(wj.job)
    })
  }

  override def onWorkerUpdate_async(cb: AMD_SchedulerServer_onWorkerUpdate,
                                    wu: WorkerUpdate,
                                    __current: Current): Unit = {
    import EnumJobState._
    import JobStates._
    val updated = new util.ArrayList[JobDTO]()
    for {
      u <- wu.updates
      wj <- graph.jobs.get(u.id)
    } {
      val newState = (wj.state, u.state) match {
        case (x, y) if !x.isTerminal && y.isTerminal => {
          wj.jobState.currentWorker = Array.empty[WorkerId]
          Some(y)
        }
        case (SCHEDULED, STARTED) => Some(STARTED)
        case (CANCELLING, CANCELLED) => Some(CANCELLED)
        case _ => None
      }
      newState.foreach((x) => {
        wj.state = x
        updated.add(wj.makeDTO())
      })
    }
    cb.ice_response()
    if (!updated.isEmpty) {
      checkStartableStates(updated)
      publisher.begin_onUpdate( updated.toArray(Array.empty[JobDTO]))
    }
  }

  override def setState_async(cb: AMD_SchedulerServer_setState, id: JobId, state: EnumJobState, __current: Current): Unit = {
    println(s"Trying to Setting state $id $state")
    withJobOnExecutor(id, cb, (wj) => {
      println(s"Setting state $id $state")
      wj.state = state
      cb.ice_response()
    })
  }
}
