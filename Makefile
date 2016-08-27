CXX=clang++-3.8
CXXFLAGS=-std=c++11 -g -O3 -Wall -I.
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/obj

PROFILER_SRC=$(filter-out tests/**/*.cpp, $(wildcard **/*.cpp))
PROFILER_OBJ=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(PROFILER_SRC))

# @echo '$(PROFILER_SRC)'

build_dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o : $(PROFILER_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< $(LIB_PATH) $(LIBS) -o $@

profiler: $(PROFILER_OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

all: profiler

.PHONY : all
