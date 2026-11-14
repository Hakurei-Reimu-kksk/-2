cmake_minimum_required(VERSION 3.10)
project(Homework2)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(OpenCV REQUIRED)
include_directories(include ${OpenCV_INCLUDE_DIRS})
add_executable(homework_2 
    src/main.cpp 
    src/Detector.cpp
)
target_link_libraries(homework_2 ${OpenCV_LIBS})