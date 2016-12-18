import Ice

from schedulerHelper import Scheduler
from graphGenerator import doGraph

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("SchedulerServer.Proxy")
server = Scheduler.SchedulerServerPrx.checkedCast( prx )

from exampleBatch import makeBatch

batch = makeBatch()

print "Sending"
server.reset()
server.submitBatch( batch )
print "sent"
doGraph( 'submitter', server)
