CXX=clang++-3.8
INC=-Iprofiler
CFLAGS=-fPIC -std=c++14 -g -O3 -Wall
LFLAGS=-shared
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
LIBPROFILER=$(BUILD_DIR)/lib/libprofiler.so

PROFILER_SRC=$(wildcard profiler/**/*.cpp)
PROFILER_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PROFILER_SRC))

all: $(LIBPROFILER)

$(LIBPROFILER): $(PROFILER_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(LFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@rm build -rf

.PHONY : all clean
