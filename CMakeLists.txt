cmake_minimum_required(VERSION 3.25)
project(x264_test C)

set(CMAKE_C_STANDARD 99)

add_executable(x264_test main.c socket.c)

target_link_libraries(x264_test x264)
