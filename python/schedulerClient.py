import random
import exampleBatch

import Ice
from schedulerHelper import scheduler
from graphGenerator import doGraph

communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("SchedulerServer.Proxy")
server = scheduler.SchedulerServerPrx.checkedCast( prx )

server.reset()

batch = exampleBatch.makeBatch()

print "Sending"
server.submitBatch( batch )
print "sent"

wid = scheduler.WorkerId("worker1")

if 0:
    jobs1 = server.getStartableJob(wid)
    jobs2 = server.getStartableJob(wid)
    jobs3 = server.getStartableJob(wid)
    print jobs1, jobs2, jobs3

js = server.getJobs()

print "Got jobs"
import os

dn = os.path.expanduser('~/jobs')
for x in os.listdir( dn):
    print x
    os.remove( '%s/%s' % (dn,x))

js = scheduler.EnumJobState

def doComplete(jid):
    workerUpdate(jid, js.COMPLETED)

def workerUpdate(jid,s):
    """Use the cheeky set method"""
    print "Worker update %s %s" % (jid, s)
    wsd = scheduler.WorkerStateDescription(jid, s)
    s = scheduler.WorkerUpdate( wid, [ wsd ] )
    server.onWorkerUpdate(s)

def setState(jid,s):
    """Use the cheeky set method"""
    print "SetState %s %s" % (jid, s)
    server.setState( jid, s)
    
def stateSummary():
    xs = server.getJobs()
    for x in xs:
        print x.state, x.id

n = 0
doGraph( 'state%03d' % n, server)
n += 1

while True:
    jobs = server.getJobs()
    if set( [ x.state.state for x in jobs ] ) == set( [ js.COMPLETED ] ):
        break
    fails = [ x for x in jobs if x.state.state == js.FAILED ]
    jobs1 = []
    
    print "In loop %s fails" % len(fails)
    for i in range(5):
        #print "Get job"
        newJobs = server.getStartableJob(wid)
        #print "Got %s" % len(newJobs)
        if not newJobs:
            break
        jobs1 += newJobs
        
    if not jobs1 and fails:
        # lets pretend Manual intervention
        print "Doing setstate"
        setState( fails[0].job.id, js.READY)
        
    doGraph( 'state%03d' % n, server)
    n += 1
    
    for x in jobs1:
        state = random.choice( [ js.FAILED, js.COMPLETED,js.COMPLETED ] )
        workerUpdate( x.id, state)
    doGraph( 'state%03d' % n, server)
    n += 1
        

