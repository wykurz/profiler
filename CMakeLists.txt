cmake_minimum_required(VERSION 2.8)
project("Profiler")

add_compile_options(-std=c++14 -stdlib=libc++ -Wall)
# Release builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -fno-omit-frame-pointer -DDEBUG")
# Debug builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
# Coverage builds
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

find_package(Threads REQUIRED)
# Add libc++ dependency in case the library is in non-standard location. Making the dependency explicit will force
# cmake to pass proper -L flags.
find_library(LIBCXX c++)

include_directories(${Profiler_SOURCE_DIR}/profiler)
file(GLOB Profiler_SRC "profiler/**/*.cpp")
add_library(cxxprofiler SHARED ${Profiler_SRC})
target_link_libraries(cxxprofiler "-stdlib=libc++" ${CMAKE_THREAD_LIBS_INIT} ${LIBCXX})

add_subdirectory(tests)
