import Ice

Ice.loadSlice('../slice/Gem.ice')

print "Woot"

import Gem

communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")

server = Gem.GemServerPrx.checkedCast( prx )


job1 = Gem.Job( 'id1', Gem.JobState.STARTABLE )
job2 = Gem.Job( 'id2', Gem.JobState.STARTABLE )

job2.dependencies = [job1.id]

batch = Gem.Batch()
batch.jobs = [job1, job2]

print "Sending"
server.submitBatch( batch )
print "sent"
