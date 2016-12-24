package scheduler

// Job is immutable
class WrappedJob(val job : Job) {

  val jobState = new JobState(job.id,
    EnumJobState.State.DORMANT,
    Array.empty[WorkerId],
    Array.empty[JobStateDescription],
    Array.empty[WorkerStateDescription] )

  def state = jobState.state

  def state_=( s : scheduler.EnumJobState.State) : Unit = {
    jobState.state = s
  }

  def priority = job.priority

  def isStartable = JobStates.startable.contains(state)
}
