import sys
import zmq
import cv2

import msgpack
import msgpack_numpy as m

def unpackage(pack):
    return msgpack.unpackb(pack,object_hook=m.decode)

port = "5558"
if len(sys.argv) > 1:
    port = sys.argv[1]
    int(port)

context = zmq.Context()
socket = context.socket(zmq.SUB)

socket.connect("tcp://localhost:%s" % port)

topicfilter = "scrshot"
socket.setsockopt(zmq.SUBSCRIBE,topicfilter)

index = len(topicfilter) + 1
screen = None # keep this around in scope
while True:
    try:
        msg = socket.recv(flags=zmq.NOBLOCK)
        topic,pack = msg[:index],msg[index:]
        screen = unpackage(pack)
    except zmq.Again as e:
        print "noblock"
        pass
    if screen is not None:
        cv2.imshow("screen",screen)
        cv2.waitKey(1)
