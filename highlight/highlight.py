##### highlights shapes on screen when user looks at them
##### made to test viewing the screen from different angles
width = 1680
height = 1080

import zmq
import json

import numpy as np
import cv2

img = np.zeros((height,width,3),np.uint8)

shapes = list()
sq1 = [100,100,200,200]
sq2 = [320,100,420,200]
shapes.append(sq1)
shapes.append(sq2)

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

#network setup
port = "5000"
context = zmq.Context();
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:"+port)

#recieve gaze positions
socket.setsockopt(zmq.SUBSCRIBE,'gaze_positions')
surface_name = "monitor"
Showing = True
cv2.namedWindow("Gazing", cv2.WND_PROP_FULLSCREEN)
cv2.setWindowProperty("Gazing", cv2.WND_PROP_FULLSCREEN, cv2.cv.CV_WINDOW_FULLSCREEN)
cv2.imshow('Gazing',img)
k = cv2.waitKey(1)
while Showing:
	topic,msg = socket.recv_multipart()
	eye_positions = json.loads(msg)
	try:
		eyex,eyey = eye_positions['realtime gaze on ' + surface_name]
		eyex = 1 - eyex
		for shape in shapes:
			within = checkBounds(shape,eyex,eyey)
			drawShape(shape,within)
		cv2.namedWindow("Gazing", cv2.WND_PROP_FULLSCREEN)
		cv2.setWindowProperty("Gazing", cv2.WND_PROP_FULLSCREEN, cv2.cv.CV_WINDOW_FULLSCREEN)
		cv2.imshow('Gazing',img)
		k = cv2.waitKey(1)
		if k == 27:
			Showing = False
	except:
		pass
