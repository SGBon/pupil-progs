#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <thread>

#include <opencv2/opencv.hpp>

#include "PupilGazeScraper.hpp"
#include "PupilFrameGrabber.hpp"
#include "scrshot.hpp"
#include "util.hpp"

enum homography_state{
	HOMOG_SUCCESS,
	HOMOG_FAIL
};

/* finds a homography between two images and creates a perspective matrix
 * so that points from one image can be mapped to points in the other
 */
homography_state retrieveHomography(const cv::Mat &frame,const cv::Mat &screen, cv::Mat &homography);

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

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0);

	/* continously get feed from pupil for gaze data and take screenshots
	 * get a homography between frame and screenshot and project the gaze coordinates
	 * to the screen space
	 */
	int key = 0;
	while(key != 'q'){
		GazePoint gaze_point = gaze_scraper.getGazePoint();
		cv::Mat frame = frame_grabber.getLastFrame();
		clahe->apply(frame,frame);
		cv::Mat screen;
		cv::resize(printscreen(0,0,screen_width,screen_height),screen,cv::Size(800,600));
		clahe->apply(screen,screen);

		cv::Mat homography;
		homography_state hs = retrieveHomography(frame,screen,homography);
		std::cout << homography << std::endl;

		cv::imshow("Frame",frame);
		cv::imshow("Screen",screen);
		key = cv::waitKey(1);
	}

	gaze_scraper.stop();
	frame_grabber.stop();

	gaze_thread.join();
	frame_thread.join();

	return 0;
}

homography_state retrieveHomography(const cv::Mat &frame, const cv::Mat &screen, cv::Mat &homography){
	/* create an ORB detector and keypoint vectors */
	cv::Ptr<cv::FeatureDetector> detector = cv::ORB::create();
	std::vector<cv::KeyPoint> keypoints_frame, keypoints_screen;

	/* get keypoints and descriptors with ORB */
	detector->detect(frame,keypoints_frame);
	detector->detect(screen,keypoints_screen);

	/* extract descriptor vectors from keypoints */
	cv::Ptr<cv::DescriptorExtractor> extractor = cv::ORB::create();
	cv::Mat descriptors_frame, descriptors_screen;
	extractor->compute(frame,keypoints_frame,descriptors_frame);
	extractor->compute(screen,keypoints_screen,descriptors_screen);

	/* FLANN requires descriptors to be in CV_32F */
	if(descriptors_frame.type() != CV_32F)
		descriptors_frame.convertTo(descriptors_frame,CV_32F);

	if(descriptors_screen.type() != CV_32F)
		descriptors_screen.convertTo(descriptors_screen,CV_32F);

	/* match vectors with FLANN matcher */
	cv::FlannBasedMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors_frame,descriptors_screen,matches);

	double max_dist = 0; double min_dist = 100;
	/* quickly get min and max */
	for(int i = 0; i < descriptors_frame.rows; ++i){
		const double dist = matches[i].distance;
		if(dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	}

	/* get only the good matches (where distance is less than 2*min_dist) or
	 * a low ceiling if min_dist is too small
	 */
	std::vector<cv::DMatch> good_matches;
	for(int i = 0; i < descriptors_frame.rows; ++i)
		 if(matches[i].distance <= std::max(2*min_dist,0.02))
			 good_matches.push_back(matches[i]);

	/* compute the homography using matching points */
	if(good_matches.size() >= 10){
		std::vector<cv::Point2f> frame_points;
		std::vector<cv::Point2f> screen_points;
		if(good_matches.size())
		for(size_t i = 0; i < good_matches.size(); ++i){
			frame_points.push_back(keypoints_frame[good_matches[i].queryIdx].pt);
			screen_points.push_back(keypoints_screen[good_matches[i].trainIdx].pt);
		}
		homography = cv::findHomography(screen_points,frame_points,CV_RANSAC);
		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}
