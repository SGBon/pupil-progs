import zmq
import json

#network setup
port = "5000"
context = zmq.Context();
socket = context.socket(zmq.SUB)
socket.connect("tcp://127.0.0.1:"+port)

#recieve gaze positions
socket.setsockopt(zmq.SUBSCRIBE,'pupil_positions')

import matplotlib.pyplot as plt

diameters = []
rolling = 0
go = True
MAXSAMPLES = 1000 # maximum samples to collect

while go:
	topic,msg = socket.recv_multipart()
	msg = json.loads(msg)
	diameter = msg['diameter']
	if(diameter > 0):
		diameters = diameters + [diameter]
		print diameter
	# collect samples
	if(len(diameters) > MAXSAMPLES):
		go = False


plt.plot(diameters)
plt.ylabel('Diameters')
plt.show()
