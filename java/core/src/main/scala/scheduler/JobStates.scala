package scheduler

object JobStates {
  import scheduler.EnumJobState.State._

  val terminal = {
    Set(FAILED,COMPLETED,CANCELLED)
  }

  val startable = {
    Set(STARTABLE)
  }

  val stopabble = {
    Set(STARTED)
  }
}
