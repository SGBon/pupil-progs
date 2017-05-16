#include "PupilGazeScraper.hpp"
#include <msgpack.hpp>

PupilGazeScraper::PupilGazeScraper(zmq::socket_t *subscriber):
	subscriber(subscriber),
	active(false){}

void PupilGazeScraper::run(){
	state_mutex.lock();
	active = true;
	state_mutex.unlock();

	zmq::message_t gaze_msg;
	char *sub_buffer;
	while(active){
		subscriber->recv(&gaze_msg);
		sub_buffer = (char *) malloc(sizeof(char)*gaze_msg.size());
		memcpy(sub_buffer,gaze_msg.data(),gaze_msg.size());
		free(sub_buffer);

		/* see if there's more data to get, if yes then get the data */
		size_t remaining_messages = 0;
		size_t option_length = sizeof(remaining_messages);
		subscriber->getsockopt(ZMQ_RCVMORE,&remaining_messages,&option_length);
		if(remaining_messages){
			gaze_msg.rebuild();
			subscriber->recv(&gaze_msg);
			sub_buffer = (char *) malloc(sizeof(char)*gaze_msg.size());
			memcpy(sub_buffer,gaze_msg.data(),gaze_msg.size());

			/* de serialize the gaze data */
			msgpack::unpacked deserial;
			msgpack::unpack(&deserial,sub_buffer,gaze_msg.size());
			msgpack::object gaze_obj = deserial.get();

			/* first item in map is norm_pos */
			msgpack::object_kv *p(gaze_obj.via.map.ptr);
			msgpack::object *norm_pos = p->val.via.array.ptr;
			storeGaze(norm_pos[0].as<float>(),norm_pos[1].as<float>());
			free(sub_buffer);
		}
	}
}

void PupilGazeScraper::stop(){
	state_mutex.lock();
	active = false;
	state_mutex.unlock();
}

void PupilGazeScraper::storeGaze(float x, float y){
	state_mutex.lock();
	gaze_point.x = x;
	gaze_point.y = y;
	state_mutex.unlock();
}

GazePoint PupilGazeScraper::getGazePoint(){
	std::lock_guard<std::mutex> lock(state_mutex);
	return gaze_point;
}
