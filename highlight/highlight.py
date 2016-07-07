##### highlights shapes on screen when user looks at them
##### made to test viewing the screen from different angles
import zmq
from msgpack import loads

import time
from platform import system
from pymouse import PyMouse
m = PyMouse()
m.move(0,0) # hack to init PyMouse -- still needed

import numpy as np
import cv2

shapes = list()
sq1 = [100,100,200,200]
sq2 = [320,100,420,200]
sq3 = [700,800,800,900]
shapes.append(sq1)
shapes.append(sq2)
shapes.append(sq3)

def get_screen_size():
	screen_size = None
	if system() == "Darwin":
		screen_size = sp.check_output(["./mac_os_helpers/get_screen_size"]).split(",")
		screen_size = float(screen_size[0]),float(screen_size[1])
	else:
		screen_size = m.screen_size()
	return screen_size

def drawShape(shape,within):
	global img
	T = 1 #thickness of edge, -1 when fill
	if within == True:
		T = -1
	cv2.rectangle(img,(shape[0],shape[1]),(shape[2],shape[3]),(0,255,0),T)
	return

def checkBounds(shape ,eyex,eyey):
	if eyex >= shape[0] and eyex <= shape[2] and eyey >= shape[1] and eyey <= shape[3]:
		return True
	else:
		return False

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
sub.setsockopt(zmq.SUBSCRIBE, 'surface')

surface_name = "monitor"
Showing = True

# screen size
x_dim, y_dim = get_screen_size()
img = np.zeros((y_dim,x_dim,3),np.uint8)
print "x_dim: %s, y_dim: %s" %(x_dim,y_dim)

while Showing:
	topic,msg = sub.recv_multipart()
	eye_positions = loads(msg)
	try:
		img = np.zeros((y_dim,x_dim,3),np.uint8)
		eyex,eyey = eye_positions['gaze_on_srf'][0]
		eyey = 1 - eyey
		scrx = int(eyex*x_dim)
		scry = int(eyey*y_dim)
		if eyex >= 0 and eyey >= 0 and eyex <= x_dim and eyey <= y_dim:
			for shape in shapes:
				within = checkBounds(shape,scrx,scry)
				drawShape(shape,within)
			cv2.namedWindow("Gazing", cv2.WND_PROP_FULLSCREEN)
			cv2.setWindowProperty("Gazing", cv2.WND_PROP_FULLSCREEN, cv2.cv.CV_WINDOW_FULLSCREEN)
			cv2.ellipse(img,(scrx,scry),(10,10),0,0,360,(0,0,255),3)
			cv2.imshow('Gazing',img)
			k = cv2.waitKey(1)

			if k == 27:
				Showing = False
	except:
		pass
