cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

find_package(OpenCV REQUIRED)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        include
)

set(SOURCE_FILES
        src/main.cpp
        src/config.cpp
        src/metrics.cpp
        src/frame_analysis.cpp
        src/stream_analyzer.cpp
        src/helpers.cpp
        src/capture.cpp
        src/ffmpeg_capture.cpp
        src/gstreamer_capture.cpp
)


set(HEADER_FILES
        include/stream_analyzer.hpp
        include/config.hpp
        include/metrics.hpp
        include/frame_analysis.hpp
        include/threading.hpp
        include/helpers.hpp
        include/ffmpeg_capture.hpp
        include/icapture.hpp
        include/capture.hpp
        include/gstreamer_capture.hpp
)

add_executable(stream_analysis
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

target_link_libraries(stream_analysis
        ${OpenCV_LIBS}
        pthread
)

pkg_check_modules(FFMPEG libavformat libavcodec libavutil)
if (FFMPEG_FOUND)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    target_link_libraries(stream_analysis ${FFMPEG_LIBRARIES})
    target_compile_definitions(stream_analysis PRIVATE FFMPEG_ENABLED)
endif()

pkg_check_modules(GSTREAMER gstreamer-1.0 gstreamer-video-1.0)
if (GSTREAMER_FOUND)
    include_directories(${GSTREAMER_INCLUDE_DIRS})
    target_link_libraries(stream_analysis ${GSTREAMER_LIBRARIES})
    target_compile_definitions(stream_analysis PRIVATE GSTREAMER_ENABLED)
endif()
