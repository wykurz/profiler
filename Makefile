LIBDIRS?=/usr/lib
INCDIRS?=/usr/include
LIBNAME=cxxprof
CXX=clang++-3.8
INC=-Iprofiler
CFLAGS=-std=c++14 -g -O3 -Wall
LFLAGS=-lpthread -latomic
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
TEST_DIR=$(BUILD_DIR)/tests
LIBPROFILER=$(BUILD_DIR)/lib/lib$(LIBNAME).so
LIBPATH=$(abspath $(dir $(LIBPROFILER)))

all: lib tests perf

PERF_TESTS_SRC=$(wildcard tests/perf/**/*.cpp)
PERF_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PERF_TESTS_SRC))

perf: $(TEST_DIR)/perf
	./build/tests/perf

$(TEST_DIR)/perf: $(PERF_TESTS_OBJ) $(LIBPROFILER)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -l$(LIBNAME) -L$(LIBPATH) -Wl,-R$(LIBPATH) -o $@ $^ -Wl,-Bstatic -L$(LIBDIRS) -lbenchmark -Wl,-Bdynamic

$(OBJ_DIR)/tests/perf/%.o: tests/perf/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC) -I$(INCDIRS) -c -o $@ $<

UNIT_TESTS_SRC=$(wildcard tests/unit/**/*.cpp) tests/unit/Main.cpp
UNIT_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(UNIT_TESTS_SRC))

tests: $(TEST_DIR)/unit_tests
	./build/tests/unit_tests --show_progress --log_level=test_suite

$(TEST_DIR)/unit_tests: $(UNIT_TESTS_OBJ)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -lboost_unit_test_framework -l$(LIBNAME) -L$(LIBPATH) -Wl,-R$(LIBPATH) -o $@ $^

$(OBJ_DIR)/tests/unit/%.o: tests/unit/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -DBOOST_TEST_DYN_LINK $(CFLAGS) $(INC) -c -o $@ $<

PROFILER_SRC=$(wildcard profiler/**/*.cpp)
PROFILER_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PROFILER_SRC))

lib: $(LIBPROFILER)

$(LIBPROFILER): $(PROFILER_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(CFLAGS) $(LFLAGS) -o $@ $^

$(OBJ_DIR)/profiler/%.o: profiler/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -fPIC $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@rm build -rf

.PHONY : all lib tests clean
