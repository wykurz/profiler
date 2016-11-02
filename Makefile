LIBDIRS?=/usr/lib
INCDIRS?=/usr/include
LIBNAME=cxxprof
CXX=clang++-3.8
DOXYGEN=doxygen
INC=-Iprofiler
CFLAGS=-std=c++14 -g -O3 -Wall
LFLAGS=-lpthread -latomic
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj
TEST_DIR=$(BUILD_DIR)/tests
DOCS_DIR=$(BUILD_DIR)/docs
LIBPROFILER=$(BUILD_DIR)/lib/lib$(LIBNAME).so
LIBPATH=$(abspath $(dir $(LIBPROFILER)))
DEPDIR := .dependencies
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: lib unit stress perf docs

#
# Doxygen docs
#
docs:
	@mkdir -p $(TEST_DIR)
	$(DOXYGEN) Doxyfile

#
# Perf tests
#
PERF_TESTS_SRC=$(wildcard tests/perf/**/*.cpp)
PERF_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PERF_TESTS_SRC))

perf: $(TEST_DIR)/perf
	./build/tests/perf

$(TEST_DIR)/perf: $(PERF_TESTS_OBJ) $(LIBPROFILER)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -l$(LIBNAME) -L$(LIBPATH) -Wl,-R$(LIBPATH) -o $@ $^ -Wl,-Bstatic -L$(LIBDIRS) -lbenchmark -Wl,-Bdynamic

$(OBJ_DIR)/tests/perf/%.o: tests/perf/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC) -I$(INCDIRS) -c -o $@ $<
	-$(POSTCOMPILE)

#
# Stress tests
#
STRESS_TESTS_SRC=$(wildcard tests/stress/**/*.cpp) tests/stress/Main.cpp
STRESS_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(STRESS_TESTS_SRC))

stress: $(TEST_DIR)/stress
	./build/tests/stress --show_progress --log_level=test_suite

$(TEST_DIR)/stress: $(STRESS_TESTS_OBJ) $(LIBPROFILER)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -lboost_unit_test_framework -l$(LIBNAME) -L$(LIBPATH) -Wl,-R$(LIBPATH) -o $@ $^

$(OBJ_DIR)/tests/stress/%.o: tests/stress/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) -DBOOST_TEST_DYN_LINK $(CFLAGS) $(INC) -c -o $@ $<
	-$(POSTCOMPILE)

#
# Unit tests
#
UNIT_TESTS_SRC=$(wildcard tests/unit/**/*.cpp) tests/unit/Main.cpp
UNIT_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(UNIT_TESTS_SRC))

unit: $(TEST_DIR)/unit_tests
	./build/tests/unit_tests --show_progress --log_level=test_suite

$(TEST_DIR)/unit_tests: $(UNIT_TESTS_OBJ) $(LIBPROFILER)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -lboost_unit_test_framework -l$(LIBNAME) -L$(LIBPATH) -Wl,-R$(LIBPATH) -o $@ $^

$(OBJ_DIR)/tests/unit/%.o: tests/unit/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) -DBOOST_TEST_DYN_LINK $(CFLAGS) $(INC) -c -o $@ $<
	-$(POSTCOMPILE)


#
# Profiler library
#
PROFILER_SRC=$(wildcard profiler/**/*.cpp)
PROFILER_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PROFILER_SRC))

lib: $(LIBPROFILER)

$(LIBPROFILER): $(PROFILER_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(CFLAGS) $(LFLAGS) -o $@ $^

$(OBJ_DIR)/profiler/%.o: profiler/%.cpp $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CXX) -fPIC $(CFLAGS) $(INC) -c -o $@ $<
	-$(POSTCOMPILE)

#
# Headers
#
$(DEPDIR)/%.d:
	@mkdir -p $(DEPDIR)

.PRECIOUS: $(DEPDIR)/%.d

ALL_SRC=$(wildcard **/*.cpp)
-include $(patsubst %, $(DEPDIR)/%.d, $(basename $(ALL_SRC)))

clean:
	@rm build -rf

.PHONY : all lib headers unit stress clean docs
