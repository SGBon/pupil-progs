jsimport sys
import time
import zmq
import cv2

import msgpack
import msgpack_numpy as m

def unpackage(pack):
    return msgpack.unpackb(pack,object_hook=m.decode)

def fetchFromServer(socket,index):
    msg = socket.recv()
    topic,pack = msg[:index],msg[index:]
    return unpackage(pack)

port = "5555"
if len(sys.argv) > 1:
    port = sys.argv[1]
    int(port)

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.setsockopt(zmq.RCVHWM,50)

topicfilter = "scrshot"
socket.setsockopt(zmq.SUBSCRIBE,topicfilter)

index = len(topicfilter) + 1
screen = None # keep this around in scope
if __name__ == '__main__':
    while True:
        socket.connect("tcp://localhost:%s" % port)
        start = time.time()
        screen = fetchFromServer(socket,index)
        cv2.imshow("screen",screen)
        k = cv2.waitKey(1) & 0xFF
        if k == 27:
            break;
        socket.disconnect("tcp://localhost:%s" % port)
        print time.time() - start

cv2.destroyAllWindows()
