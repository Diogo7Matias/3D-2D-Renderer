cmake_minimum_required(VERSION 3.16)
project(rendering-pipeline)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(rendering-pipeline WIN32 src/main.cpp src/mesh/geometry.cpp src/scene.cpp
    src/renderer.cpp src/window.cpp)

target_link_libraries(rendering-pipeline PRIVATE SDL3::SDL3)