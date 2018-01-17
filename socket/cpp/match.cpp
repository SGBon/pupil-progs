/* performs matching between video stream of the pupil eye tracker world camera
 * and continous screenshots of the target screen.
 * gaze data and world frames are collected on worker threads
 * requires C++11, socketIO-client for cpp, opencv 3.x, Xlib, zmq, msgpack
 * additional dependencies for boost comes from socketIO
 */
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>
#include <sio_client.h>

#include "PupilGazeScraper.hpp"
#include "PupilFrameGrabber.hpp"
#include "scrshot.hpp"
#include "homography.hpp"
#include "util.hpp"
#include "averagewindow.hpp"

int main(int argc, char **argv){
	int frame_width = 640;
	int frame_height = 480;
	/* actual resolution of screen */
	int screen_width = 1920;
	int screen_height = 1080;
	/* resolution of screen in homography retrieval */
	const int screen_sub_width = 1280;
	const int screen_sub_height = 720;

	const int signature = 0;
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

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0);

	sio::client gaze_emitter;
	gaze_emitter.connect("http://127.0.0.1:3000");

	/* average window for smoothing gaze */
	const int WINDOW_SIZE = 30;
	AverageWindow x_average(WINDOW_SIZE);
	AverageWindow y_average(WINDOW_SIZE);

	/* continously get feed from pupil for gaze data and take screenshots
	 * get a homography between frame and screenshot and project the gaze coordinates
	 * to the screen space
	 */
	int key = 0;
	const cv::Size outputSize(frame_width+screen_sub_width,std::max(frame_height,screen_sub_height));
	cv::VideoWriter outputVideo;
	//outputVideo.open("debug.mp4",-1,25,outputSize,true);
	const bool opened = outputVideo.open("debug.mp4",cv::VideoWriter::fourcc('M','J','P','G'),5,outputSize,true);
	if(!opened){
		std::cerr << "Video did not open" << std::endl;
		exit(-1);
	}
	while(key != 'q'){
		GazePoint gaze_point = gaze_scraper.getGazePoint();
		/* smooth eye movement */
		x_average.push_back(gaze_point.x);
		gaze_point.x = x_average.getAverage();
		y_average.push_back(gaze_point.y);
		gaze_point.y = x_average.getAverage();

		cv::Mat frame = frame_grabber.getLastFrame();
		clahe->apply(frame,frame);
		cv::Mat screen;
		cv::resize(printscreen(0,0,screen_width,screen_height),screen,cv::Size(screen_sub_width,screen_sub_height));
		clahe->apply(screen,screen);

		/* homography from screen to frame and vice-versa */
		cv::Mat homography_s2f;
		cv::Mat homography_f2s;
		cv::Mat debug;
		const homography_state hs = retrieveHomography(frame,screen,homography_s2f,&debug);
		//const homography_state hs = retrieveHomographyNeighbourhood(frame,screen,homography_s2f,&debug);
		/* using the homography, invert it to go from frame to screenspace
		 * apply the inverse homography to the gaze point, normalize the output
		 * relative to the screen dimensions, and send down socketIO for use by
		 * gaze-enabled applications
		 */
		if(hs == HOMOG_SUCCESS){
			homography_f2s = homography_s2f.inv();

			cv::Mat point(1,1,CV_64FC2);
			cv::Mat normalized_point(1,1,CV_64FC2);
			point.at<double>(0,0) = gaze_point.x*frame_width;
			point.at<double>(0,1) = gaze_point.y*frame_height;

			try{
				cv::perspectiveTransform(point,normalized_point,homography_f2s);
				/* normalize the point and send it off down the pipeline */
				normalized_point.at<double>(0,0) /= screen_sub_width;
				normalized_point.at<double>(0,1) /= screen_sub_height;
				sio::message::list li;
				li.push(sio::double_message::create(normalized_point.at<double>(0,0)));
				li.push(sio::double_message::create(normalized_point.at<double>(0,1)));
				li.push(sio::int_message::create(signature));
				gaze_emitter.socket()->emit("eye pos",li.to_array_message());
			}catch (const std::exception &e){
				/* this is a non fatal error that pops up */
				std::cerr << e.what() << std::endl;
			}

			cv::imshow("Frame",frame);
			//cv::imshow("debug",debug);
			outputVideo << debug;
			key = cv::waitKey(1);
		}
	}

	gaze_scraper.stop();
	frame_grabber.stop();

	gaze_thread.join();
	frame_thread.join();

	gaze_emitter.sync_close();

	return 0;
}
