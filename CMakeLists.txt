cmake_minimum_required(VERSION 2.8)
project("PROFILER")
# TODO: debug and release
add_compile_options(-std=c++14 -stdlib=libc++ -Wall)

find_package(Threads)
find_library(LIBCXX c++)

include_directories(${PROFILER_SOURCE_DIR}/profiler)
file(GLOB PROFILER_SRC "profiler/**/*.cpp")
add_library(cxxprofiler SHARED ${PROFILER_SRC})
target_link_libraries(cxxprofiler "-stdlib=libc++" ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

# TODO: Should we make boost optional?
find_package(Boost 1.59.0 REQUIRED COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB unit_tests_src "tests/unit/**/*.cpp" "tests/unit/*.cpp")
add_executable(unit_tests ${unit_tests_src})
target_compile_definitions(unit_tests PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(unit_tests cxxprofiler "-stdlib=libc++" ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

enable_testing()
add_test(unit unit_tests)

# link_directories(${PROFILER_SOURCE_DIR}/profiler)

# Maybe for examples?
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# add_subdirectory(examples)
