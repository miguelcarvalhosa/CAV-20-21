cmake_minimum_required(VERSION 3.12)
project(classes)

set(CMAKE_CXX_STANDARD 17)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(example example.cpp Img.h)

add_executable(image opencv.cpp)
target_link_libraries( image ${OpenCV_LIBS} )

add_executable(video video.cpp)
target_link_libraries( video ${OpenCV_LIBS} )

add_executable(player player.cpp)
target_link_libraries( player ${OpenCV_LIBS} )

