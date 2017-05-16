#include "scrshot.hpp"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cstdlib>
#include <cstdio>

/* solution taken from http://stackoverflow.com/questions/3124229/taking-a-screenshot-with-c-gtk */
cv::Mat printscreen(int x, int y, int w, int h){
	gtk_init(0,NULL);

	GdkPixbuf *screenshot;
	GdkWindow *root_window;
	root_window = gdk_get_default_root_window();

	screenshot = gdk_pixbuf_get_from_drawable(NULL,root_window,NULL,x,y,0,0,w,h);
	cv::Mat screen(h,w,CV_8UC3);
	cv::Mat bwscreen(h,w,CV_8U);
	memcpy(screen.data,gdk_pixbuf_get_pixels(screenshot),w*h*3);
	cv:cvtColor(screen,bwscreen,cv::COLOR_RGB2GRAY);

	return bwscreen;
}
