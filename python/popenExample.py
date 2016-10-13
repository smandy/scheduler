import time
#import select
import subprocess

f = open( '/tmp/counter.txt', 'w')
d5 = subprocess.Popen( ['./counter.py'], stdout = f, stderr = subprocess.STDOUT)

while True:
    x = d5.poll()
    print x, "Beep"
    if not x is None:
        break
    time.sleep(2)
    
