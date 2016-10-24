import uuid
import Ice

from gemHelper import Gem
from graphGenerator import doGraphFromJobs
import os

#communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])
communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")
server = Gem.GemServerPrx.checkedCast( prx )
imageDir = os.path.expanduser( communicator.getProperties().getProperty('GemListener.ImageDir'))

def onFail(self, ex, current):
    print "Failed %s" % ex
    
class MyListener(Gem.GemServerListener):
    def __init__(self):
        self.imgCounter = 0

    def onImageReady_async(self, cb, s, current):
        print "onImageReady %s" % s
        cb.ice_response()
        
    def onImage_async(self, cb, image, current):
        print "Woot got image %s" % ",".join( [x.id for x in image.jobs] )
        cb.ice_response()
        self.onJobs(image.jobs)
        print "done"
        
    def onJobs(self,jobs):
        suffix = 'image_%03d' % self.imgCounter
        doGraphFromJobs(suffix, jobs, prefix = imageDir)
        self.imgCounter = (self.imgCounter + 1 ) % 4
        server.begin_imageReady( 'images/%s.png' % suffix)
        
    def onUpdate_async(self, cb, update, current):
        print "Woot got udpate %s" % ",".join( [x.id for x in update] )
        cb.ice_response()
        server.begin_getJobs( _response = self.onJobs, _ex = onFail)
        #print "done"
        
adapter = communicator.createObjectAdapterWithEndpoints( str(uuid.uuid4()),  "tcp -h localhost")
adapter.activate()

myListener = MyListener()
myPrx = adapter.addWithUUID( myListener )

print "MyPrx is %s" % myPrx
listenerPrx = Gem.GemServerListenerPrx.uncheckedCast(myPrx)

server.addListener(listenerPrx)
communicator.waitForShutdown()
