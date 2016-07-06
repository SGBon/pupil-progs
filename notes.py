import zmq
ctx = zmq.Context()
# The requester talks to Pupil remote and receives the session unique IPC SUB PORT
requester = ctx.socket(zmq.REQ)
ip = 'localhost' #If you talk to a different machine use its IP.
port = 50020 #The port defaults to 50020 but can be set in the GUI of Pupil Capture.
requester.connect('tcp://%s:%s'%(ip,port))
requester.send('SUB_PORT')
sub_port = requester.recv()

subscriber = ctx.socket(zmq.SUB)
subscriber.connect('tcp://%s:%s'%(ip,sub_port))
subscriber.set(zmq.SUBSCRIBE, 'notify.') #receive all notification messages
subscriber.set(zmq.SUBSCRIBE, 'logging.error') #receive logging error messages
#subscriber.set(zmq.SUBSCRIBE, '') #receive everything (don't do this)
# you can setup multiple subscriber sockets
# Sockets can be polled or read in different threads.

# we need a serialiser
import msgpack as serializer

while True:
    topic,payload = subscriber.recv_multipart()
    message = serializer.loads(payload)
    print topic,':',message
