#include "zhelpers.hpp"
#include <msgpack.hpp>

int main(){
	const std::string addr = "tcp://127.0.0.1:";
	zmq::context_t context(1);
	zmq::socket_t subscriber(context,ZMQ_SUB);
	/* I want the junk in here to deallocate at end of scope */
	{
		zmq::socket_t requester(context,ZMQ_REQ);
		const std::string req_port = "50020";
		requester.connect(addr+req_port);

		const std::string sendmsg = "SUB_PORT";
		s_send(requester,sendmsg);

		const std::string sub_port = s_recv(requester);
		std::cout << sub_port << std::endl;

		subscriber.connect(addr+sub_port);
		subscriber.setsockopt(ZMQ_SUBSCRIBE,"pupil.0");
	}
	/*
	msgpack::type::tuple<int,bool,std::string> src(1,true,"example");

	// serialize the object into the buffer.
	std::stringstream buffer;
	msgpack::pack(buffer,src);

	buffer.seekg(0);

	std::string str(buffer.str());

	msgpack::object_handle oh = msgpack::unpack(str.data(),str.size());

	msgpack::object deserialized = oh.get();

	std::cout << deserialized << std::endl;

	msgpack::type::tuple<int,bool,std::string> dst;
	deserialized.convert(dst);
	*/
	return 0;
}
