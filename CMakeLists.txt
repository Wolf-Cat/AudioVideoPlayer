cmake_minimum_required(VERSION 3.22)
project(AudioVideoPlayer)

set(CMAKE_CXX_STANDARD 11)

include_directories(D:/ffmpeg-6.0-full_build-dev/include)
include_directories(D:/SDL-mingw-VC/SDL2-2.26.5-mingw/x86_64-w64-mingw32/include)
include_directories(./)

link_directories(D:/ffmpeg-6.0-full_build-dev/lib)
link_directories(D:/SDL-mingw-VC/SDL2-2.26.5-mingw/x86_64-w64-mingw32/lib)

add_executable(AudioVideoPlayer
        AVGlobal.cpp
        AVPacketQueue.cpp
        VideoFrameQueue.cpp
        main.cpp)

#添加动态库
target_link_libraries(AudioVideoPlayer
        avformat
        avutil
        avcodec
)

target_link_libraries(AudioVideoPlayer
        mingw32
        SDL2main
        SDL2)

