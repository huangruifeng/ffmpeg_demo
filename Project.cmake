
option(WITH_FFMPEG "This is a option for WITH_FFMPEG" OFF)
option(WITH_SAMPLES "This is a option for WITH_SAMPLES" OFF)
option(WITH_PROJECT_WIN32 "This is a option for WITH_WIN32" OFF)

message("output config: " ${PROJECT_NAME})
message("WITH_PROJECT_WIN32 " ${WITH_PROJECT_WIN32})
message("WITH_FFMPEG " ${WITH_FFMPEG})
message("WITH_SAMPLES " ${WITH_SAMPLES})

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
    file(COPY ${FFMPEG_DLLS} DESTINATION  ${PROJECT_BINARY_DIR}/Debug)
    message("move ffmpeg")
endif()

if(WITH_SAMPLES)
 file(COPY ${SAMPLES} DESTINATION ${PROJECT_BINARY_DIR}/Debug)
 message("move samples")
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/Debug")
endif()