
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")		# 程序输出文件为exe文件时起作用
    set(CMAKE_SHARED_LINKR_FLAGS "${CMAKE_SHARED_LINKR_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")		# 程序输出文件为dll文件时起作用
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKR_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")	        # 程序输出文件为lib文件时起作用
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO /NODEFAULTLIB:libc.lib")	# 程序输出文件为module文件时起作用
endif()

set(FFMPEG_HEADER ${FFMPEG_PATH}/include)
set(FFMPEG_BIN ${FFMPEG_PATH}/bin)
set(FFMPEG_LIB ${FFMPEG_PATH}/lib)


include_directories(${FFMPEG_HEADER})
link_directories(${FFMPEG_LIB})


file(GLOB FFMPEG_DLLS
    ${FFMPEG_BIN}/*.dll
)

list(APPEND FFMPEG_LIBS
    avformat
    swresample
    avcodec
    swscale
    avutil
)
