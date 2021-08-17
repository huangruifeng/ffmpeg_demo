
cmake_minimum_required(VERSION 3.0.0)
project(rgb_to_video_format VERSION 0.1.0)

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)


add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}  ${FFMPEG_LIBS} )

file(GLOB SAMPLES
    ${SAMPLE_PATH}/*
)
message(${SAMPLES})

file(COPY ${FFMPEG_DLLS} DESTINATION  ${PROJECT_BINARY_DIR}/Debug FOLLOW_SYMLINK_CHAIN)
file(COPY ${SAMPLES} DESTINATION ${PROJECT_BINARY_DIR}/Debug FOLLOW_SYMLINK_CHAIN)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/Debug")
endif()