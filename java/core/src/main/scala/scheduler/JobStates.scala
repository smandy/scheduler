package scheduler

object JobStates {
  import scheduler.EnumJobState._

  val terminal = {
    Set(FAILED,COMPLETED,CANCELLED)
  }

  val startable = {
    Set(READY)
  }

  val stoppable = {
    Set(STARTED, SCHEDULED)
  }

  implicit class PimpState( val state : EnumJobState) {
    val isTerminal = terminal.contains(state)
    val isStartable = startable.contains(state)
    val isStoppable = stoppable.contains(state)
  }

}
