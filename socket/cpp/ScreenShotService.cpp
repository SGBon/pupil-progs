#include "ScreenShotService.hpp"
#include "scrshot.hpp"

ScreenShotService::ScreenShotService(const int width, const int height, const int output_width,
	const int output_height):
	active(false),
	width(width),
	height(height),
	output_width(output_width),
	output_height(output_height),
	last_screen(height,width,CV_8U){};

void ScreenShotService::run(){
	state_mutex.lock();
	active = true;
	state_mutex.unlock();

	/* take screenshots until stopped */
	while(active){
		cv::Mat scaled_screen;
		cv::resize(printscreen(0,0,width,height),scaled_screen,cv::Size(output_width,output_height));
		storeScreen(scaled_screen);
	}
}

void ScreenShotService::stop(){
	state_mutex.lock();
	active = false;
	state_mutex.unlock();
}

cv::Mat ScreenShotService::getLastScreen(){
	std::lock_guard<std::mutex> lock(state_mutex);
	return last_screen.clone();
}

void ScreenShotService::storeScreen(const cv::Mat &new_screen){
	state_mutex.lock();
	new_screen.copyTo(last_screen);
	state_mutex.unlock();
}
