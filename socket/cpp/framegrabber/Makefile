CXX=g++
INCLUDE=`pkg-config --cflags gdk-2.0` `pkg-config --cflags atk` `pkg-config --cflags gtk+-2.0`
CFLAGS=-c -Wall -Wextra -g -std=c++11 -Wno-write-strings $(INCLUDE)
LIBS=-lzmq -lmsgpack `pkg-config --libs opencv` `pkg-config --libs gdk-2.0` `pkg-config --libs atk` `pkg-config --libs gtk+-2.0`
LDFLAGS=$(LIBS)
SOURCES=main.cpp util.cpp scrshot.cpp
OBJECTS=$(SOURCES:.cpp=.o)
DEPEND=.depend
EXECUTABLE=framegrabber.exe

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
