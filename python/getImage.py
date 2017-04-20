import random
import exampleBatch

import Ice
from schedulerHelper import scheduler
from graphGenerator import doGraph

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("SchedulerServer.Proxy")
server = scheduler.SchedulerServerPrx.checkedCast( prx )

