package scheduler

class WorkerState(val workerId : WorkerId) {
  var jobStates = Set[JobState]()
}
