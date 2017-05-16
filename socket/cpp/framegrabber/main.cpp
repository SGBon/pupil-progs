#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

#include <zmq.hpp>
#include <msgpack.hpp>
#include <opencv2/opencv.hpp>

#include "util.hpp"
#include "scrshot.hpp"

int main(int argc, char **argv){
	int width;
	int height;
	int scrheight = 1080;
	int scrwidth = 1920;
	if(argc == 3){
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}else{
		width = 640;
		height = 480;
	}

	const size_t BUFFER_LEN = 100;
	char buffer[BUFFER_LEN] = {0};
	std::string transport("tcp://");
	std::string address("127.0.0.1");
	std::string req_port(":50020");
	zmq::context_t context(1);
	zmq::socket_t req(context,ZMQ_REQ);

	/* connect the requester to pupil */
	req.connect((transport+address+req_port).c_str());

	/* send over requester that we want a subscriber port */
	strncpy(buffer,"SUB_PORT",BUFFER_LEN);
	zmq::message_t req_msg(8);
	memcpy(req_msg.data(),buffer,8);
	req.send(req_msg);

	/* clear the message, and retrieve the subscriber port */
	req_msg.rebuild();
	req.recv(&req_msg);
	memcpy(buffer,req_msg.data(),BUFFER_LEN);

	/* extract port from all the other junk */
	int index = find_first_nonnumber(buffer,BUFFER_LEN);
	memset(buffer+index,0,BUFFER_LEN-index);
	std::string sub_port(":");
	sub_port += buffer;

	zmq::message_t frame_msg;
	unsigned char *sub_buffer;
	int key;
	/* receive topic */
	while(key != 'q'){
		/* create subscriber to subscribe for gaze data */
		zmq::socket_t sub(context,ZMQ_SUB);
		sub.setsockopt(ZMQ_SUBSCRIBE,"frame.world",11);
		sub.connect((transport+address+sub_port).c_str());
		sub.recv(&frame_msg);
		sub_buffer = new unsigned char[frame_msg.size()];
		memcpy(sub_buffer,frame_msg.data(),frame_msg.size());
		delete[] sub_buffer;

		/* see if there's more data to get, if yes then get the data */
		size_t remaining_messages = 0;
		size_t option_length = sizeof(remaining_messages);
		sub.getsockopt(ZMQ_RCVMORE,&remaining_messages,&option_length);
		if(remaining_messages){
			/* frame meta data */
			frame_msg.rebuild();
			sub.recv(&frame_msg);

			/* the frame itself */
			frame_msg.rebuild();
			sub.recv(&frame_msg);

			/* receive again cause frame comes after second message */
			cv::Mat frame(height,width,CV_8U);
			memcpy(frame.data,frame_msg.data(),frame_msg.size());
			cv::Mat screen = printscreen(0,0,scrwidth,scrheight);

			cv::namedWindow("Frame");
			cv::imshow("Frame",frame);
			cv::namedWindow("Screen");
			cv::imshow("Screen",screen);
			key = cv::waitKey(5);

			/* if there's more messages left in multipart message just waste them */
			while(remaining_messages){
				sub.getsockopt(ZMQ_RCVMORE,&remaining_messages,&option_length);
			}
		}
	}

	return 0;
}
