cmake_minimum_required(VERSION 4.3.2)

# Name your project
project(AdvancedTaskManager VERSION 1.0)

# Enforce C++20 standard (Crucial for the <chrono> calendar math)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tell CMake where your header files (.hpp) will live
include_directories(include)

# Tell CMake to build an executable named "TaskManager" using main.cpp
add_executable(Program 
    src/main.cpp
    src/Date/date.cpp
    src/task_structure
    src/data_management/data_management.cpp
    )
