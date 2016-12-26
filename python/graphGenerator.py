from schedulerHelper import scheduler
import os

js = scheduler.EnumJobState

template = """\
digraph G2 {
    graph [rankdir = LR];
    node[shape=record,fontname="Liberation"];

%(strEdges)s

%(strNodes)s

}"""

colorForState = {
    js.DORMANT   : 'grey',
    js.READY : 'orange4',
    js.SCHEDULED : 'orange2',
    js.STARTED   : 'orange',
    #js.BLOCKED   : 'gray',
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
        for dep in j.job.dependencies:
            edges.append( "%s -> %s;" % (dep.id, j.job.id.id) )

    for n in jobs:
        nodes.append( '%s [ fillcolor=%s, style = filled];' % (n.job.id.id,colorForState[n.state.state]))
        
    with open(dotFile, 'w') as outFile:
        strEdges = "\n".join(edges)
        strNodes = "\n".join(nodes)
        outFile.write( template % locals())
    import subprocess
    subprocess.Popen( 'dot -Tpng -o %s %s' % (pngFile, dotFile), shell = True).communicate()
