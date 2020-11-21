cmake_minimum_required(VERSION 3.17)
project(deliverable_3)

SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

add_executable(testGolomb testGolomb.cpp BitStream_Write.cpp BitStream_Write.h BitStream_Read.cpp BitStream_Read GolombEncoder.cpp GolombEncoder.h GolombDecoder.cpp GolombDecoder.h)

add_executable(point15 point15.cpp)
target_link_libraries(point15 sndfile)