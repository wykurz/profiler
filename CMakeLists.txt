cmake_minimum_required(VERSION 2.8)
project("profiler")

set(VERSION_MAJOR 0 CACHE STRING "Project major version number.")
set(VERSION_MINOR 0 CACHE STRING "Project minor version number.")
set(VERSION_PATCH 1 CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

option(USE_LIBCXX "Use libc++ (instead of stdlibc++). Enables sanitizer build types." OFF)
option(DEBUG_LOG "Debug log printing." OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
if (DEBUG_LOG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG_LOG")
endif()
# Release builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
# Debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fno-omit-frame-pointer -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -DDEBUG")
# Coverage builds
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

find_package(Threads REQUIRED)

include_directories(${profiler_SOURCE_DIR})
file(GLOB profiler_SRC "Profiler/**/*.cpp")
add_library(cxxprofiler INTERFACE ${profiler_SRC})

# TODO: With C++17 support we can drop the boost::filesystem library
find_package(Boost 1.62.0 COMPONENTS filesystem REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
# Making MSVC happy:
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_ALL_DYN_LINK)

if (USE_LIBCXX)
  message(STATUS "Configuring project to use libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  # Sanitizer builds
  set(CMAKE_CXX_FLAGS_ASAN_USAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=address,undefined")
  set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=thread")
  set(CXXPROFILER_DEPS ${CMAKE_THREAD_LIBS_INIT} supc++ ${Boost_LIBRARIES})
  # Profiler library link:
  # target_link_libraries(cxxprofiler ${CMAKE_THREAD_LIBS_INIT} supc++ ${Boost_LIBRARIES})
else ()
  message(STATUS "Configuring project to use stdlibc++, sanitizer builds disabled")
  set(CXXPROFILER_DEPS ${CMAKE_THREAD_LIBS_INIT} atomic ${Boost_LIBRARIES})
  # target_link_libraries(cxxprofiler ${CMAKE_THREAD_LIBS_INIT} atomic ${Boost_LIBRARIES})
endif ()

add_subdirectory(tests)
add_subdirectory(examples)

find_package(Doxygen)
if (Doxygen_FOUND)
  set(doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  set(doxygen_main_page ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
  configure_file(${doxyfile.in} ${doxyfile} @ONLY)
  add_custom_target(
    docs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else ()
  message(STATUS "Doxygen not found, building documentation disabled")
endif ()
