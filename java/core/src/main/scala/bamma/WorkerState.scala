package bamma

import Gem.{JobState, WorkerId}

class WorkerState(val workerId : WorkerId) {
  var jobStates = Set[JobState]()
}
