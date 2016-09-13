PROJECT=cpptester
CXX=g++
LIBS=-Llib/Release -lsioclient -Wl,-rpath=lib/Release/
INCLUDE=-Iinclude/
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
