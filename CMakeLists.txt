cmake_minimum_required(VERSION 3.25)
project(x264_test C)

set(CMAKE_C_STANDARD 99)

add_executable(x264_test main.c socket.c screen_win32.c)

target_link_libraries(x264_test x264)
if (WIN32)
    target_link_libraries(x264_test ws2_32)
endif ()
