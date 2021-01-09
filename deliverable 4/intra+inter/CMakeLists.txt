cmake_minimum_required(VERSION 3.17)
project(deliverable_4_intra_inter)

find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIR})

SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")


add_executable(testeCodec testeCodec.cpp BitStream_Write.h BitStream_Read.h BitStream_Write.cpp BitStream_Read.cpp GolombDecoder.cpp GolombDecoder.h GolombEncoder.cpp GolombEncoder.h VideoCodec.cpp VideoCodec.h)
target_link_libraries(testeCodec ${OpenCV_LIBS})