import Ice

from schedulerHelper import Scheduler

# An example batch.

def makeBatch():
    job1 = Scheduler.Job( 'id01', [], Scheduler.JobState.STARTABLE , 5)
    job2 = Scheduler.Job( 'id02', ['id01'], Scheduler.JobState.STARTABLE , 2)
    job3 = Scheduler.Job( 'id03', ['id01', 'id02'], Scheduler.JobState.STARTABLE , 2)

    #naughty = Scheduler.Job( 'bad', [], Scheduler.JobState.FAILED , 2)

    job4 = Scheduler.Job( 'id04', ['id03'], Scheduler.JobState.STARTABLE , 2)
    pt1 = Scheduler.Job( 'pt1', ['id04'], Scheduler.JobState.STARTABLE , 2)
    pt2 = Scheduler.Job( 'pt2', ['pt1'], Scheduler.JobState.STARTABLE , 2)

    reduces = [ Scheduler.Job( 'mr%s' % i, ['pt2'], Scheduler.JobState.STARTABLE , 2) for i in range(5) ]
    rids = [ x.id for x in reduces]

    job5 = Scheduler.Job( 'id05', ['id03'] + rids, Scheduler.JobState.STARTABLE , 2)
    job6 = Scheduler.Job( 'id06', ['id03'], Scheduler.JobState.STARTABLE , 2)
    job7 = Scheduler.Job( 'id07', ['id03'], Scheduler.JobState.STARTABLE , 2)


    job8 = Scheduler.Job( 'id08', ['id05', 'id06'], Scheduler.JobState.STARTABLE , 1)
    job9 = Scheduler.Job( 'id09', ['id06', 'id07'], Scheduler.JobState.STARTABLE , 2)

    job10 = Scheduler.Job( 'id10', ['id08', 'id09'], Scheduler.JobState.STARTABLE , 2)
    job11 = Scheduler.Job( 'id11', ['id08', 'id09'], Scheduler.JobState.STARTABLE , 2)

    job12 = Scheduler.Job( 'id12', ['id05', 'id10'], Scheduler.JobState.STARTABLE , 2)
    job13 = Scheduler.Job( 'id13', ['id02', 'id03' ,'id10'], Scheduler.JobState.STARTABLE , 2)
    job14 = Scheduler.Job( 'id14', ['id10', 'id11'], Scheduler.JobState.STARTABLE , 2)

    terminal = Scheduler.Job( 'terminal', ['id12','id13','id14'], Scheduler.JobState.STARTABLE , 2)

    batch = Scheduler.Batch()


    tmpJobs = [job1, job2, job3, job4, job5,
               job6, job7, job8, job9 , job10, job11,
               job12, job13, job14, pt1, pt2, terminal ] + reduces

    def decorate(j):
        j.cmdLine = ['./counter.py']
        j.pwd = '/home/andy/repos/scheduler/python'
        return j
    
    batch.jobs = [ decorate(j) for j in tmpJobs ]
    return batch

if __name__=='__main__':
    b = makeBatch()
    print b
