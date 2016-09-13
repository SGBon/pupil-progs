PROJECT=zmqreceiver
CXX=g++
LIBS=-lzmq -lmsgpackc
INCLUDE=
CFLAGS=-c -Wall -Wextra -g -O0 $(INCLUDE)
LDFLAGS=$(LIBS)
SOURCES=$(wildcard *.cpp)

OBJECTS=$(SOURCES:.cpp=.o)
BINARY=$(PROJECT)

all: $(SOURCES) $(BINARY)

$(BINARY): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(BINARY)

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(BINARY)
