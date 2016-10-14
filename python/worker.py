#!/usr/bin/env python

import Ice
import os
import sys
import time
import subprocess
from datetime import datetime

Ice.loadSlice( '../slice/gem.ice')
import Gem

print "Goo"
print "Foo"

if 'INSIDE_EMACS' in os.environ:
    # Andy Testing
    args = [ '--Ice.Config=../config/worker.config' ]
else:
    args = sys.argv
    
communicator = Ice.initialize(args)

def checkedProperty(s, p = communicator.getProperties()):
    ret = p.getProperty(s)
    if ret=='':
        raise ValueError('Missing string property %s' % s)
    return ret

defaultDir = os.path.expanduser('~/worker')
if not os.path.isdir(defaultDir):
    os.makedirs(defaultDir)
i = 0
while True:
    dirName = "%s/%05d" % (defaultDir, i)
    if not os.path.isdir(dirName):
        os.makedirs(dirName)
        break
    i += 1
print "Dirname is %s" % dirName

prx = communicator.propertyToProxy("GemServer.Proxy").ice_timeout(1000)
server = Gem.GemServerPrx.checkedCast( prx )
print server

myId    = Gem.WorkerId( checkedProperty('Worker.ID'))
maxJobs = int( checkedProperty('Worker.MaxJobs' ))
jobs = []

# Cue up a batch - don't use in real world!
import exampleBatch
server.reset()
batch = exampleBatch.makeBatch()
server.submitBatch(batch)

class Job(object):
    __slots__= ['job','p']
    def __init__(self, job):
        self.job = job
        f = open('%s/%s.txt' % (dirName, job.id), 'w')
        #print f
        #print job
        self.p = subprocess.Popen( job.cmdLine,
                                   cwd = job.pwd,
                                   env = job.env,
                                   stdout = f,
                                   stderr = f, shell = True)
        
    @property
    def finished(self):
        return self.p.poll() is not None

    @property
    def succeeded(self):
        return self.finished and self.p.poll() == 0

    @property
    def id(self):
        return self.job.id
    
    def __str__(self):
        return str(self.job)

js = Gem.JobState
while True:
    dirty = False
    newStates = []
    while len(jobs) < maxJobs:
        newJob = server.getStartableJob( myId )
        if newJob:
            print "Got job %s" % newJob[0].id
            newJob[0].state = js.STARTED
            jobs.append( Job(newJob[0]))
            newStates.append( Gem.JobWorkerState(myId, newJob[0].id, js.STARTED))
            dirty = True
            break
        else:
            break
    # Check status of current jobs
    newJobs = []
    for job in jobs:
        if job.finished:
            if job.succeeded:
                state = js.COMPLETED
            else:
                state = js.FAILED
            print "Job %s %s" % (job.id, state)
            s = Gem.JobWorkerState(myId, job.id, state)
            newStates.append(s)
            dirty = True
        else:
            newJobs.append(job)
    if newStates:
        server.onWorkerStates( newStates )
    jobs = newJobs
    if dirty:
        print datetime.now().isoformat(), len(jobs), [x.id for x in jobs ]
    time.sleep(0.5)
