/* gets gaze data from the pupil pipeline
 * performs this on seperate thread
 */
#ifndef PUPIL_GAZE_SCRAPER_HPP
#define PUPIL_GAZE_SCRAPER_HPP

#include <zmq.hpp>
#include <mutex>

struct GazePoint{
	float x;
	float y;
};

class PupilGazeScraper{
public:
	PupilGazeScraper(zmq::socket_t *subscriber);

	/* callback function for a thread to initiate the gaze scraper */
	void run();

	/* function to stop the gaze scraper */
	void stop();

	GazePoint getGazePoint();

private:
	void storeGaze(float x, float y);

	zmq::socket_t *subscriber;
	std::mutex state_mutex;
	bool active;
	/* locations of gaze, normalized to camera space */
	GazePoint gaze_point;
};

#endif
