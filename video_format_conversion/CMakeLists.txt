
cmake_minimum_required(VERSION 3.0.0)
project(video_format_conversion VERSION 0.1.0)

list (APPEND SRC
    main.cpp
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