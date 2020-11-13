cmake_minimum_required(VERSION 3.17)
project(deliverable_2)

set(CMAKE_CXX_STANDARD 14)
SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

add_executable(point1 point1.cpp BitStream_Write.cpp BitStream_Write.h BitStream_Read.cpp BitStream_Read)
add_executable(point2 point2.cpp BitStream_Write.cpp BitStream_Write.h BitStream_Read.cpp BitStream_Read GolombEncoder.cpp GolombEncoder.h GolombDecoder.cpp GolombDecoder.h)

add_executable(test_BitStream test_BitStream.cpp main_test.cpp BitStream_Write.h BitStream_Read.h BitStream_Write.cpp BitStream_Read.cpp)
target_link_libraries (test_BitStream CppUTest)

add_executable(test_Golomb test_Golomb.cpp main_test.cpp BitStream_Write.h BitStream_Read.h BitStream_Write.cpp BitStream_Read.cpp GolombDecoder.cpp GolombDecoder.h GolombEncoder.cpp GolombEncoder.h)
target_link_libraries (test_Golomb CppUTest)
