#!/usr/bin/env python

# Simple counter for diagnostic purposes

import time
import sys
counter = 0

while True:
    print counter, '...'
    sys.stdout.flush()
    counter += 1
    if counter > 5:
        break
    time.sleep(1)
    
