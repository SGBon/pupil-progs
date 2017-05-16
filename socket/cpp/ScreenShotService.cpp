#include "ScreenShotService.hpp"

ScreenShotService::ScreenShotService(const int width, const int height):
	active(false),
	width(width),
	height(height),
	last_screen(height,width,CV_8U){};

void ScreenShotService::run(){
	state_mutex.lock();
	active = true;
	state_mutex.unlock();

	/* TODO: finish this up ie. port the screenshot gdk stuff in here */
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
