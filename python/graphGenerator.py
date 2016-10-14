from gemHelper import Gem
import os

js = Gem.JobState

template = """
digraph G2 {
    graph [rankdir = LR];
    node[shape=record,fontname="Liberation"];

%(strEdges)s

%(strNodes)s

}
"""

colorForState = {
    js.STARTABLE : 'orange4',
    js.SCHEDULED : 'orange2',
    js.STARTED   : 'orange',
    js.BLOCKED   : 'gray',
    js.COMPLETED : 'green',
    js.FAILED    : 'red'
    }


def doGraph(t, server):
    jobs = server.getJobs()
    doGraphFromJobs(t, jobs)

def doGraphFromJobs(t, jobs):
    #stateSummary()
    print t
    dotFile = os.path.expanduser( '~/jobs/%s.dot' % t)
    pngFile = os.path.expanduser( '~/jobs/%s.png' % t)

    edges = []
    nodes = []
    for j in jobs:
        for dep in j.dependencies:
            edges.append( "%s -> %s;\n" % (dep, j.id) )

    for n in jobs:
        nodes.append( '%s [ fillcolor=%s, style = filled];' % (n.id,colorForState[n.state]))
        
    with open(dotFile, 'w') as outFile:
        strEdges = "\n".join(edges)
        strNodes = "\n".join(nodes)
        outFile.write( template % locals())
    import subprocess
    subprocess.Popen( 'dot -Tpng -o %s %s' % (pngFile, dotFile), shell = True)
