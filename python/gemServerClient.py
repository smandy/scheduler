import Ice

Ice.loadSlice('../slice/Gem.ice')
print "Woot"

import Gem

#communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")

server = Gem.GemServerPrx.checkedCast( prx )

job1 = Gem.Job( 'id1', 5, Gem.JobState.STARTABLE )
job2 = Gem.Job( 'id2', 2, Gem.JobState.STARTABLE )

job2.dependencies = [job1.id]
batch = Gem.Batch()
batch.jobs = [job1, job2]

print "Sending"
server.submitBatch( batch )
print "sent"

wid = Gem.WorkerId("worker1")

jobs1 = server.getStartableJob(wid)
jobs2 = server.getStartableJob(wid)
jobs3 = server.getStartableJob(wid)

print jobs1, jobs2, jobs3
