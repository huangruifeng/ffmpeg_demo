cmake_minimum_required(VERSION 3.0.0)
project(ffmpeg_demo VERSION 0.1.0)
set(BUILD_TYPE  "Debug")

set(SAMPLE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/samples)
set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(FFMPEG_PATH ${PROJECT_PATH}/ffmpeg)
include(${FFMPEG_PATH}/ffmpeg.cmake)

message(${PROJECT_PATH})


add_subdirectory(ffmpeg_test)
add_subdirectory(video_format_conversion)
add_subdirectory(rgb_to_video_format)
add_subdirectory(d3d_render)
add_subdirectory(pcm_to_aac)