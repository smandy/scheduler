import uuid
import Ice

from gemHelper import Gem
from graphGenerator import doGraphFromJobs

#communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")
server = Gem.GemServerPrx.checkedCast( prx )

def onJobs(jobs):
    doGraphFromJobs('listener', jobs)

def onFail(self, ex, current):
    print "Failed %s" % ex
    
class MyListener(Gem.GemServerListener):
    def onImage_async(self, cb, image, current):
        print "Woot got image %s" % ",".join( [x.id for x in image.jobs] )
        cb.ice_response()
        doGraphFromJobs('listener',image.jobs)
        print "done"

    def onUpdate_async(self, cb, update, current):
        print "Woot got udpate %s" % ",".join( [x.id for x in update] )
        cb.ice_response()
        server.begin_getJobs( _response = onJobs, _ex = onFail)
        #print "done"
        
adapter = communicator.createObjectAdapterWithEndpoints( str(uuid.uuid4()),  "tcp -h localhost")
adapter.activate()

myListener = MyListener()
myPrx = adapter.addWithUUID( myListener)

print "MyPrx is %s" % myPrx
listenerPrx = Gem.GemServerListenerPrx.uncheckedCast(myPrx)

server.addListener(listenerPrx)
communicator.waitForShutdown()
