import Ice

from gemHelper import Gem
from graphGenerator import doGraph

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")
server = Gem.GemServerPrx.checkedCast( prx )

from exampleBatch import makeBatch

batch = makeBatch()

print "Sending"
server.reset()
server.submitBatch( batch )
print "sent"
doGraph( 'submitter', server)
