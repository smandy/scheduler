import uuid
import Ice

from gemHelper import Gem
from graphGenerator import doGraphFromJobs
import os
from datetime import datetime

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
        print "%s onImageReady %s" % (datetime.now().isoformat(), s)
        cb.ice_response()
        
    def onImage_async(self, cb, image, current):
        print "%s Woot got image %s" % (datetime.now().isoformat(), ",".join( [x.id for x in image.jobs] ))
        cb.ice_response()
        self.onJobs(image.jobs)
        print "done"
        
    def onJobs(self,jobs):
        suffix = 'image_%03d' % self.imgCounter
        doGraphFromJobs(suffix, jobs, prefix = imageDir)
        self.imgCounter = (self.imgCounter + 1 ) % 4
        img =  'images/%s.png' % suffix
        print "%s notifying server of image ready %s" % (datetime.now().isoformat() , img)
        server.begin_imageReady(img)
        
    def onUpdate_async(self, cb, update, current):
        print "%s Woot got udpate %s" % (datetime.now().isoformat(), ",".join( [x.id for x in update] ))
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
