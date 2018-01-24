#include "homography.hpp"
#include <limits>
#include <algorithm>
#include <vector>
#include <cmath>

#include "scd.hpp"

bool dmatchCompare(cv::DMatch first, cv::DMatch second){
	return first.distance < second.distance;
}

/* the method here is borrowed from http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html
 * that tutorial is in 2.4 but this requires opencv 3.x
 */
homography_state retrieveHomography(const cv::Mat &frame, const cv::Mat &screen, cv::Mat &homography, cv::Mat *debug){
	/* create an ORB detector and keypoint vectors */
	cv::Ptr<cv::ORB> orb = cv::ORB::create(1000,1.2f,8,31,0,4);
	std::vector<cv::KeyPoint> keypoints_frame, keypoints_screen;

	/* get keypoints and descriptors with ORB */
	orb->detect(frame,keypoints_frame);
	orb->detect(screen,keypoints_screen);

	/* extract descriptor vectors from keypoints */
	cv::Mat descriptors_frame, descriptors_screen;
	orb->compute(frame,keypoints_frame,descriptors_frame);
	orb->compute(screen,keypoints_screen,descriptors_screen);

	/* FLANN requires descriptors to be in CV_32F */
	/*
	if(descriptors_frame.type() != CV_32F)
		descriptors_frame.convertTo(descriptors_frame,CV_32F);

	if(descriptors_screen.type() != CV_32F)
		descriptors_screen.convertTo(descriptors_screen,CV_32F);
		*/
	/* match vectors with BF matcher */
	cv::BFMatcher matcher(cv::NORM_HAMMING2);
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors_frame,descriptors_screen,matches);
	std::sort(matches.begin(),matches.end(),dmatchCompare);

	/* quickly get min and max */
	double max_dist = 0;
	double min_dist = DBL_MAX;
	for(size_t i = 0; i < matches.size(); ++i){
		const double dist = matches[i].distance;
		if(dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	}

	/* get the top matches necessary for a homography computation */
	std::vector<cv::DMatch> good_matches;
	for(size_t i = 0; i < std::min((size_t)60,matches.size()); ++i)
			 good_matches.push_back(matches[i]);

	/* compute the homography using matching points */
	if(good_matches.size() >= 10){
		std::vector<cv::Point2f> frame_points;
		std::vector<cv::Point2f> screen_points;
		for(size_t i = 0; i < good_matches.size(); ++i){
			frame_points.push_back(keypoints_frame[good_matches[i].queryIdx].pt);
			screen_points.push_back(keypoints_screen[good_matches[i].trainIdx].pt);
		}
		homography = cv::findHomography(screen_points,frame_points,CV_RANSAC);
		if(homography.empty()){
			std::cout << "no homography" << std::endl;
			return HOMOG_FAIL;
		}

		/* rectangle on screen in the frame */
		const int height = screen.rows;
		const int width = screen.cols;
		std::vector<cv::Point2f> rect;
		rect.emplace_back(0,0);
		rect.emplace_back(0,height-1);
		rect.emplace_back(width-1,height-1);
		rect.emplace_back(width-1,0);

		cv::Mat transformed;
		cv::perspectiveTransform(rect,transformed,homography);
		transformed.convertTo(transformed,CV_32S);
		cv::polylines(frame,transformed,true,255,3,cv::LINE_AA);

		if(debug != NULL){
			//cv::drawKeypoints(frame,keypoints_frame,debug_frame);
			cv::drawMatches(frame,keypoints_frame,screen,keypoints_screen,good_matches,*debug,
				cv::Scalar::all(-1),cv::Scalar(255,0,0,100),std::vector<char>(),
			cv::DrawMatchesFlags::DEFAULT | cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		}
		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}

/* TODO: remove this when not applicable */
bool first = true;

homography_state retrieveHomographyNeighbourhood(const cv::Mat &frame, const cv::Mat &screen, cv::Mat &homography, cv::Mat *debug){
	/* create an ORB detector and keypoint vectors */
	cv::Ptr<cv::ORB> orb = cv::ORB::create(1000,1.2f,8,31,0,4);
	std::vector<cv::KeyPoint> keypoints_frame, keypoints_screen;

	/* get keypoints and descriptors with ORB */
	orb->detect(frame,keypoints_frame);
	orb->detect(screen,keypoints_screen);

	/* extract descriptor vectors from keypoints */
	cv::Mat descriptors_frame, descriptors_screen;
	computeSCD(frame,keypoints_frame,descriptors_frame);
	computeSCD(screen,keypoints_screen,descriptors_screen);

	/* TODO: remove this when not applicable */
	if(first){
		std::cout << "FRAME" << std::endl;
		std::cout << descriptors_frame << std::endl;
		std::cout << "SCREEN" << std::endl;
		std::cout << descriptors_screen << std::endl;
		first = false;
	}

	/* FLANN requires descriptors to be in CV_32F */
	/*
	if(descriptors_frame.type() != CV_32F)
		descriptors_frame.convertTo(descriptors_frame,CV_32F);

	if(descriptors_screen.type() != CV_32F)
		descriptors_screen.convertTo(descriptors_screen,CV_32F);
		*/
	/* match vectors with BF matcher */
	cv::FlannBasedMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors_frame,descriptors_screen,matches);
	std::sort(matches.begin(),matches.end(),dmatchCompare);

	/* get the top matches necessary for a homography computation */
	std::vector<cv::DMatch> good_matches;
	for(size_t i = 0; i < std::min((size_t)60,matches.size()); ++i)
			 good_matches.push_back(matches[i]);

	/* compute the homography using matching points */
	if(good_matches.size() >= 10){
		std::vector<cv::Point2f> frame_points;
		std::vector<cv::Point2f> screen_points;
		for(size_t i = 0; i < good_matches.size(); ++i){
			frame_points.push_back(keypoints_frame[good_matches[i].queryIdx].pt);
			screen_points.push_back(keypoints_screen[good_matches[i].trainIdx].pt);
		}
		homography = cv::findHomography(screen_points,frame_points,CV_RANSAC);
		if(homography.empty()){
			std::cout << "no homography" << std::endl;
			return HOMOG_FAIL;
		}

		/* rectangle on screen in the frame */
		const int height = screen.rows;
		const int width = screen.cols;
		std::vector<cv::Point2f> rect;
		rect.emplace_back(0,0);
		rect.emplace_back(0,height-1);
		rect.emplace_back(width-1,height-1);
		rect.emplace_back(width-1,0);

		cv::Mat transformed;
		cv::perspectiveTransform(rect,transformed,homography);
		transformed.convertTo(transformed,CV_32S);
		cv::polylines(frame,transformed,true,255,3,cv::LINE_AA);

		if(debug != NULL){
			//cv::drawKeypoints(frame,keypoints_frame,debug_frame);
			cv::drawMatches(frame,keypoints_frame,screen,keypoints_screen,good_matches,*debug,
				cv::Scalar::all(-1),cv::Scalar(255,0,0),std::vector<char>(),
			cv::DrawMatchesFlags::DEFAULT | cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		}

		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}
