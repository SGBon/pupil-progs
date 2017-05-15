#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <typeinfo>

#include <zmq.hpp>

#include <msgpack.hpp>

void print_buffer(const char *buffer, size_t len);

int find_first_nonnumber(const char *buffer,size_t len);

int main(int argc, char **argv){
	const size_t BUFFER_LEN = 100;
	char buffer[BUFFER_LEN] = {0};
	std::string transport("tcp://");
	std::string address("127.0.0.1");
	std::string req_port(":50020");
	zmq::context_t context(1);
	zmq::socket_t req(context,ZMQ_REQ);

	/* connect the requester to pupil */
	req.connect((transport+address+req_port).c_str());

	/* send over requester that we want a subscriber port */
	strncpy(buffer,"SUB_PORT",BUFFER_LEN);
	zmq::message_t req_msg(8);
	memcpy(req_msg.data(),buffer,8);
	req.send(req_msg);

	/* clear the message, and retrieve the subscriber port */
	req_msg.rebuild();
	req.recv(&req_msg);
	memcpy(buffer,req_msg.data(),BUFFER_LEN);

	/* extract port from all the other junk */
	int index = find_first_nonnumber(buffer,BUFFER_LEN);
	memset(buffer+index,0,BUFFER_LEN-index);
	std::string sub_port(":");
	sub_port += buffer;

	/* create subscriber to subscribe for gaze data */
	zmq::socket_t sub(context,ZMQ_SUB);
	sub.connect((transport+address+sub_port).c_str());
	sub.setsockopt(ZMQ_SUBSCRIBE,"gaze",4);

	/* receive topic */
	zmq::message_t gaze_msg;
	sub.recv(&gaze_msg);
	char *sub_buffer = (char *) malloc(sizeof(char)*gaze_msg.size());
	memcpy(sub_buffer,gaze_msg.data(),gaze_msg.size());
	free(sub_buffer);

	/* see if there's more data to get, if yes then get the data */
	size_t remaining_messages = 0;
	size_t option_length = sizeof(remaining_messages);
	sub.getsockopt(ZMQ_RCVMORE,&remaining_messages,&option_length);
	if(remaining_messages){
		gaze_msg.rebuild();
		sub.recv(&gaze_msg);
		sub_buffer = (char *) malloc(sizeof(char)*gaze_msg.size());
		memcpy(sub_buffer,gaze_msg.data(),gaze_msg.size());

		/* de serialize the gaze data */
		msgpack::unpacked deserial;
		msgpack::unpack(&deserial,sub_buffer,gaze_msg.size());
		msgpack::object gaze_obj = deserial.get();

		/* first item in map is norm_pos */
		msgpack::object_kv *p(gaze_obj.via.map.ptr);
		msgpack::object *norm_pos = p->val.via.array.ptr;
		float x = norm_pos[0].as<float>();
		float y = norm_pos[1].as<float>();

		printf("%f %f\n",x,y);

		free(sub_buffer);
	}

	return 0;
}

void print_buffer(const char *buffer, size_t len){
	for(size_t i = 0; i < len; ++i)
		printf("%c",buffer[i]);
	printf("\n");
}

int find_first_nonnumber(const char *buffer,size_t len){
	int index = -1;
	for(size_t i = 0; i < len; ++i)
		if(buffer[i] < '0' || buffer[i] > '9')
			return i;
	return index;
}
