import uuid
import Ice

Ice.loadSlice('../slice/Gem.ice')
#print "Woot"

import Gem

#communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")

server = Gem.GemServerPrx.checkedCast( prx )

class MyListener(Gem.GemServerListener):
    def onImage_async(self, cb, image, current):
        print "Woot got image"
        cb.ice_response()
        print "done"

    def onUpdate_async(self, cb, update, current):
        print "Woot got udpate"
        cb.ice_response()
        print "done"

        
adapter = communicator.createObjectAdapterWithEndpoints( str(uuid.uuid4()),  "tcp -h localhost")
adapter.activate()

myListener = MyListener()
myPrx = adapter.addWithUUID( myListener)

print "MyPrx is %s" % myPrx
listenerPrx = Gem.GemServerListenerPrx.uncheckedCast(myPrx)

server.addListener(listenerPrx)

communicator.waitForShutdown()
