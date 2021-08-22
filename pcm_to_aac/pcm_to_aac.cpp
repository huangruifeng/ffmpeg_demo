#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

#include<string>


#define  DEBUG

#include<sstream>
#include<iostream>
#include<fstream>
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

int main(int argc,char* argv)
{
    std::string in_file = "16.wav";
    std::string out_file = "out.aac";

    av_register_all();
    avcodec_register_all();

    AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);

    if (codec == nullptr)
    {
        LOG_ERROR("avcodec_find_encoder failed!");
        return -1;
    }

    std::shared_ptr<AVCodecContext> context(avcodec_alloc_context3(codec), [](AVCodecContext* c) {
        avcodec_free_context(&c);
    });

    context->bit_rate = 64000;
    context->sample_rate = 44100;
    context->sample_fmt = AV_SAMPLE_FMT_FLTP;
    context->channels = 2;
    context->channel_layout = AV_CH_LAYOUT_STEREO;
    context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    auto error = avcodec_open2(context.get(), codec, nullptr);
    if (error != 0)
    {
        LOG_ERROR("avcodec_open2 failed!");
        return -2;
    }

    AVFormatContext* frame_context = nullptr;
    error  = avformat_alloc_output_context2(&frame_context, nullptr, nullptr, out_file.c_str());
    std::shared_ptr<AVFormatContext>output_context(frame_context, [] (AVFormatContext* ptr){
        avformat_free_context(ptr);
    });
    if (error < 0)
    {
        LOG_ERROR("avformat_alloc_output_context2 failed!");
        return -3;
    }

    AVStream* st = avformat_new_stream(output_context.get(), nullptr);
    st->codecpar->codec_tag = 0;
    avcodec_parameters_from_context(st->codecpar, context.get());

#ifdef DEBUG
    av_dump_format(output_context.get(),0,out_file.c_str(),1);
#endif

    error = avio_open(&output_context->pb, out_file.c_str(), AVIO_FLAG_WRITE);
    if (error < 0)
    {
        LOG_ERROR("avio_open failed!");
        return -4;
    }

    error = avformat_write_header(output_context.get(),nullptr);
    if (error == AVSTREAM_INIT_IN_WRITE_HEADER)
    {
        LOG_DEBUG("AVSTREAM_INIT_IN_WRITE_HEADER");
    }

    SwrContext* actx      = nullptr;
    actx = swr_alloc_set_opts(actx, 
        context->channel_layout, context->sample_fmt, context->sample_rate,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
        0, 0);
    if (!actx)
    {
        LOG_ERROR("swr_alloc_set_opts failed!");
        return -5;
    }
    error = swr_init(actx);
    if (AVERROR(error))
    {
        LOG_ERROR("swr_init failed!");
        return -6;
    }
    std::shared_ptr<SwrContext> swrContext(actx, [](SwrContext* ptr) {
        swr_free(&ptr);
    });

    AVFrame* frame = av_frame_alloc();
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channels = 2;
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    frame->nb_samples = 1024;

    error = av_frame_get_buffer(frame,0);
    if (error < 0)
    {
        LOG_ERROR("av_frame_get_buffer failed!");
        return -7;
    }
    int read_size = frame->nb_samples * 2*2;

    std::shared_ptr<uint8_t> pcm(new uint8_t[read_size], std::default_delete<uint8_t[]>());
    std::ifstream file(in_file.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERROR("open file failed!");
        return -8;
    }

    std::shared_ptr<AVPacket> pkt(av_packet_alloc(), [](AVPacket* p) {
        av_packet_free(&p);
    });
    while (true) {
        file.read((char*)pcm.get(), read_size);
        if (file.eof())
            break;

        const uint8_t* data[1];
        data[0] = pcm.get();

        error = swr_convert(swrContext.get(), frame->data, frame->nb_samples, data, frame->nb_samples);
        if (error <= 0)
            break;

        error = avcodec_send_frame(context.get(),frame);
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
            pkt->pts = 0;
            pkt->dts = 0;
            av_interleaved_write_frame(output_context.get(),pkt.get());

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
        av_interleaved_write_frame(output_context.get(), pkt.get());

    } while (true);

    av_write_trailer(output_context.get());
    file.close();

    avcodec_close(context.get());
    return 0;
}