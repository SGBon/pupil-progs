# spoofs pupil gaze data at random points
from socketIO_client import SocketIO, LoggingNamespace
import time
import random

random.seed()

spoofport = 50020
with SocketIO('localhost',3000,LoggingNamespace) as socketIO:
	while True:
		x = random.random()*0.58
		y = random.random()*0.39
		eyes = [x,1 - y]
		eyes.append(spoofport) # send port as signature
		socketIO.emit('eye pos',eyes)
		time.sleep(0.5)

socketIO.wait(1)
