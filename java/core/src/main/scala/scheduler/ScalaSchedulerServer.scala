package scheduler

import java.util
import java.util.concurrent.{Callable, CompletableFuture, CompletionStage, ScheduledExecutorService}

import com.zeroc.Ice.{Communicator, Current, Identity}
import com.zeroc.IceStorm.{NoSuchTopic, TopicManagerPrx}

class ScalaSchedulerServer(val communicator: Communicator,
                           val executor: ScheduledExecutorService) extends SchedulerServer {


  override def getImageAsync(current: Current): CompletionStage[Image] = ???

  private var graph = Graph.empty

  def assertOnExecutor(): Unit = {
    //assert( executor.)
  }

  val (topic, publisher) = {
    val topicPrx = TopicManagerPrx.checkedCast(communicator.propertyToProxy("icestorm.topicmanager"))
    val subject = communicator.getProperties.getProperty("scheduler.topic")
    val topic = try {
      topicPrx.retrieve(subject)
    } catch {
      case (nst: NoSuchTopic) => topicPrx.create(subject)
    }
    (topic, SchedulerServerListenerPrx.uncheckedCast(topic.getPublisher()))
  }

  var listeners = Set[SchedulerServerListenerPrx](publisher)

  /* Call from executor */
  def onUpdate(update: Array[JobDTO]): Unit = {
    listeners.foreach((x) => {
      val tmp = x.onUpdateAsync(update)
    })
  }

  def checkInvariants(): Unit = {
    // Check dependencies + dependants self-consistent.
  }

  def runOnExecutor(f: => Unit) = {
    val x: Runnable = () => {
      // checkInvariants()
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

  override def resetAsync(current: Current): CompletionStage[Void] = {
    println("Reset")
    graph = Graph.empty
    CompletableFuture.completedFuture(null)
  }

  /*
  Create an array of jobs to add. We don't add them immediately
  in case we need to back out because of a detected cycle
   */
  def mapForJobs(newJobs: Array[Job]) = Map(newJobs.map(x => (x.id, new Node(x))): _*)

  override def submitBatchAsync(batch: Batch, current: Current): CompletionStage[Void] = {
    val cb = new CompletableFuture[Void]
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
          cb.complete(null)
        }
        case Left(jc) => cb.completeExceptionally(jc)
      }
      onUpdate(updated.toArray(Array.empty[JobDTO]))
    }
    cb
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

  override def getStartableJobAsync(workerId: WorkerId, current: Current): CompletableFuture[Array[Job]] = {
    val cb = new CompletableFuture[Array[Job]]
    runOnExecutor {
      graph.jobs.values.filter(_.isStartable).toArray.sortBy(_.priority).headOption match {
        case Some(x) => {
          x.jobState.currentWorker = Array(workerId)
          x.jobState.state = EnumJobState.SCHEDULED
          onUpdate(Array(x.makeDTO()))
          cb.complete(Array(x.job))
        }
        case None => cb.complete(Array.empty[Job])
      }
    }
    cb
  }

  override def dumpStatusAsync(current: Current): CompletableFuture[String] = {
    val ret = graph.jobs.map(_.toString).mkString("\n")
    CompletableFuture.completedFuture(ret)
  }

  def withJobOnExecutor[T](jid: JobId, cb: CompletableFuture[T], f: (Node) => Unit) = runOnExecutor {
    graph.jobs.get(jid) match {
      case Some(wj) => {
        f(wj)
      }
      case None => cb.completeExceptionally(new JobNotExist(Array(jid)))
    }
  }

  override def invalidateJobAsync(id: JobId, __current: Current): CompletionStage[Void] = {
    val cb = new CompletableFuture[Void]
    withJobOnExecutor(id, cb, (wj) => {
      val toKill = new java.util.ArrayList[Node]()
      graph.invalidateImpl(wj, toKill)
      cb.complete(null)
    })
    cb
  }

  override def startJobAsync(jid: JobId, current: Current): CompletableFuture[Void] = {
    val cb = new CompletableFuture[Void]
    withJobOnExecutor(jid, cb, (wj) =>
      if (wj.state == EnumJobState.DORMANT) {
        wj.state = EnumJobState.READY
        cb.complete(null)
      } else {
        cb.completeExceptionally(new JobNotStartable(jid))
      })
    cb
  }

  override def getJobsAsync(current: Current): CompletionStage[Array[JobDTO]] = {
    CompletableFuture.completedFuture(graph.jobs.values.map(_.makeDTO()).toArray[JobDTO])
  }

  override def addListenerAsync(listener: SchedulerServerListenerPrx,
                                current: Current): CompletionStage[Void] = {
    val cb = new CompletableFuture[Void]
    runOnExecutor {
      val qos = new java.util.HashMap[String, String]()
      topic.subscribeAndGetPublisherAsync(qos, listener).whenComplete((rawPrx, ex) => {
        val prx = SchedulerServerListenerPrx.uncheckedCast(rawPrx)
        val image = makeImage()
        prx.onImageAsync(image).whenComplete((r, x) => {
          println("Call complete")
          cb.complete(null)
        })
      })
    }
    cb
  }

  def makeImage(): Image = {
    val tmpJobs = graph.jobs.values.map(_.makeDTO()).toArray[JobDTO]
    new Image(tmpJobs, "")
  }

  override def addListenerWithIdentAsync(ident: Identity, current: Current): CompletableFuture[Void] = {
    println("Add listener with ident jamone")
    val prx = SchedulerServerListenerPrx.uncheckedCast(current.con.createProxy(ident));
    val image = makeImage()
    prx.onImageAsync(image)
    println("Sending image")
    CompletableFuture.completedFuture(null)
  }

  override def stopJobAsync(jid: JobId,
                            current: Current): CompletableFuture[Void] = {
    val cb = new CompletableFuture[Void]()
    withJobOnExecutor(jid, cb, (wj) => {
      import EnumJobState._
      if (wj.state == STARTED) {
        wj.state = CANCELLING
      }
      cb.complete(null)
    })
    cb
  }

  override def imageReadyAsync(batchId: String, s: String, current: Current): CompletionStage[Void] = {
    println(s"Image readyy $batchId $s")
    CompletableFuture.completedFuture(null)
  }

  override def getJobAsync(jid: JobId, current: Current): CompletionStage[Job] = {
    val cb = new CompletableFuture[Job]()
    withJobOnExecutor(jid, cb, (wj) => {
      cb.complete(wj.job)
    })
    cb
  }

  override def onWorkerUpdateAsync(wu: WorkerUpdate,
                                   __current: Current): CompletableFuture[Void] = {
    import EnumJobState._
    import JobStates._
    val updated = new util.ArrayList[JobDTO]()
    for {
      u <- wu.updates
      wj <- graph.jobs.get(u.id)
    } {
      val newState = (wj.state, u.state) match {
        case (x, y) if !x.isTerminal && y.isTerminal => {
          assert(!wj.jobState.currentWorker.isEmpty, "Logic error")
          val wid = wj.jobState.currentWorker.head
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
    if (!updated.isEmpty) {
      checkStartableStates(updated)
      onUpdate(updated.toArray(Array.empty[JobDTO]))
    }
    CompletableFuture.completedFuture(null)
  }

  override def setStateAsync(id: JobId, state: EnumJobState, __current: Current): CompletableFuture[Void] = {
    println(s"Trying to Setting state $id $state")
    val cb = new CompletableFuture[Void]()
    withJobOnExecutor(id, cb, (wj) => {
      println(s"Setting state $id $state")
      wj.state = state
      cb.complete(null)
    })
    cb
  }
}
