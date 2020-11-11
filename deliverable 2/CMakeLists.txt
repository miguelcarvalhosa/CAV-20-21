cmake_minimum_required(VERSION 3.17)
project(deliverable_2)

set(CMAKE_CXX_STANDARD 14)

add_executable(point1 point1.cpp BitStream_Write.cpp BitStream_Write.h BitStream_Read.cpp BitStream_Read)
add_executable(point2 point2.cpp BitStream_Write.cpp BitStream_Write.h BitStream_Read.cpp BitStream_Read GolombEncoder.cpp GolombEncoder.h GolombDecoder.cpp GolombDecoder.h)