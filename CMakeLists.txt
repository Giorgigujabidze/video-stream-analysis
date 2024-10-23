cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(stream_analysis stream_analysis.cpp
        stream_analysis.hpp
)
target_link_libraries(stream_analysis ${OpenCV_LIBS})
