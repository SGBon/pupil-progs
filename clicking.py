#facilitates clicking by finding out when the eye closes
start = 0
elapsed = 0
def checkElapsed(current):
	global start
	global elapsed
	delay = 1.0
	if(start != 0 and elapsed > delay):
		start = 0
		elapsed = 0
		return True
	elif(start == 0):
		start = current
	else:
		elapsed = current - start
	return False

def resetTimer():
	global start
	global elapsed
	start = 0
	elapsed = 0

# borrowed from pupil-helper
def set_mouse(x=0,y=0,click=0):
	try:
		from pymouse import PyMouse
		m = PyMouse()
		m.move(0,0) # hack to init PyMouse -- still needed
		m.move(x,y)
	except ImportError as e:
		print "Error: %s" %(e)

def click(x, y):
	from pymouse import PyMouse
	m = PyMouse()
	m.click(x,y)
	print "clicking"

import zmq
import json

#network setup
port = "5000"
context = zmq.Context();
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:"+port)

#recieve gaze positions
socket.setsockopt(zmq.SUBSCRIBE,'gaze_positions')
socket.setsockopt(zmq.SUBSCRIBE,'pupil_positions')

#client variables
surface_name = "monitor"
x_dim = 1680
y_dim = 1050

#smoothing variable
smooth_x, smooth_y = 0.5, 0.5
x = 0
y = 0

while True:
	topic,msg = socket.recv_multipart()
	msg = json.loads(msg)
	if(topic == "pupil_positions"):
		if(msg["diameter"] == 0):
			current = msg["timestamp"]
			if(checkElapsed(current)):
				click(x,y)
		else:
			resetTimer()
	if(topic == "gaze_positions")
		# mouse location code borrowed from pupil-helpers
		gaze_on_screen = msg['realtime gaze on ' + surface_name]
		raw_x,raw_y = gaze_on_screen
		raw_x = 1 - raw_x
		# smoothing out the gaze so the mouse has smoother movement
		smooth_x += 0.5 * (raw_x-smooth_x)
		smooth_y += 0.5 * (raw_y-smooth_y)
		x = smooth_x
		y = smooth_y

		y = 1-y # inverting y so it shows up correctly on screen
		x *= int(x_dim)
		y *= int(y_dim)
		# PyMouse or MacOS bugfix - can not go to extreme corners because of hot corners?
		x = min(x_dim-10, max(10,x))
		y = min(y_dim-10, max(10,y))
		set_mouse(x,y)

	#print "\n\n",topic,":\n",msg
