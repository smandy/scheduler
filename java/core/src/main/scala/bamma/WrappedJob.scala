package bamma

import Gem.{Job, JobState, WorkerId, JobDescription}

object WrappedJob {
  def forJob( j : Job) = new JobDescription(j, JobState.STARTABLE, Array.empty[WorkerId])

  import Gem.JobState._

  val startableStates = {
    Set(STARTABLE)
  }

  val stoppableStates = {
    Set(STARTED)
  }
}

class WrappedJob(val jd : JobDescription) {

  def state = jd.state

  def state_=( s : JobState) : Unit = {
    jd.state = s
  }

  def priority = jd.job.priority

  def isStartable = WrappedJob.startableStates.contains(state)
}
