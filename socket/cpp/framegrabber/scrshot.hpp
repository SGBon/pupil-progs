#ifndef PUPIL_PROGS_SCRSHOT_HPP
#define PUPIL_PROGS_SCRSHOT_HPP

#include <opencv2/opencv.hpp>

/* return screen or part of screen as an opencv matrix
 * parameters are a rectangle on the screen starting from the
 * top left defining area to take screenshot of
	*/
	cv::Mat printscreen(int x, int y, int w, int h);

#endif
