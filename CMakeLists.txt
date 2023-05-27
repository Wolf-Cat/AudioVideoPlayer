cmake_minimum_required(VERSION 3.22)
project(AudioVideoPlayer)

set(CMAKE_CXX_STANDARD 11)

include_directories(D:/ffmpeg-6.0-full_build-dev/include)
link_directories(D:/ffmpeg-6.0-full_build-dev/lib)

add_executable(AudioVideoPlayer main.cpp)

#添加动态库
target_link_libraries(
        AudioVideoPlayer
        avformat
        avutil
)

