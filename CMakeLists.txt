cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(stream_analysis main.cpp
        stream_analyzer.hpp
        config.hpp
        metrics.hpp
        frame_analysis.hpp
        config.cpp
        metrics.cpp
        frame_analysis.cpp
        stream_analyzer.cpp
)
target_link_libraries(stream_analysis ${OpenCV_LIBS})
