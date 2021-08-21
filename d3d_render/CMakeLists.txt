
cmake_minimum_required(VERSION 3.0.0)
project(d3d_render VERSION 0.1.0)

add_definitions(-DUNICODE -D_UNICODE)

set(WITH_FFMPEG OFF)
set(WITH_PROJECT_WIN32 ON)
set(WITH_SAMPLES ON)
include(${CMAKE_SOURCE_DIR}/Project.cmake)