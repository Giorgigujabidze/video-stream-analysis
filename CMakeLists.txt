cmake_minimum_required(VERSION 3.10)
project(untitled8)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(untitled8 main.cpp
        main.hpp
)
target_link_libraries(untitled8 ${OpenCV_LIBS})
