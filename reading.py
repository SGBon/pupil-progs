import zmq
import json

#network setup
port = "5000"
context = zmq.Context();
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:"+port)

#recieve gaze positions
socket.setsockopt(zmq.SUBSCRIBE,'pupil_positions')

while True:
	topic,msg = socket.recv_multipart()
	msg = json.loads(msg)
	print "\n\n",topic,":\n",msg
