#include <cstdio>
#include <cstdlib>
#include <thread>


#include "PupilGazeScraper.hpp"
#include "PupilFrameGrabber.hpp"
#include "scrshot.hpp"
#include "util.hpp"

int main(int argc, char **argv){
	int frame_width = 640;
	int frame_height = 480;
	int screen_width = 1920;
	int screen_height = 1080;
	if(argc > 4){
		frame_width = atoi(argv[1]);
		frame_height = atoi(argv[2]);
		screen_width = atoi(argv[3]);
		screen_height = atoi(argv[4]);
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

	/* create subscriber for gaze data */
	zmq::socket_t gaze_sub(context,ZMQ_SUB);
	gaze_sub.connect((transport+address+sub_port).c_str());
	gaze_sub.setsockopt(ZMQ_SUBSCRIBE,"gaze",4);

	/* create subcriber for frame data */
	zmq::socket_t frame_sub(context,ZMQ_SUB);
	frame_sub.connect((transport+address+sub_port).c_str());
	frame_sub.setsockopt(ZMQ_SUBSCRIBE,"frame.world",11);

	/* create the worker threads */
	PupilGazeScraper gaze_scraper(&gaze_sub);
	std::thread gaze_thread(&PupilGazeScraper::run,&gaze_scraper);

	PupilFrameGrabber frame_grabber(&frame_sub,frame_width,frame_height);
	std::thread frame_thread(&PupilFrameGrabber::run,&frame_grabber);

	/* continously get feed from pupil for gaze data and take screenshots
	 * get a homography between frame and screenshot and project the gaze coordinates
	 * to the screen space
	 */
	int key = 0;
	while(key != 'q'){
		GazePoint gaze_point = gaze_scraper.getGazePoint();
		cv::Mat frame = frame_grabber.getLastFrame();
		cv::Mat screen;
		cv::resize(printscreen(0,0,screen_width,screen_height),screen,cv::Size(frame_width,frame_height));

		cv::namedWindow("Frame");
		cv::imshow("Frame",frame);
		cv::namedWindow("Screen");
		cv::imshow("Screen",screen);
		key = cv::waitKey(5);
	}

	gaze_scraper.stop();
	frame_grabber.stop();

	gaze_thread.join();
	frame_thread.join();

}
