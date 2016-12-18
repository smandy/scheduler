package bamma

import Scheduler.{JobState, WorkerId}

class WorkerState(val workerId : WorkerId) {
  var jobStates = Set[JobState]()
}
