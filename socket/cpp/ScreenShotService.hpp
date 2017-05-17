/* takes a screenshot constantly,
 * runs on a seperate thread through the run() callback
 */

#ifndef SCREEN_SHOT_SERVICE_HPP
#define SCREEN_SHOT_SERVICE_HPP

#include <zmq.hpp>
#include <mutex>
#include <opencv2/opencv.hpp>

class ScreenShotService{
public:
	ScreenShotService(const int width, const int height, const int output_width,
	const int output_height);

	/* callback function for thread to initiate the screenshotting */
	void run();

	/* function to stop the screenshotting */
	void stop();

	/* gets last screenshot taken */
	cv::Mat getLastScreen();

private:
	void storeScreen(const cv::Mat &new_screen);

	std::mutex state_mutex;
	bool active;

	/* dimensions of screen */
	const int width;
	const int height;

	const int output_width;
	const int output_height;

	/* last screen stored */
	cv::Mat last_screen;

};

#endif
