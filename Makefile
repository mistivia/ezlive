CC := gcc
CXX := g++
CFLAGS := -g -Wall
CXXFLAGS := -g -Wall -std=c++14
LDFLAGS := -g \
	-lavformat -lavutil -lavcodec \
	-laws-cpp-sdk-core -laws-cpp-sdk-s3

C_SOURCES := $(shell find . -maxdepth 1 -name '*.c')
CPP_SOURCES := $(shell find . -maxdepth 1 -name '*.cpp')

C_OBJS := $(C_SOURCES:.c=.o)
CPP_OBJS := $(CPP_SOURCES:.cpp=.o)

TARGET := ezlive

all: $(TARGET)

$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(C_OBJS) $(CPP_OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(C_OBJS) $(CPP_OBJS) $(TARGET)

.PHONY: all clean