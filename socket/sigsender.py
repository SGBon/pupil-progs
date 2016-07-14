import zmq
from msgpack import loads

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

from socketIO_client import SocketIO, LoggingNamespace

with SocketIO('localhost',3000,LoggingNamespace) as socketIO:
	while True:
		topic,msg = sub.recv_multipart()
		eye_positions = loads(msg)
		try:
			eyes = eye_positions['gaze_on_srf'][0]
			if(eyes[0] >=0 and eyes[0] <= 1 and eyes[1] >= 0 and eyes[1] <= 1):
				eyes.append(req_port) # send tracker port as signature
				print eyes
				socketIO.emit('eye pos',eyes)
		except:
			pass
socketIO.wait(1)
