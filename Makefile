CC := gcc
CXX := g++
CFLAGS := -g -Wall -std=gnu99
CXXFLAGS := -g -Wall -fno-exceptions -std=c++14
UNAME := $(shell uname -s)
LDFLAGS := -g -lsrt \
	-lavformat -lavutil -lavcodec \
	-laws-cpp-sdk-core -laws-cpp-sdk-s3

ifeq ($(findstring MINGW,$(UNAME)),MINGW)
	LDFLAGS += -lws2_32
endif

C_SOURCES := $(shell find src -name '*.c')
CXX_SOURCES := $(shell find src -name '*.cc')

C_OBJS := $(C_SOURCES:src/%.c=build/%.o)
CXX_OBJS := $(CXX_SOURCES:src/%.cc=build/%.o)

TARGET := ezlive

all: $(TARGET)

docker: ezlive-docker-image.tar.gz

ezlive-docker-image.tar.gz: all
	sh ./scripts/build-docker.sh

$(TARGET): build $(C_OBJS) $(CXX_OBJS)
	$(CXX) $(C_OBJS) $(CXX_OBJS) -o $@ $(LDFLAGS)

build:
	mkdir -p build

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
