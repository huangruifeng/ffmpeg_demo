#include<iostream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/mem.h>
#include <libavutil/fifo.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif


int main(int argc, char* argv[])
{

	av_register_all();

	avcodec_register_all();


	avformat_network_init();

	std::cout << "hello ffmpeg!" << std::endl;
	return 0;
}