#include<iostream>
#include<string>
#include<sstream>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

#define  DEBUG
/**
 * \brief 
 * \param x log message
 */
#define LOG(x)\
    std::stringstream ss;\
	ss << x;\
	std::cout<<  ss.str() << std::endl

#define LOG_ERROR(x) LOG(x)


#define LOG_INFO(x) LOG(x)

#ifdef DEBUG
#define LOG_DEBUG(x) LOG(x)
#else
#define LOG_DEBUG(x)
#endif

/**
 * \brief width
 */
#define  W  848
#define  H   480
#define  C  4

//RAII
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

struct AVCodecContextDeleter
{
    void operator()(AVCodecContext *p) const
    {
		//todo
		avcodec_free_context(&p);
    }
};

struct AVFrameDeleter
{
    void operator()(AVFrame*p) const
    {
		av_frame_free(&p);
    }
};

struct AVpacketDeleter
{
	void operator()(AVPacket* p) const
	{
		av_packet_free(&p);
	}
};

int write_rgb_file(const std::string& path)
{
    constexpr int width = W;
    constexpr int height = H;
    constexpr int channel = C;

	std::ofstream file(path, std::ios::binary);
	if(!file.is_open())
	{
		LOG_ERROR("write_rgb_file open file failed!");
		return  -1;
	}

	char tmp = 255;
	char zero = 0;

	int frameCount = 100;
	while (frameCount > 0)
	{
		for (int j = 0; j < height; j++)
		{
			int b = j * width * channel;
			for (int i = 0; i < width * channel; i += channel)
			{
				for (int n = 0; n < channel; n++)
				{
					if (n == 2)
					{
						file.write(&zero, 1);
					}
					else
					{
						file.write(&tmp, 1);
					}
				}
			}
		}
		frameCount--;
		tmp--;
	}

	file.close();
    return 0;
}

int main(int argc, char* argv[])
{
	std::string in_file = "out.rgb";
	std::string out_file = "rgb.mp4";

	//write_rgb_file(in_file);

	av_register_all();
	avcodec_register_all();

	std::ifstream file(in_file, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		LOG_ERROR("open  file failed!");
		return -1;
	}

	int width = W;
	int height = H;
	int channel = C;
	int fps = 60;

	//1 find encoder
	AVCodecID condec_id = AV_CODEC_ID_H264;
	AVCodec* codec = avcodec_find_encoder(condec_id);
	if (!codec)
	{
		LOG_ERROR("avcodec_find_encoder AV_CODEC_ID_H264 failed!");
		return -2;
	}

	//2 alloc codec context
	std::unique_ptr<AVCodecContext, AVCodecContextDeleter> context(avcodec_alloc_context3(codec));
	if (!context)
	{
		LOG_ERROR("avcodec_alloc_context3 afailed!");
		return -3;
	}
	context->time_base = { 1,fps };
	context->framerate = { fps,1 };
	context->bit_rate = 400000000;
	context->width = width;
	context->height = height;
	context->gop_size = 900;
	context->max_b_frames = 80;
	context->has_b_frames = 400;
	context->pix_fmt = AV_PIX_FMT_YUV420P;
	context->codec_id = condec_id;
	context->thread_count = 16;
	context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	//3 open codec
	int error = avcodec_open2(context.get(), codec, nullptr);
	if (error < 0)
	{
		LOG_ERROR("avcodec_open2 failed!");
		return -4;
	}

	//create out context
	AVFormatContext* oc = nullptr;
	avformat_alloc_output_context2(&oc, nullptr, nullptr, out_file.c_str());
	std::unique_ptr<AVFormatContext, AVFormatContextOutputDeleter> out_context(oc);

	AVStream* video_stream = avformat_new_stream(oc, nullptr);
	//video_stream->codec = context.get();
	//video_stream->id = 0;
	//video_stream->codecpar->codec_tag = 0;
	avcodec_parameters_from_context(video_stream->codecpar, context.get());

# ifdef  DEBUG
	av_dump_format(out_context.get(), 0, out_file.c_str(), 1);
#endif

	//rgb to yuv context
	SwsContext* ctx = nullptr;
	ctx = sws_getCachedContext(ctx,
		width, height, AV_PIX_FMT_BGRA,
		width, height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC,
		nullptr, nullptr, nullptr
	);
	//rgb buffer
	std::unique_ptr<uint8_t, std::default_delete<uint8_t[]>> rgb(new uint8_t[width * height * channel]);

	//yuv Frame
	std::unique_ptr<AVFrame, AVFrameDeleter> yuv(av_frame_alloc());
	yuv->format = AV_PIX_FMT_YUV420P;
	yuv->width = width;
	yuv->height = height;

	error = av_frame_get_buffer(yuv.get(), 32);
	if (error != 0)
	{
		LOG_ERROR("av_frame_get_buffer failed! error is : " << error);
		return -5;
	}

	//open mp4 file
	error = avio_open(&out_context->pb, out_file.c_str(), AVIO_FLAG_WRITE);
	if (error < 0)
	{
		LOG_ERROR("avio_open failed error is " << error);
		return -6;
	}
	std::unique_ptr<AVIOContext, AVIOContextDeleter> avio_context(out_context->pb);

	error = avformat_write_header(out_context.get(), nullptr);
	if (error < 0)
	{
		LOG_ERROR("avformat_write_header failed error is : " << error);
	}

	std::unique_ptr<AVPacket, AVpacketDeleter> pkt(av_packet_alloc());

	int pts = 0;
	while (!file.eof())
	{
		file.read(reinterpret_cast<char*>(rgb.get()), static_cast<long long>(width) * height * channel);
		if (file.eof())
			break;
		uint8_t* in_data[AV_NUM_DATA_POINTERS] = { rgb.get(),nullptr };

		int in_line_size[AV_NUM_DATA_POINTERS] = { width * channel,0 };

		//rgb to yuv
		auto rh = sws_scale(ctx, in_data, in_line_size, 0, height, yuv->data, yuv->linesize);
		if (rh <= 0)
		{
			LOG_ERROR("sws_scale error, h is " << rh);
			break;
		}
		yuv->pts = pts;
		pts += video_stream->time_base.den / context->framerate.num;

		error = avcodec_send_frame(context.get(), yuv.get());
		if (error != 0)
		{
			LOG_DEBUG("avcodec_send_frame failed");
			continue;
		}

		av_init_packet(pkt.get());
		do {
			error = avcodec_receive_packet(context.get(), pkt.get());
			LOG_DEBUG(pkt->size);

			if (AVERROR_EOF == error)
			{
				LOG_DEBUG("AVERROR_EOF");
				break;
			}
			else if (AVERROR(EAGAIN) == error)
			{
				LOG_DEBUG("AVERROR(EAGAIN)");
				break;
			}
			else if (error != 0)
			{
				LOG_DEBUG("AVERROR(EAGAIN)");
				break;
			}
			//av_write_frame(out_context.get(), pkt.get());
			//av_packet_unref(pkt.get());
			av_interleaved_write_frame(out_context.get(), pkt.get());

		} while (true);

	}

	LOG_DEBUG("begin avcodec_send_frame nullptr !");
	error = avcodec_send_frame(context.get(), nullptr);
	if (error != 0)
	{
		LOG_ERROR("avcodec_send_frame failed!");
	}

	do {
		error = avcodec_receive_packet(context.get(), pkt.get());
		LOG_DEBUG(pkt->size);
		//av_write_frame(out_context.get(), pkt.get());
		//av_packet_unref(pkt.get());
		if (AVERROR_EOF == error)
		{
			LOG_INFO("end AVERROR_EOF");
			break;
		}
		else if (AVERROR(EAGAIN) == error)
		{
			LOG_INFO("end AVERROR(EAGAIN)");
			break;
		}
		else if (error != 0)
		{
			LOG_DEBUG("end avcodec_receive_packet error " << error);
			break;
		}
		av_interleaved_write_frame(out_context.get(), pkt.get());

	} while (true);

	av_write_trailer(out_context.get());
	file.close();

	return 0;
}
