NAME?=cxxprof
BUILD_DIR?=build
LIB=$(BUILD_DIR)/lib/lib$(NAME).so

LIB_DIRS?=/usr/lib
INC_DIRS?=/usr/include

CXX?=clang++
DOXYGEN?=doxygen
CFLAGS?=-std=c++14 -g -Wall -pthread

# Use libc++ w/ Clang by default:
ifndef USE_LIBCXX
  ifneq (,$(findstring clang, $(CXX)))
    USE_LIBCXX=1
  else
    USE_LIBCXX=0
  endif
endif

# Using libc++
ifeq ($(USE_LIBCXX), 1)
  ifneq (,$(findstring clang, $(CXX)))
    # Clang:
    CFLAGS+=-stdlib=libc++
  else
    CFLAGS+=-nostdinc++ -nodefaultlibs
  endif
endif

LFLAGS?=-latomic
ifeq ($(USE_LIBCXX), 1)
  ifneq (,$(findstring clang, $(CXX)))
    # Clang: do nothing
  else
    LFLAGS+=-lc++ -lsupc++ -lm -lc -lgcc_s -lgcc
  endif
endif

ifdef DEBUG
  CFLAGS+=-O2 -fno-omit-frame-pointer -DDEBUG
else
  ifdef COVERAGE
    CFLAGS+=-O0 --coverage
    LFLAGS+=--coverage
  else
    CFLAGS+=-O3
  endif
endif

SAN_COMMON=-fsanitize-blacklist=sanitize_blacklist.txt
ifdef ASAN
  CFLAGS+=-fsanitize=address $(SAN_COMMON)
  export ASAN_OPTIONS=check_initialization_order=1,detect_stack_use_after_return=1
endif
ifdef MSAN
  CFLAGS+=-fsanitize=memory -fsanitize-memory-track-origins=2 $(SAN_COMMON)
endif
ifdef TSAN
  CFLAGS+=-fsanitize=thread $(SAN_COMMON)
endif
ifdef USAN
  CFLAGS+=-fsanitize=undefined $(SAN_COMMON)
  export USAN_OPTIONS=print_stacktrace=1
endif

OBJ_DIR=$(BUILD_DIR)/obj
TEST_DIR=$(BUILD_DIR)/tests
DOCS_DIR=$(BUILD_DIR)/docs

LIB_DIRS+=$(abspath $(dir $(LIB)))
LIB_FLAG=$(foreach dir, $(LIB_DIRS), -L$(dir))
INC_FLAG=-Iprofiler $(foreach dir, $(INC_DIRS), -I$(dir))

RPATH_FLAG=$(foreach dir, $(LIB_DIRS), -Wl,-R$(dir))

all: lib unit stress perf docs

#
# Doxygen docs
#
docs:
	@mkdir -p $(DOCS_DIR)
	$(DOXYGEN) Doxyfile

#
# Perf tests
#
PERF_TESTS_SRC=$(wildcard tests/perf/**/*.cpp)
PERF_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PERF_TESTS_SRC))

perf: $(TEST_DIR)/perf
	./build/tests/perf

$(TEST_DIR)/perf: $(PERF_TESTS_OBJ) $(LIB)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -l$(NAME) $(RPATH_FLAG) -o $@ $^ -Wl,-Bstatic $(LIB_FLAG) -lsupc++ -lbenchmark -Wl,-Bdynamic

$(OBJ_DIR)/tests/perf/%.o: tests/perf/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INC_FLAG) -c -o $@ $<

#
# Stress tests
#
STRESS_TESTS_SRC=$(wildcard tests/stress/**/*.cpp) tests/stress/Main.cpp
STRESS_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(STRESS_TESTS_SRC))

stress: $(TEST_DIR)/stress
	./build/tests/stress --show_progress --log_level=test_suite

$(TEST_DIR)/stress: $(STRESS_TESTS_OBJ) $(LIB)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -l$(NAME) -lboost_unit_test_framework $(LIB_FLAG) $(RPATH_FLAG) -o $@ $^

$(OBJ_DIR)/tests/stress/%.o: tests/stress/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -DBOOST_TEST_DYN_LINK $(CFLAGS) $(INC_FLAG) -c -o $@ $<

#
# Unit tests
#
UNIT_TESTS_SRC=$(wildcard tests/unit/**/*.cpp) tests/unit/Main.cpp
UNIT_TESTS_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(UNIT_TESTS_SRC))

unit: $(TEST_DIR)/unit_tests
	./build/tests/unit_tests --show_progress --log_level=test_suite

$(TEST_DIR)/unit_tests: $(UNIT_TESTS_OBJ) $(LIB)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CFLAGS) $(LFLAGS) -l$(NAME) -lboost_unit_test_framework $(LIB_FLAG) $(RPATH_FLAG) -o $@ $^

$(OBJ_DIR)/tests/unit/%.o: tests/unit/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -DBOOST_TEST_DYN_LINK $(CFLAGS) $(INC_FLAG) -c -o $@ $<

#
# Profiler library
#
PROFILER_SRC=$(wildcard profiler/**/*.cpp)
PROFILER_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PROFILER_SRC))

lib: $(LIB)

$(LIB): $(PROFILER_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) -shared $(CFLAGS) $(LFLAGS) $(LIB_FLAG) -o $@ $^

$(OBJ_DIR)/profiler/%.o: profiler/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -fPIC $(CFLAGS) $(INC_FLAG) -c -o $@ $<

clean:
	@rm build -rf

.PHONY : all lib unit stress clean docs
