package scheduler

object JobStates {
  import scheduler.EnumJobState.State._

  val startable = {
    Set(STARTABLE)
  }

  val stopabble = {
    Set(STARTED)
  }
}
