cmake_minimum_required(VERSION 2.8)
project("PROFILER")

add_compile_options(-std=c++14 -stdlib=libc++ -Wall)
# Release builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -fno-omit-frame-pointer -DDEBUG")
# Debug builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
# Coverage builds
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

find_package(Threads)
# Add libc++ dependency in case the library is in non-standard location. Making the dependency explicit will force
# cmake to pass proper -L flags.
find_library(LIBCXX c++)

include_directories(${PROFILER_SOURCE_DIR}/profiler)
file(GLOB PROFILER_SRC "profiler/**/*.cpp")
add_library(cxxprofiler SHARED ${PROFILER_SRC})
target_link_libraries(cxxprofiler "-stdlib=libc++" ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

# TODO: Should we make boost optional?
find_package(Boost 1.58.0 REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB unit_tests_src "tests/unit/**/*.cpp" "tests/unit/Main.cpp")
add_executable(unit_tests ${unit_tests_src})
target_compile_definitions(unit_tests PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(unit_tests cxxprofiler "-stdlib=libc++" ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

file(GLOB stress_tests_src "tests/stress/**/*.cpp" "tests/stress/Main.cpp")
add_executable(stress_tests ${stress_tests_src})
target_compile_definitions(stress_tests PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(stress_tests cxxprofiler "-stdlib=libc++" ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

find_library(BENCHMARK benchmark)
include_directories(${BENCHMARK_INCLUDE_DIRS})

file(GLOB perf_tests_src "tests/perf/**/*.cpp" "tests/perf/Main.cpp")
add_executable(perf_tests ${perf_tests_src})
target_link_libraries(perf_tests cxxprofiler "-stdlib=libc++" ${BENCHMARK} ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

enable_testing()
add_test(unit unit_tests)
add_test(stress stress_tests)

# link_directories(${PROFILER_SOURCE_DIR}/profiler)

# Maybe for examples?
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# add_subdirectory(examples)
