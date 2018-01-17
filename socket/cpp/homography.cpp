#include "homography.hpp"
#include <limits>
#include <algorithm>
#include <vector>

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

	/* get only the good matches (where distance is less than 3*min_dist) or
	 * a low ceiling if min_dist is too small
	 */
	std::vector<cv::DMatch> good_matches;
	for(size_t i = 0; i < matches.size(); ++i)
		 if(matches[i].distance <= std::max(3*min_dist,0.02))
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

			std::vector<cv::DMatch> top_matches;
			for(size_t i = 0; i < std::min((size_t)20,good_matches.size());++i){
				top_matches.push_back(good_matches[i]);
			}
			cv::drawMatches(frame,keypoints_frame,screen,keypoints_screen,top_matches,*debug,
				cv::Scalar(0,0,255),cv::Scalar(255,0,0,100),std::vector<char>(),
			cv::DrawMatchesFlags::DEFAULT | cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		}

		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}

/* TODO: remove this when not applicable */
bool first = true;
const unsigned char MAX_K = 8; // max neighbours for nearest neighhour
const unsigned char DESC_PER_K = 2; // max description contribution from each neighbour

/* compute descriptors of keypoints based on the neighbourhood of keypoints
 * near them. First put the keypoints into a matrix which can be accepted
 * by cv::FlannBasedMatcher, then query the matcher for k-nearest neighbours
 * for each point and construct descriptors based on those neighbours.
 */
void computeNeighbourDescriptor(const cv::Mat &image,
	std::vector<cv::KeyPoint> &keypoints,
	cv::Mat &descriptors){
	cv::Mat keypoints_mat;
	for(size_t i = 0; i < keypoints.size();++i){
		cv::Mat row(1,2,CV_32F);
		row.at<float>(0) = keypoints[i].pt.x;
		row.at<float>(1) = keypoints[i].pt.y;
		keypoints_mat.push_back(row);
	}

	cv::FlannBasedMatcher keypointsMatcher;
	std::vector<std::vector<cv::DMatch>> matches;
	keypointsMatcher.knnMatch(keypoints_mat,keypoints_mat,matches,MAX_K);

	cv::Mat output;
	for(size_t i = 0; i < matches.size(); ++i){
		cv::Mat descriptor_row(1,(MAX_K-1)*DESC_PER_K,CV_32F);
		for(unsigned char j = 1; j < matches[i].size(); ++j){
			const cv::Mat point = keypoints_mat.row(matches[i][j].queryIdx);
			cv::Mat norm_point;
			cv::normalize(point,norm_point);
			for(unsigned char k = 0; k <DESC_PER_K; ++k){
				descriptor_row.at<float>(j*DESC_PER_K+k) = norm_point.at<float>(k);
			}
		}
		output.push_back(descriptor_row);
	}

	descriptors = output.clone();
}

homography_state retrieveHomographyNeighbourhood(const cv::Mat &frame, const cv::Mat &screen, cv::Mat &homography, cv::Mat *debug){
	/* create an ORB detector and keypoint vectors */
	cv::Ptr<cv::ORB> orb = cv::ORB::create(1000,1.2f,8,31,0,4);
	std::vector<cv::KeyPoint> keypoints_frame, keypoints_screen;

	/* get keypoints and descriptors with ORB */
	orb->detect(frame,keypoints_frame);
	orb->detect(screen,keypoints_screen);

	/* extract descriptor vectors from keypoints */
	cv::Mat descriptors_frame, descriptors_screen;
	computeNeighbourDescriptor(frame,keypoints_frame,descriptors_frame);
	computeNeighbourDescriptor(screen,keypoints_screen,descriptors_screen);

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
	cv::BFMatcher matcher(cv::NORM_L2);
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
		if(good_matches.size())
		for(size_t i = 0; i < good_matches.size(); ++i){
			frame_points.push_back(keypoints_frame[good_matches[i].queryIdx].pt);
			screen_points.push_back(keypoints_screen[good_matches[i].trainIdx].pt);
		}
		homography = cv::findHomography(screen_points,frame_points,CV_RANSAC);
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
				cv::Scalar(0,0,255),cv::Scalar(255,0,0,100),std::vector<char>(),
			cv::DrawMatchesFlags::DEFAULT | cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		}

		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}
