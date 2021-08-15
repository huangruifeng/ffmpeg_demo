
cmake_minimum_required(VERSION 3.0.0)
project(ffmpeg_test VERSION 0.1.0)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")		# 程序输出文件为exe文件时起作用
    set(CMAKE_SHARED_LINKR_FLAGS "${CMAKE_SHARED_LINKR_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")		# 程序输出文件为dll文件时起作用
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKR_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")	        # 程序输出文件为lib文件时起作用
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")	# 程序输出文件为module文件时起作用
endif()

add_executable(ffmpeg_test ${SOURCES})
target_link_libraries(${PROJECT_NAME}  ${FFMPEG_LIBS} )

file(COPY ${FFMPEG_DLLS} DESTINATION  ${PROJECT_BINARY_DIR}/Debug FOLLOW_SYMLINK_CHAIN)
