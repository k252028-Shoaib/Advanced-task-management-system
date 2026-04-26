cmake_minimum_required(VERSION 4.3.2) 
project(AdvancedTaskManager VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header search paths
include_directories(include)
include_directories(external/imgui)
include_directories(external/imgui/backends)
include_directories(external/glfw/include)

# Gather source files
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# ImGui core and backend sources (must be compiled into the exe)
set(IMGUI_FILES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_stdlib.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(program ${SRC_FILES} ${IMGUI_FILES})

# Force the executable to the project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Linking order is critical for MinGW
target_link_libraries(program PRIVATE 
    "${CMAKE_SOURCE_DIR}/external/glfw/lib/libglfw3.a"
    opengl32
    gdi32
    imm32
)