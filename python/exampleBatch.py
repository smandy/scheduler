import Ice

from schedulerHelper import scheduler

# An example batch.

def m(s):
    return scheduler.JobId( s,'batch')

def makeBatch():
    job1 = scheduler.Job( m('id01'), [],  5)
    job2 = scheduler.Job( m('id02'), [m('id01')],  2)
    job3 = scheduler.Job( m('id03'), [m('id01'), m('id02')],  2)

    job4 = scheduler.Job( m('id04'), [m('id03')],  2)
    pt1 = scheduler.Job( m('pt1'), [m('id04')],  2)
    pt2 = scheduler.Job( m('pt2'), [m('pt1')],  2)

    reduces = [ scheduler.Job( m('mr%s' % i), [m('pt2')],  2) for i in range(5) ]
    rids = [ x.id for x in reduces]

    job5 = scheduler.Job( m('id05'), [m('id03')] + rids,  2)
    job6 = scheduler.Job( m('id06'), [m('id03')],  2)
    job7 = scheduler.Job( m('id07'), [m('id03')],  2)


    job8 = scheduler.Job( m('id08'), [m('id05'), m('id06')],  1)
    job9 = scheduler.Job( m('id09'), [m('id06'), m('id07')],  2)

    job10 = scheduler.Job( m('id10'), [m('id08'), m('id09')],  2)
    job11 = scheduler.Job( m('id11'), [m('id08'), m('id09')],  2)

    job12 = scheduler.Job( m('id12'), [m('id05'), m('id10')],  2)
    job13 = scheduler.Job( m('id13'), [m('id02'), m('id03') ,m('id10')],  2)
    job14 = scheduler.Job( m('id14'), [m('id10'), m('id11')],  2)

    terminal = scheduler.Job( m('terminal'), [m('id12'),m('id13'),m('id14')],  2)

    batch = scheduler.Batch()

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
