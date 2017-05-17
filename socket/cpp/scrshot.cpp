#include "scrshot.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>

/* this method from http://stackoverflow.com/questions/24988164/c-fast-screenshots-in-linux-for-use-with-opencv */
cv::Mat printscreen(const int x, const int y, const int w, const int h){
	Display *display = XOpenDisplay(NULL);
	Window root = DefaultRootWindow(display);

	XImage *img = XGetImage(display,root,0,0,w,h,AllPlanes,ZPixmap);
	const int bits_per_pixel = img->bits_per_pixel;

	cv::Mat screen (h,w,bits_per_pixel > 24 ? CV_8UC4 : CV_8UC3,img->data);
	cv::Mat bwscreen(h,w,CV_8U);
	cv::cvtColor(screen,bwscreen,cv::COLOR_RGBA2GRAY);

	XDestroyImage(img);
	XCloseDisplay(display);

	return bwscreen;
}
