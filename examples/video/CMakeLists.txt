cmake_minimum_required(VERSION 3.12)
project(PlayerYUV444)

set(CMAKE_CXX_STANDARD 17)

MESSAGE ("\n==> Finding 3rd Party Packages")
find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIR})

add_executable( PlayerYUV444 PlayerYUV444.cpp )
target_link_libraries( PlayerYUV444 ${OpenCV_LIBS} )
