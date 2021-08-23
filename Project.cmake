message("***************** begin ${PROJECT_NAME} *****************")
option(WITH_FFMPEG "This is a option for WITH_FFMPEG" OFF)
option(WITH_SAMPLES "This is a option for WITH_SAMPLES" OFF)
option(WITH_PROJECT_WIN32 "This is a option for WITH_WIN32" OFF)
option(WITH_PCM_SAMPLES "generate test.pcm file" OFF)
option(WITH_RGB_SAMPLES "generate test.rgb file" OFF)

message("output config: " ${PROJECT_NAME})
message("BUILD_TYPE " ${BUILD_TYPE})
message("WITH_PROJECT_WIN32 " ${WITH_PROJECT_WIN32})
message("WITH_FFMPEG " ${WITH_FFMPEG})
message("WITH_SAMPLES " ${WITH_SAMPLES})
message("WITH_PCM_SAMPLES" ${WITH_PCM_SAMPLES})
message("WITH_RGB_SAMPLES" ${WITH_RGB_SAMPLES})

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

if(WITH_PROJECT_WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SRC})
else()
    add_executable(${PROJECT_NAME} ${SRC})
endif() 
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBS} )

file(GLOB SAMPLES
    ${SAMPLE_PATH}/*
)
message(${SAMPLES})

if(WITH_FFMPEG)
    file(COPY ${FFMPEG_DLLS} DESTINATION  ${PROJECT_BINARY_DIR}/${BUILD_TYPE})
    message("move ffmpeg")
endif()

if(WITH_SAMPLES)
    file(COPY ${SAMPLES} DESTINATION ${PROJECT_BINARY_DIR}/${BUILD_TYPE})
    message("move samples")
    set(FFMPEG_EXE ${FFMPEG_PATH}/bin/ffmpeg.exe)
    set(TEST_MP4 ${PROJECT_BINARY_DIR}/${BUILD_TYPE}/test.mp4)
    
    if(WITH_PCM_SAMPLES)
        set(TEST_PCM test.pcm)
        execute_process(COMMAND ${FFMPEG_EXE} -i  ${TEST_MP4} -f s16le ${TEST_PCM} WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${BUILD_TYPE})
    endif()

    if(WITH_RGB_SAMPLES)
        set(TEST_RGB test.rgb)
        execute_process(COMMAND ${FFMPEG_EXE} -i  ${TEST_MP4} -pix_fmt bgra ${TEST_RGB} WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${BUILD_TYPE})
    endif()
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/${BUILD_TYPE}")
endif()
message("***************** end ${PROJECT_NAME} *****************")