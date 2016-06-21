from graphics import *
width = 1680
height = 1050
win = GraphWin('Tracking Gaze',width,height)
win.yUp()
win.autoflush = False

#draw ellipse at point
def drawEye(x,y):

	win.flush()
	cir = Circle(Point(x,y),5)
	cir.draw(win)
	return

import zmq
import json

#network setup
port = "5000"
context = zmq.Context();
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:"+port)

#recieve gaze positions
socket.setsockopt(zmq.SUBSCRIBE,'gaze_positions')
surface_name = "monitor"
while True:
	topic,msg = socket.recv_multipart()
	eye_positions = json.loads(msg)
	#for pos in eye_positions:
		#print pos
	try:
		eyex,eyey = eye_positions['realtime gaze on ' + surface_name]
		eyex = 1 - eyex
		drawEye(eyex*width,eyey*height)
		print "\n\n",topic,":\n",eyex,eyey
	except:
		pass
