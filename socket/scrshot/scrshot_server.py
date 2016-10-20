# returns a screenshot, uses GTK
def getScreen(x,y,w,h):
	import gtk.gdk
	wind = gtk.gdk.get_default_root_window()
	sz = wind.get_size()
	pb = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB,False,8,w,h)
	pb = pb.get_from_drawable(wind,wind.get_colormap(),0,0,0,0,w,h)
	return pb.pixel_array

import msgpack
import msgpack_numpy as m

def package(msg):
	return msgpack.packb(msg,default=m.encode)

def unpackage(pack):
    return msgpack.unpackb(pack,object_hook=m.decode)

import zmq
import numpy as np
import random
import sys
import time

port = "5558"
if len(sys.argv) > 1:
	port = sys.argv[1]
	int (port)

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:%s" % port)

topic = "scrshot"
while True:
	screen = getScreen(0,0,1920,1080)
	pack = package(screen)
	socket.send(topic + ' ' + pack)
	print 'sending'
	#time.sleep(1)
