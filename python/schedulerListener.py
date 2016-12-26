import uuid
import Ice

from schedulerHelper import scheduler
from graphGenerator import doGraphFromJobs
import os
from datetime import datetime

#communicator = Ice.initialize(['--Ice.Config=../config/schedulerserver.config'])
communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("SchedulerServer.Proxy")
server = scheduler.SchedulerServerPrx.checkedCast( prx )
imageDir = os.path.expanduser( communicator.getProperties().getProperty('SchedulerListener.ImageDir'))

def onFail(self, ex, current):
    print "Failed %s" % ex
    
class MyListener(scheduler.SchedulerServerListener):
    def __init__(self):
        self.imgCounter = 0

    def onImageReady_async(self, cb, s, current):
        print "%s onImageReady %s" % (datetime.now().isoformat(), s)
        cb.ice_response()
        
    def onImage_async(self, cb, image, current):
        print "%s Woot got image %s" % (datetime.now().isoformat(), ",".join( [x.job.id.id for x in image.jobs] ))
        cb.ice_response()
        self.onJobs(image.jobs)
        print "done"
        
    def onJobs(self,jobs):
        suffix = 'image_%03d' % self.imgCounter
        doGraphFromJobs(suffix, jobs, prefix = imageDir)
        self.imgCounter = (self.imgCounter + 1 ) % 4
        img =  'images/%s.png' % suffix
        print "%s notifying server of image ready %s" % (datetime.now().isoformat() , img)
        server.begin_imageReady('woot', img)
        
    def onUpdate_async(self, cb, update, current):
        print "%s Woot got udpate %s" % (datetime.now().isoformat(), ",".join( [x.job.id.id for x in update] ))
        cb.ice_response()
        server.begin_getJobs( _response = self.onJobs, _ex = onFail)
        #print "done"
        
adapter = communicator.createObjectAdapterWithEndpoints( str(uuid.uuid4()),  "tcp -h localhost")
adapter.activate()

myListener = MyListener()
myPrx = adapter.addWithUUID( myListener )

print "MyPrx is %s" % myPrx
listenerPrx = scheduler.SchedulerServerListenerPrx.uncheckedCast(myPrx)

server.addListener(listenerPrx)
communicator.waitForShutdown()
