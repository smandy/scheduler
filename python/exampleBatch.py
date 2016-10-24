import Ice

from gemHelper import Gem

# An example batch.

def makeBatch():
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


    tmpJobs = [job1, job2, job3, job4, job5,
               job6, job7, job8, job9 , job10, job11,
               job12, job13, job14, pt1, pt2, terminal ] + reduces

    def decorate(j):
        j.cmdLine = ['./counter.py']
        j.pwd = '/home/andy/repos/gem/python'
        return j
    
    batch.jobs = [ decorate(j) for j in tmpJobs ]
    return batch

if __name__=='__main__':
    b = makeBatch()
    print b
