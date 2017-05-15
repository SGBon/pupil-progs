CXX=g++
CFLAGS=-c -Wall -Wextra -g -std=c++11 -Wno-write-strings
LIBS=-lzmq -lmsgpack
LDFLAGS=$(LIBS)
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
DEPEND=.depend
EXECUTABLE=sigsender.exe

all: $(SOURCES) $(EXECUTABLE) $(DEPEND)

$(DEPEND): $(SOURCES)
	rm -f $@
	$(CXX) $(CFLAGS) -MM $^ > $@;

include .depend

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o : %.cpp
	$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean run
run: all
	./$(EXECUTABLE)
clean:
	rm $(OBJECTS) $(EXECUTABLE) $(DEPEND)
