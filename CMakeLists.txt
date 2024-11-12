cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF)

set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories("/home/gio/static_build_test/ffmpeg-7.1/include")

add_executable(stream_analysis
        main.cpp
        stream_analyzer.hpp
        config.hpp
        metrics.hpp
        frame_analysis.hpp
        config.cpp
        metrics.cpp
        frame_analysis.cpp
        stream_analyzer.cpp
)

set(FFMPEG_LIBS
        /home/gio/static_build_test/ffmpeg-7.1/libavcodec/libavcodec.a
        /home/gio/static_build_test/ffmpeg-7.1/libavformat/libavformat.a
        /home/gio/static_build_test/ffmpeg-7.1/libavutil/libavutil.a
        /home/gio/static_build_test/ffmpeg-7.1/libswscale/libswscale.a
)

target_link_libraries(stream_analysis ${OpenCV_LIBS} ${FFMPEG_LIBS} pthread dl m)

target_link_options(stream_analysis PRIVATE  -static-libgcc -static-libstdc++)
