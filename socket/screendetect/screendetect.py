#uses the nmgaze library to map gaze
# requires PyGtk, numpy, socketIO-client, opencv 3.x, pyzmq, msgpack
import cv2
import numpy as np
import math
import time

# performs histogram equalization on the input image, returns equalized image
def equalize(frame):
	# do some equalization on frame
	 clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8,8))
	 cl1 = clahe.apply(frame)
	 return cl1

# receive multipart from subcriber
def recfromsub(sub):
	topic = sub.recv()
	msg = loads(sub.recv())
	extra = []
	while sub.get(zmq.RCVMORE):
		extra.append(sub.recv())
	if extra:
		msg['extra'] = extra[0]
	return topic,msg

def acceptLinePair(line1, line2, minTheta):
	theta1 = line1[1]
	theta2 = line2[1]

	if(theta1 < minTheta):
		theta1 += np.pi

	if(theta2 < minTheta):
		theta2 += np.pi

	return abs(theta1 - theta2) > minTheta

def computeIntersect(line1, line2):
	p1 = lineToPointPair(line1)
	p2 = lineToPointPair(line2)

	denom = (p1[0][0] - p1[1][0])*(p2[0][1] - p2[1][1]) - (p1[0][1] - p1[1][1])*(p2[0][0] - p2[1][0]);
	intersect = [((p1[0][0]*p1[1][1] - p1[0][1]*p1[1][0])*(p2[0][0] - p2[1][0]) -
		(p1[0][0] - p1[1][0])*(p2[0][0]*p2[1][1] - p2[0][1]*p2[1][0])) / denom,
		((p1[0][0]*p1[1][1] - p1[0][1]*p1[1][0])*(p2[0][1] - p2[1][1]) -
		(p1[0][1] - p1[1][1])*(p2[0][0]*p2[1][1] - p2[0][1]*p2[1][0])) / denom];

	return intersect


def lineToPointPair(line):
	cos_t = math.cos(line[1])
	sin_t = math.sin(line[1])
	x0 = line[0]*cos_t
	y0 = line[0]*sin_t
	alpha = 1000

	p1 = [x0+alpha*(-sin_t),y0 + alpha*(cos_t)]
	p2 = [x0-alpha*(-sin_t),y0 - alpha*(cos_t)]

	return [p1,p2]

# end of function definitions

import sys

if len(sys.argv) > 3:
	sig = int(sys.argv[1]) # set signature to first argument
	addr = str(sys.argv[2]) # set ip address to second argument
	req_port = str(sys.argv[3]) # set port to third argument
else:
	sig = 0 # default signature
	addr = '127.0.0.1' # default address
	req_port = "50020" # default port

import zmq

# initialize our ZMQ socket to pupil tracker
context = zmq.Context()
#open a req port to talk to pupil
req = context.socket(zmq.REQ)
req.connect("tcp://%s:%s" %(addr,req_port))
# ask for the sub port
req.send('SUB_PORT')
sub_port = req.recv()
# open a sub port to listen to pupil
sub = context.socket(zmq.SUB)
sub.setsockopt(zmq.SUBSCRIBE, 'frame.world') # we want frames from world camera
sub.setsockopt(zmq.SUBSCRIBE,'gaze') # gaze position data

# end ZMQ socket initialization

scrwid = 1920 # temporary
scrhei = 1080 # values
eqwid = 1280 # these should match
eqhei = 720 # from pupil output
focal_length = 526 # focal length of world camera, depends on camera used
fov_cam = 90
pixel_angle = fov_cam/math.sqrt((eqwid * eqwid) + (eqhei * eqhei))
physical_width = 512 # width of display in mm
px,py = [0,0]
smooth_x,smooth_y = 0.5, 0.5 # smoothing parameters

thresh1 = 50
thresh2 = 200
hough_votes = 150

from msgpack import loads
from socketIO_client import SocketIO, LoggingNamespace
# our communicator created here
with SocketIO('localhost',3000,LoggingNamespace) as socketIO:
	k=0
	while(k!=27):
		# important note, reconnecting and disconnecting
		sub.connect("tcp://%s:%s" %(addr,sub_port))

		start = time.time()
		topic,msg = recfromsub(sub)
		if topic == "gaze":
			x,y = msg['norm_pos']
			# smooth eye movement
			smooth_x += 0.5 * (x-smooth_x)
			smooth_y += 0.5 * (y-smooth_y)
			x = smooth_x
			y = 1 - smooth_y # y needs to be inverted
			px = int(x*eqwid)
			py = int(y*eqhei)
			# likely to get world from here
			topic,msg = recfromsub(sub)

		if topic == "frame.world":
			laststamp = msg['timestamp']
			npframe = np.fromstring(msg['extra'],np.uint8).reshape(eqhei,eqwid)
			#_, frame = cap.read()
			eq = equalize(npframe)
		else:
			continue

		dst = cv2.Canny(eq,thresh1,thresh2,None,3)
		cdst = cv2.cvtColor(dst,cv2.COLOR_GRAY2BGR)

		# find lines in video feed and get intersections of the lines
		lines = cv2.HoughLines(dst,1,np.pi/180,hough_votes,None,0,0)
		intersections = []
		if lines is not None:
			for i in range(0,len(lines)-1):
				for j in range(i+1,len(lines)):
					if acceptLinePair(lines[i][0],lines[j][0],np.pi/32.0):
						intersections.append(computeIntersect(lines[i][0],lines[j][0]))

		
		if len(intersections) > 3:
			intersections = np.float32(np.array(intersections))
			box = np.int0(cv2.boxPoints(cv2.minAreaRect(intersections)))
			intersections = np.int32(intersections)
			cv2.drawContours(cdst,[box],0,(0,0,255),2)
			for point in intersections:
				cv2.circle(cdst,(point[0],point[1]),1,(0,255,0),3)


		normp = None
		if normp != None:
			normp.append(sig)
			normp.append(10)
			socketIO.emit("eye pos",normp)

		#cv2.imshow("feed",eq)
		cv2.imshow("dst",dst)
		cv2.imshow("cdst",cdst)
		k = cv2.waitKey(1) & 0xFF
		if k == ord('a'):
			thresh1 += 1
		elif k == ord('z'):
			thresh1 -= 1

		if k == ord('s'):
			thresh2 += 1
		elif k == ord('x'):
			thresh2 -= 1

		if k == ord('d'):
			hough_votes += 1
		if k == ord('c'):
			hough_votes -= 1

		# disconnect so that publisher drops all messages
		# we only want latest messages and this is best way to do this
		sub.disconnect("tcp://%s:%s" %(addr,sub_port))
		elapsed = (time.time() - start)
		print thresh1,thresh2,hough_votes
		#print elapsed

cv2.destroyAllWindows()
