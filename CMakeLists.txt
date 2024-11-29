cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

include_directories(
        ${FFMPEG_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        include
)


add_definitions(${FFMPEG_CFLAGS_OTHER})

set(SOURCE_FILES
        src/main.cpp
        src/config.cpp
        src/metrics.cpp
        src/frame_analysis.cpp
        src/stream_analyzer.cpp
        src/helpers.cpp
)

set(HEADER_FILES
        include/stream_analyzer.hpp
        include/config.hpp
        include/metrics.hpp
        include/frame_analysis.hpp
        include/threading.hpp
        include/helpers.hpp
        src/frame.cpp
        include/frame.hpp
)

add_executable(stream_analysis
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

target_link_libraries(stream_analysis
        ${OpenCV_LIBS}
        ${FFMPEG_LIBRARIES}
        pthread
)
