import time
import zmq
from msgpack import loads

import sys
if len(sys.argv) > 1:
	sig = int(sys.argv[1]) # set signature to first argument
else:
	sig = 0 # default signature
# get top messages and just drop them

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

import cv2
import numpy as np

imfile = "frame.jpg"
lasttime = time.clock()
currtime = 0
while True:
	topic,msg,frame = sub.recv_multipart()
	unpacked = loads(msg)
	npframe = np.fromstring(frame,np.uint8).reshape(480,640)
	cv2.imshow('img',npframe)
	cv2.imwrite(imfile,npframe)
	k = cv2.waitKey(1) & 0xFF
	if k == 27:
		break;

cv2.destroyAllWindows()
