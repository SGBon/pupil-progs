#include "homography.hpp"

/* the method here is borrowed from http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html
 * that tutorial is in 2.4 but this requires opencv 3.x
 */
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

	/* match vectors with BF matcher */
	cv::BFMatcher matcher(cv::NORM_L2);
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors_frame,descriptors_screen,matches);

	double max_dist = 0; double min_dist = 100;
	/* quickly get min and max */
	for(int i = 0; i < descriptors_frame.rows; ++i){
		const double dist = matches[i].distance;
		if(dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	}

	/* get only the good matches (where distance is less than 3*min_dist) or
	 * a low ceiling if min_dist is too small
	 */
	std::vector<cv::DMatch> good_matches;
	for(int i = 0; i < descriptors_frame.rows; ++i)
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

		return HOMOG_SUCCESS;
	}else{
		return HOMOG_FAIL;
	}
}
