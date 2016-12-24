package scheduler

object Graph {
  def forJobs(jobs: Map[JobId, Node]): Either[JobCycleDetected, Graph] = {
    assert(jobs.forall(_._2.job.dependencies.forall(jobs.contains)), "Malformed graph")

    def hasDependency(jobId: JobId, dep: JobId): Boolean = {
      val depList = jobs(jobId).job.dependencies
      depList.contains(dep) || depList.exists(x => hasDependency(x, dep))
    }

    val badNodes = jobs.keys.filter(x => hasDependency(x, x))

    if (badNodes.nonEmpty) {
      Left(new JobCycleDetected(badNodes.toArray))
    } else {
      Right(Graph(jobs))
    }
  }
}

case class Graph private(val jobs: Map[JobId, Node]) {
  lazy val dependencies = for {
    (q, v) <- jobs
    j = jobs(q)
    deps = v.job.dependencies.map(jobs(_))
  } yield {
    (j, deps.toSet)
  }

  lazy val dependents: Map[Node, Set[Node]] = {
    val tups = for {
      (q, v) <- jobs.toIterable
      j = jobs(q)
      dep <- v.job.dependencies
      n = jobs(dep)
    } yield {
      (j, n)
    }
    val grouped = tups.groupBy(_._2)
    grouped.map(x => (x._1, x._2.map(_._2).toSet))
  }

  /*
    def invalidateImpl( toKill : java.util.Set[WrappedJob]) : Unit = {
      if (EnumJobState.State.STARTED == state) {
        toKill.add( this )
      }
      state = EnumJobState.State.DORMANT
      dependencies.foreach( _.invalidateImpl(toKill))
    }*/

  /*  def hasDependency( wj : WrappedJob) : Boolean = {
      !dependencies.contains(wj) && dependencies.forall( d => !d.hasDependency(wj))
    }*/
}

