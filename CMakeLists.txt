cmake_minimum_required(VERSION 2.8.8)
project(qnodeeditor)

add_definitions(-std=c++11)
set(CMAKE_BUILD_TYPE DEBUG)

# set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

# add additional warnings when compiling with gcc or clang
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC OR (CMAKE_C_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	# set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC OR (CMAKE_C_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# main library
include_directories(src)
add_subdirectory(src)

# demo applications
# add_subdirectory(examples/simple-data-forwarding)
