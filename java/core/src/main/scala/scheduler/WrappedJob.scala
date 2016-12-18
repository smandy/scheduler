package scheduler

object WrappedJob {
  import scheduler.EnumJobState.State._

  val startableStates = {
    Set(STARTABLE)
  }

  val stoppableStates = {
    Set(STARTED)
  }
}

class WrappedJob(val job : Job) {

  val jobState = new JobState()

  def state = jobState.state

  def state_=( s : scheduler.EnumJobState.State) : Unit = {
    jobState.state = s
  }

  def priority = job.priority

  def isStartable = WrappedJob.startableStates.contains(state)
}
