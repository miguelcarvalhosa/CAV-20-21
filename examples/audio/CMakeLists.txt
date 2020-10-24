project(Sound)

cmake_minimum_required(VERSION 2.8)

SET (CMAKE_BUILD_TYPE "Release")
#SET (CMAKE_BUILD_TYPE "Debug")

#SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -I/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28include -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

add_executable (wavcp wavcp.cpp)
target_link_libraries (wavcp sndfile)

#add_executable (wavhist wavhist.cpp)
#target_link_libraries (wavhist sndfile)

