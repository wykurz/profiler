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

all: lib tests

UNIT_TESTS_SRC=$(wildcard tests/unit/**/*.cpp) tests/unit/Main.cpp
UNIT_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(UNIT_TESTS_SRC))

tests: $(TEST_DIR)/unit_tests

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
