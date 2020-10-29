project(deliverable1)

find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIR})

cmake_minimum_required(VERSION 2.8)

SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -L/usr/lib/x86_64-linux-gnu/libsndfile.so.1.0.28")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

# POINT 1 COMPILATION
add_executable (point1 point1.cpp sndfile.hh)
target_link_libraries (point1 sndfile)

# POINT 2 COMPILATION
add_executable (point2 point2.cpp)
target_link_libraries(point2 ${OpenCV_LIBS} )

# POINT 3 COMPILATION
add_executable (point3_444 point3_444.cpp)
target_link_libraries(point3_444 ${OpenCV_LIBS} )

# POINT 3 COMPILATION
add_executable (point3_422 point3_422.cpp)
target_link_libraries(point3_422 ${OpenCV_LIBS} )

# POINT 3 COMPILATION
add_executable (point3_420 point3_420.cpp)
target_link_libraries(point3_420 ${OpenCV_LIBS} )

# POINT 4 COMPILATION
add_executable (point4 point4.cpp sndfile.hh)
target_link_libraries(point4 sndfile ${OpenCV_LIBS} )

# POINT 5 COMPILATION
add_executable (point5 point5.cpp)
target_link_libraries(point5 ${OpenCV_LIBS} )

# POINT 6 COMPILATION
add_executable(point6 point6.cpp sndfile.hh point6.cpp)
target_link_libraries(point6 sndfile)