#!/usr/bin/env python

import Ice

Ice.loadSlice( '..', '../slice/gem.ice')

import Gem
import sys

communicator = Ice.initialize(sys.argv)
prx = communicator.propertyToProxy("GemServer.Proxy")

myId = Gem.WorkerId( communicator.getProperties().getProperty('Worker.ID') )
maxJobs = communicator.getProperties().getPropertyAsInt( 'Worker.MaxJobs')

jobs = []

while True:
    if len(jobs) < maxJobs:
        newJob = prx.getJobs()
