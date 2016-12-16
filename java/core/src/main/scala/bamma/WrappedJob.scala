package bamma

import Gem.{Job, JobState, WorkerId, JobDescription}

object WrappedJob {
  def forJob( j : Job) = new JobDescription(j, JobState.STARTABLE, Array.empty[WorkerId])
}

class WrappedJob(val jd : JobDescription) {
  def state = jd.state

  def start_( s : JobState) = jd.state = s
}
