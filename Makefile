CXX := g++
CXXFLAGS := -g -Wall -fno-exceptions -std=c++20
UNAME := $(shell uname -s)
LDFLAGS := -g -lsrt \
	-lavformat -lavutil -lavcodec \
	-laws-cpp-sdk-core -laws-cpp-sdk-s3

ifeq ($(findstring MINGW,$(UNAME)),MINGW)
	LDFLAGS += -lws2_32
endif

CXX_SOURCES := $(shell find src -name '*.cc')

CXX_OBJS := $(CXX_SOURCES:src/%.cc=build/%.o)

TEST_SOURCES := $(wildcard tests/test_*.cc)
TEST_BINARIES := $(TEST_SOURCES:tests/%.cc=build/%)

TARGET := ezlive

all: $(TARGET)

test: $(TEST_BINARIES) $(CXX_OBJS)
	@for test in $(TEST_BINARIES); do echo "Running $$test..."; ./$$test || exit 1; done

docker: ezlive-docker-image.tar.gz

ezlive-docker-image.tar.gz: all
	sh ./scripts/build-docker.sh

$(TARGET): build $(C_OBJS) $(CXX_OBJS)
	$(CXX) $(C_OBJS) $(CXX_OBJS) -o $@ $(LDFLAGS)

build:
	mkdir -p build

build/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test binaries compilation pattern - standalone tests
build/test_%: tests/test_%.cc $(CXX_OBJS) build
	$(CXX) $(CXXFLAGS) $< -o $@ $(CXX_OBJS) $(LDFLAGS)

clean:
	rm -rf build $(TARGET) $(TEST_BINARIES)

.PHONY: all clean test build
