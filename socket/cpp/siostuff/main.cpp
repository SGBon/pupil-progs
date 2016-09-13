#include <unistd.h>
#include <string>
#include <sio_client.h>

int main(int argc, char** argv){
	std::string sig;
	if(argc > 1)
		sig = std::string(argv[1]);
	else
		return -1;

	sio::client cl;
	cl.connect("http://127.0.0.1:3000");

	while(1){
		for(float i = 0; i < 1.0f; i+= 0.05){
			for(float j = 0; j < 1.0f; j+=0.1){
				sio::message::list li;
				li.push(std::to_string(j));
				li.push(std::to_string(1-i));
				li.push(sig);
				cl.socket()->emit("eye pos",li.to_array_message());
				usleep(50000);
			}
		}
	}

	return 0;
}
