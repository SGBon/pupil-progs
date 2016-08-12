# spoofs pupil gaze data at random points

import sys

sig = sys.argv[1] # set signature to first argument

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
		for i in frange(0.1,1.0,0.05):
			for j in frange(0.0,1.0,0.1):
		#x = random.random()*0.58
		#y = random.random()*0.78
		#eyes = [x,1 - y]
				eyes = [j,1-i]
				eyes.append(sig) # add signature to data
				socketIO.emit('eye pos',eyes)
				time.sleep(0.1)

socketIO.wait(1)
