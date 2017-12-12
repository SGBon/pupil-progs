# spoofs pupil gaze data at random points

import sys

sig = sys.argv[1] # set signature to first argument
distance = 4000 # distance in mm

from socketIO_client import SocketIO, LoggingNamespace
import time
import random

random.seed()

def frange(start,stop,step):
	i = start
	while i < stop:
		yield i
		i += step

with SocketIO('localhost',3000,LoggingNamespace) as socketIO:
	while True:
		for i in frange(0.2,0.8,0.1):
			for j in frange(0.1,0.8,0.01):
				eyes = [j,i]
				eyes.append(sig) # add signature to data
				eyes.append(distance)
				socketIO.emit('eye pos',eyes)
				time.sleep(0.1)

socketIO.wait(1)
