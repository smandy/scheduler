import Ice

from gemHelper import Gem

# An example batch.

def makeBatch():
    job1 = Gem.Job( 'id01', [], Gem.JobState.STARTABLE , 5)
    job2 = Gem.Job( 'id02', ['id01'], Gem.JobState.STARTABLE , 2)
    job3 = Gem.Job( 'id03', ['id01', 'id02'], Gem.JobState.STARTABLE , 2)

    #naughty = Gem.Job( 'bad', [], Gem.JobState.FAILED , 2)

    job4 = Gem.Job( 'id04', ['id03'], Gem.JobState.STARTABLE , 2)
    pt1 = Gem.Job( 'pt1', ['id04'], Gem.JobState.STARTABLE , 2)
    pt2 = Gem.Job( 'pt2', ['pt1'], Gem.JobState.STARTABLE , 2)

    reduces = [ Gem.Job( 'mr%s' % i, ['pt2'], Gem.JobState.STARTABLE , 2) for i in range(5) ]
    rids = [ x.id for x in reduces]

    job5 = Gem.Job( 'id05', ['id03'] + rids, Gem.JobState.STARTABLE , 2)
    job6 = Gem.Job( 'id06', ['id03'], Gem.JobState.STARTABLE , 2)
    job7 = Gem.Job( 'id07', ['id03'], Gem.JobState.STARTABLE , 2)


    job8 = Gem.Job( 'id08', ['id05', 'id06'], Gem.JobState.STARTABLE , 1)
    job9 = Gem.Job( 'id09', ['id06', 'id07'], Gem.JobState.STARTABLE , 2)

    job10 = Gem.Job( 'id10', ['id08', 'id09'], Gem.JobState.STARTABLE , 2)
    job11 = Gem.Job( 'id11', ['id08', 'id09'], Gem.JobState.STARTABLE , 2)

    job12 = Gem.Job( 'id12', ['id05', 'id10'], Gem.JobState.STARTABLE , 2)
    job13 = Gem.Job( 'id13', ['id02', 'id03' ,'id10'], Gem.JobState.STARTABLE , 2)
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
