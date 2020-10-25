project(deliverable1)

find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIR})

cmake_minimum_required(VERSION 2.8)

SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

# POINT 1 COMPILATION
add_executable (point1 point1.cpp point5.cpp)
target_link_libraries (point1 sndfile)

# POINT 2 COMPILATION
add_executable (point2 point2.cpp)
target_link_libraries(point2 ${OpenCV_LIBS} )
#add_executable (wavhist wavhist.cpp)
#target_link_libraries (wavhist sndfile)

# POINT 5 COMPILATION
add_executable (point5 point5.cpp)
target_link_libraries(point5 ${OpenCV_LIBS} )