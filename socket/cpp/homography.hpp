#include <opencv2/opencv.hpp>

enum homography_state{
	HOMOG_SUCCESS,
	HOMOG_FAIL
};

/* finds a homography between two images and creates a perspective matrix
 * so that points from one image can be mapped to points in the other
 */
homography_state retrieveHomography(const cv::Mat &frame,const cv::Mat &screen, cv::Mat &homography);
