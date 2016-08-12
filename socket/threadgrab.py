import zmq
from msgpack import loads

import cv2
import numpy as np

import sys

if len(sys.argv) > 1:
	sig = int(sys.argv[1]) # set signature to first argument
else:
	sig = 0 # default signature

import threading
class dataReceiver (threading.Thread):
	def __init__(self,lock):
		threading.Thread.__init__(self)
		self.lock = lock
		self.frame = None
		self.running = False;

	def getFrame(self):
		lock.acquire()
		out = self.frame
		lock.release()
		return out

	def stop(self):
		self.running = False

	def run(self):
		self.running = True
		context = zmq.Context()
		#open a req port to talk to pupil
		addr = '127.0.0.1' # remote ip or localhost
		req_port = "50020" if sig == 0 else "50021" # same as in the pupil remote gui
		req = context.socket(zmq.REQ)
		req.connect("tcp://%s:%s" %(addr,req_port))
		# ask for the sub port
		req.send('SUB_PORT')
		sub_port = req.recv()
		# open a sub port to listen to pupil
		sub = context.socket(zmq.SUB)
		sub.connect("tcp://%s:%s" %(addr,sub_port))
		sub.setsockopt(zmq.SUBSCRIBE, 'frame.world')

		while(self.running):
			lock.acquire()
			topic,msg,frame = sub.recv_multipart()
			unpacked = loads(msg)
			npframe = np.fromstring(frame,np.uint8)
			self.frame = cv2.imdecode(npframe,cv2.IMREAD_COLOR)
			lock.release()

lock = threading.Lock()
flist = []
drthread = dataReceiver(lock)
drthread.start()
while True:
	img = drthread.getFrame()
	if img is not None:
		cv2.imshow('img',img)
		k = cv2.waitKey(1) & 0xFF
		if k == 27:
			drthread.stop()
			break;

cv2.destroyAllWindows()
