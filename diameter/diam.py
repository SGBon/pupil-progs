# shows pupil diameter on cv image

import zmq
from msgpack import loads

import numpy as np
import cv2
import random
import math

# scales the diameter from the tracker
def scaleDiameter(diameter):
	lower = 60.0
	higher = 140.0
	scaled = math.fabs((diameter - lower) / (higher - lower))
	return scaled

context = zmq.Context()
#open a req port to talk to pupil
addr = '127.0.0.1' # remote ip or localhost
req_port = "50020" # same as in the pupil remote gui
req = context.socket(zmq.REQ)
req.connect("tcp://%s:%s" %(addr,req_port))
# ask for the sub port
req.send('SUB_PORT')
sub_port = req.recv()
# open a sub port to listen to pupil
sub = context.socket(zmq.SUB)
sub.connect("tcp://%s:%s" %(addr,sub_port))
sub.setsockopt(zmq.SUBSCRIBE, 'pupil.0')

showing = True
cx = cy = 256
img = np.zeros((512,512,3),np.uint8)

# sizing parameters for ellipse size
pupil = 50
offset = 20
avg = 90

while showing:
	topic,msg = sub.recv_multipart()
	eye_positions = loads(msg)
	diam = eye_positions['diameter']

	if diam > 0:
		d = scaleDiameter(diam)
	else:
		d = scaleDiameter(avg)
	d = int(d*pupil) + offset

	for rad in range(d,125):
		B = random.randint(0,255)
		cv2.ellipse(img,(256,256),(rad,rad),0,0,360,(0,rad/2 + B%3,rad/2 + B%3),3)
	cv2.ellipse(img,(cx,cy),(d,d),0,0,360,(0),-1)
	cv2.imshow('image',img)
	k = cv2.waitKey(1)
	if k == 27:
		cv2.destroyAllWindows()
		showing = False
