import random
import Ice

Ice.loadSlice('../slice/Gem.ice')
print "Woot"
import Gem

#communicator = Ice.initialize(['--Ice.Config=../config/gemserver.config'])
communicator = Ice.initialize(['--Ice.Config=../config/client.config'])
prx = communicator.propertyToProxy("GemServer.Proxy")

server = Gem.GemServerPrx.checkedCast( prx )
server.reset()

job1 = Gem.Job( 'id1', [], Gem.JobState.STARTABLE , 5)
job2 = Gem.Job( 'id2', ['id1'], Gem.JobState.STARTABLE , 2)
job3 = Gem.Job( 'id3', ['id1', 'id2'], Gem.JobState.STARTABLE , 2)

#naughty = Gem.Job( 'bad', [], Gem.JobState.FAILED , 2)

job4 = Gem.Job( 'id4', ['id3'], Gem.JobState.STARTABLE , 2)
pt1 = Gem.Job( 'pt1', ['id4'], Gem.JobState.STARTABLE , 2)
pt2 = Gem.Job( 'pt2', ['pt1'], Gem.JobState.STARTABLE , 2)

reduces = [ Gem.Job( 'mr%s' % i, ['pt2'], Gem.JobState.STARTABLE , 2) for i in range(5) ]
rids = [ x.id for x in reduces]

job5 = Gem.Job( 'id5', ['id3'] + rids, Gem.JobState.STARTABLE , 2)
job6 = Gem.Job( 'id6', ['id3'], Gem.JobState.STARTABLE , 2)
job7 = Gem.Job( 'id7', ['id3'], Gem.JobState.STARTABLE , 2)


job8 = Gem.Job( 'id8', ['id5', 'id6'], Gem.JobState.STARTABLE , 1)
job9 = Gem.Job( 'id9', ['id6', 'id7'], Gem.JobState.STARTABLE , 2)

job10 = Gem.Job( 'id10', ['id8', 'id9'], Gem.JobState.STARTABLE , 2)
job11 = Gem.Job( 'id11', ['id8', 'id9'], Gem.JobState.STARTABLE , 2)

job12 = Gem.Job( 'id12', ['id5', 'id10'], Gem.JobState.STARTABLE , 2)
job13 = Gem.Job( 'id13', ['id2', 'id3' ,'id10'], Gem.JobState.STARTABLE , 2)
job14 = Gem.Job( 'id14', ['id10', 'id11'], Gem.JobState.STARTABLE , 2)

terminal = Gem.Job( 'terminal', ['id12','id13','id14'], Gem.JobState.STARTABLE , 2)

batch = Gem.Batch()
batch.jobs = [job1, job2, job3, job4, job5,
              job6, job7, job8, job9 , job10, job11,
              job12, job13, job14, pt1, pt2, terminal ] + reduces

print "Sending"
server.submitBatch( batch )
print "sent"

wid = Gem.WorkerId("worker1")

if 0:
    jobs1 = server.getStartableJob(wid)
    jobs2 = server.getStartableJob(wid)
    jobs3 = server.getStartableJob(wid)
    print jobs1, jobs2, jobs3

js = server.getJobs()
import os

template = """
digraph G2 {
    graph [rankdir = LR];
    node[shape=record,fontname="Liberation"];

%(strEdges)s

%(strNodes)s

}
"""

dn = os.path.expanduser('~/jobs')
for x in os.listdir( dn):
    print x
    os.remove( '%s/%s' % (dn,x))

js = Gem.JobState

colorForState = {
    js.STARTABLE : 'orange',
    js.STARTED   : 'cyan',
    js.BLOCKED   : 'gray',
    js.COMPLETED : 'green',
    js.FAILED    : 'red'
    }

def doComplete(jid):
    setState(jid, js.COMPLETED)

def setState(jid,s):
    s = Gem.JobWorkerState(wid, jid, s)
    server.onWorkerStates( [s])
    
def stateSummary():
    xs = server.getJobs()
    for x in xs:
        print x.state, x.id

def doGraph(t):
    #stateSummary()
    print t
    dotFile = os.path.expanduser( '~/jobs/%s.dot' % t)
    pngFile = os.path.expanduser( '~/jobs/%s.png' % t)

    edges = []
    nodes = []

    xs = server.getJobs()
    for j in xs:
        for dep in j.dependencies:
            edges.append( "%s -> %s;\n" % (dep, j.id) )

    for n in xs:
        nodes.append( '%s [ fillcolor=%s, style = filled];' % (n.id,colorForState[n.state]))

    with open(dotFile, 'w') as outFile:
        strEdges = "\n".join(edges)
        strNodes = "\n".join(nodes)
        outFile.write( template % locals())
    import subprocess
    subprocess.Popen( 'dot -Tpng -o %s %s' % (pngFile, dotFile), shell = True)

n = 0
doGraph( 'state%03d' % n)
n += 1

while True:
    jobs = server.getJobs()
    if set( [ x.state for x in jobs ] ) == set( [ js.COMPLETED ] ):
        break
    fails = [ x for x in jobs if x.state == js.FAILED ]
    jobs1 = []
    
    for i in range(5):
        newJobs = server.getStartableJob(wid)
        if not newJobs:
            break
        jobs1 += newJobs
        
    if not jobs1 and fails:
        # lets pretend Manual intervention
        setState( fails[0].id, js.STARTABLE)
        
    doGraph( 'state%03d' % n)
    n += 1
    for x in jobs1:
        state = random.choice( [ js.FAILED, js.COMPLETED,
                                 js.COMPLETED, js.COMPLETED] )
        setState( x.id, state )
    doGraph( 'state%03d' % n)
    n += 1
        
