package scheduler

object WorkerJobStates {

  import EnumWorkerJobState.State._
  val terminal = Set(FAILED, COMPLETED, CANCELED)
}
