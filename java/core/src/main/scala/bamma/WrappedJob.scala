package bamma

import Scheduler.{Job, JobState, WorkerId, JobDescription}

object WrappedJob {
  def forJob( j : Job) = new JobDescription(j, JobState.STARTABLE, Array.empty[WorkerId])

  import Scheduler.JobState._

  val startableStates = {
    Set(STARTABLE)
  }

  val stoppableStates = {
    Set(STARTED)
  }
}

class WrappedJob(val job : Job, val state : JobState) {
  def state = jd.state

  def state_=( s : JobState) : Unit = {
    jd.state = s
  }

  def priority = jd.job.priority

  def isStartable = WrappedJob.startableStates.contains(state)
}
