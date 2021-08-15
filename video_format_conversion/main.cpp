#include<iostream>
#include<string>
#include<sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

/*
 *  (0)avformat_open_input -------------------------------------------------
 *   |                                                                     |
 *	(1)avformat_alloc_output_context2---------------------------------------
 *   |                                                                     |
 *	(2)avio_open  ------------------------- (8)avio_close ------------------------------
 *	 |                                                                     |           |
 *	(3)avformat_new_stream-----------------------------------------        |           |
 *   |                                                            |        |           |
 *	(4)avcodec_parameters_copy-----------------------------       |        |           |
 *	 |                                                    |       |        |           |
 *	(5)avformat_write_header------------------------------|-------|--------|           |
 *	 |                                                    |   AVstream     |           |
 *	(6)av_read_frame                                      |       |        |           |
 *	 |                                                    |       |-AVFormatContext--AVIOContext                                               
 *	 |  AVPacket                                          |       |          
 *	 |                                                    - AVCodecParameters
 *	(7)av_interleaved_write_frame-------------------------------------------
 *	 |
 *	(8)av_write_trailer
 */

/**
 * \brief 
 * \param x log message
 */
#define LOG_ERROR(x) \
	std::stringstream ss;\
	ss << x;\
	std::cout<<  ss.str() << std::endl ;

//#define UseTimeBase //error 
#define  Debug

//RTTI
struct AVIOContextDeleter
{
	void operator()(AVIOContext* p) const
    {
		avio_close(p);
	}
};

struct AVFormatContextInputDeleter
{
    void operator()(AVFormatContext *p) const
    {
		//todo
		avformat_close_input(&p);
    }
};

struct AVFormatContextOutputDeleter
{
	void operator()(AVFormatContext* p) const
	{
		avformat_free_context(p);
	}
};

struct AVPacketUnrefDeleter
{
	void operator()(AVPacket* p) const
	{
		av_packet_unref(p);
	}
};

int main(int argc, char* argv[])
{
	std::string in_file = "test.mp4";
	std::string out_file = "test.mov";


	av_register_all();

	AVFormatContext* input = nullptr;
	AVFormatContext* output = nullptr;
    int error = avformat_open_input(&input, in_file.c_str(), nullptr, nullptr);
	if(error != 0)
	{
		LOG_ERROR("avformat_open_input failed. error code is " << error);
		return -1;
	}
	std::unique_ptr<AVFormatContext, AVFormatContextInputDeleter> input_context(input);

	error = avformat_alloc_output_context2(&output, nullptr, nullptr, out_file.c_str());
	if(error < 0 )
	{
		LOG_ERROR("avformat_alloc_output_context2 failed. error code is " << error);
		return  -2;
	}
	std::unique_ptr<AVFormatContext, AVFormatContextOutputDeleter> output_context(output);

	for (unsigned i =0; i < input_context->nb_streams; i++)
	{
		AVStream * stream = avformat_new_stream(output_context.get(), nullptr);
		avcodec_parameters_copy(stream->codecpar, input_context->streams[i]->codecpar);
		stream->codecpar->codec_tag = 0;

#ifdef  UseTimeBase
		stream->time_base = input_context->streams[i]->time_base;
#endif
	}

# ifdef  Debug
	av_dump_format(input_context.get(), 0, in_file.c_str(), 0);
	std::cout << std::endl;
	av_dump_format(output_context.get(), 0, out_file.c_str(), 1);
#endif

	error = avio_open(&output_context->pb, out_file.c_str(), AVIO_FLAG_WRITE);
	if(error < 0)
	{
		LOG_ERROR("avio_open failed.error code is" << error);
		return -3;
	}
	std::unique_ptr<AVIOContext,AVIOContextDeleter> avio_context (output_context->pb);

	error = avformat_write_header(output_context.get(), nullptr);
	if(error < 0)
	{
		LOG_ERROR("avformat_write_header failed. error code is " << error);
		return -4;
	}

	AVPacket pkt;
	while(true)
	{
		//RTTI
		std::unique_ptr<AVPacket, AVPacketUnrefDeleter> paket_ptr(&pkt);
		error = av_read_frame(input_context.get(), paket_ptr.get());
		if (error < 0)
			break;

#ifndef  UseTimeBase

		pkt.pts = av_rescale_q_rnd(pkt.pts, 
			input_context->streams[pkt.stream_index]->time_base, 
			output_context->streams[pkt.stream_index]->time_base, 
			static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

		pkt.dts = av_rescale_q_rnd(pkt.dts,
			input_context->streams[pkt.stream_index]->time_base,
			output_context->streams[pkt.stream_index]->time_base,
			static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

		pkt.duration = av_rescale_q_rnd(pkt.duration,
			input_context->streams[pkt.stream_index]->time_base,
			output_context->streams[pkt.stream_index]->time_base,
			static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
#endif

		av_write_frame(output_context.get(), &pkt);
	}

	av_write_trailer(output_context.get());
	return 0;
}