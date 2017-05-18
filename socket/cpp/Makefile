CXX=clang++
INCLUDE=`pkg-config --cflags gdk-2.0` `pkg-config --cflags atk` `pkg-config --cflags gtk+-2.0` -Iextern/SocketIO/include/
CFLAGS=-c -Wall -Wextra -g -std=c++11 -Wno-write-strings $(INCLUDE)
LIBS=-lpthread -lzmq -lmsgpack -lX11 `pkg-config --libs opencv` `pkg-config --libs gdk-2.0` `pkg-config --libs atk` `pkg-config --libs gtk+-2.0` -Lextern/SocketIO/lib/Release/ -lsioclient -lboost_system -lboost_date_time -lboost_random -lcrypto -lssl
LDFLAGS=$(LIBS)
SOURCES=match.cpp PupilGazeScraper.cpp util.cpp PupilFrameGrabber.cpp scrshot.cpp
OBJECTS=$(SOURCES:.cpp=.o)
DEPEND=.depend
EXECUTABLE=match.exe

all: $(SOURCES) $(EXECUTABLE) $(DEPEND)

$(DEPEND): $(SOURCES)
	rm -f $@
	$(CXX) -MM $^ > $@ $(CFLAGS);

include .depend

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

%.o : %.cpp
	$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean run
run: all
	./$(EXECUTABLE)
clean:
	rm $(OBJECTS) $(EXECUTABLE) $(DEPEND)
