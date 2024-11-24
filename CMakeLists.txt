cmake_minimum_required(VERSION 3.10)
project(stream_analysis)

set(CMAKE_CXX_STANDARD 20)

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
)

# Header files
set(HEADER_FILES
        include/stream_analyzer.hpp
        include/config.hpp
        include/metrics.hpp
        include/frame_analysis.hpp
        include/threading.hpp
        include/helpers.hpp
)

add_executable(stream_analysis
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

target_link_libraries(stream_analysis
        ${OpenCV_LIBS}
        pthread
)
