from gemHelper import Gem
import os

js = Gem.JobState

template = """\
digraph G2 {
    graph [rankdir = LR];
    node[shape=record,fontname="Liberation"];

%(strEdges)s

%(strNodes)s

}"""

colorForState = {
    js.STARTABLE : 'orange4',
    js.SCHEDULED : 'orange2',
    js.STARTED   : 'orange',
    js.BLOCKED   : 'gray',
    js.COMPLETED : 'green',
    js.FAILED    : 'red'
    }


def doGraph(t, server, prefix = "~/jobs"):
    jobs = server.getJobs()
    doGraphFromJobs(t, jobs, prefix = prefix)

def doGraphFromJobs(t, jobs, prefix = '~/jobs'):
    #stateSummary()
    print t
    dotFile = os.path.expanduser( '%s/%s.dot' % (prefix, t))
    pngFile = os.path.expanduser( '%s/%s.png' % (prefix, t))

    edges = []
    nodes = []
    for j in jobs:
        for dep in j.dependencies:
            edges.append( "%s -> %s;" % (dep, j.id) )

    for n in jobs:
        nodes.append( '%s [ fillcolor=%s, style = filled];' % (n.id,colorForState[n.state]))
        
    with open(dotFile, 'w') as outFile:
        strEdges = "\n".join(edges)
        strNodes = "\n".join(nodes)
        outFile.write( template % locals())
    import subprocess
    subprocess.Popen( 'dot -Tpng -o %s %s' % (pngFile, dotFile), shell = True).communicate()
