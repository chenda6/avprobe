#include <cstdio>
#include <iostream>
#include <string>
#include "prober.h"
#include "ffpacket.h"

using namespace avprobe;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void logging(const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Prober::~Prober()
{
    avformat_close_input(&mFormatCtx);
    avcodec_free_context(&mCodecCtx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::open(const char *path)
{
    if(avformat_open_input(&mFormatCtx, path, nullptr, nullptr) != 0) 
    {
        std::cerr << "could not open file";
        return false;
    }

    logging("format %s, duration %lld us, bit_rate %lld", 
        mFormatCtx->iformat->name, mFormatCtx->duration, mFormatCtx->bit_rate);

    if(avformat_find_stream_info(mFormatCtx, nullptr) < 0) 
    {
        std::cerr << "could not get stream info";
        return false;
    }
    av_dump_format(mFormatCtx, 0, path, 0);  // [4]

    if(!findStreams())
    {
        std::cerr << "could not find streams in input";
        return false;
    }

    if(!openCodec())
    {
        std::cerr << "could not open codecs on input";
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static int decode_packet2(
    const AVStream *stream,
    AVCodecContext *codecContext, 
    AVPacket *pkt, 
    AVFrame *frame)
{
    // printf("stream_index %d\n", pkt->stream_index);
    // printf("pts %d\n", pkt->pts);
    // printf("dts %d\n", pkt->dts);
    //printf("dts_time", pkt->dts, &st->time_base);
    //printf("duration %d\n", pkt->duration);
    //printf("duration_time", pkt->duration, &st->time_base);
    //print_val("size",             pkt->size, unit_byte_str);

    auto ret = avcodec_send_packet(codecContext, pkt);
    if(ret < 0)
    {
        printf("Error sending packet for decoding.");
        return -1;
    }

    while(ret >= 0)
    {
        ret = avcodec_receive_frame(codecContext, frame);
        // printf("receive frame: %d %d\n", frame->width, frame->height);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
        {
            break;
        }
        else if(ret < 0)
        {
            printf("Error while decoding packet.");
            return -1;
        }

        logging(
            "Frame %d (type=%c, size=%d bytes, format=%d) pts %d key_frame %d [DTS %d]",
            codecContext->frame_number,
            av_get_picture_type_char(frame->pict_type),
            frame->pkt_size,
            frame->format,
            frame->pts,
            frame->key_frame,
            frame->coded_picture_number);


        //const auto fps = av_q2d(stream->r_frame_rate);
        //render(fps, codecContext, renderer, texture, frame, pict);
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::readPackets()
{
    AVPacket *packet(av_packet_alloc());
    if(!packet)
    {
        std::cerr << "could not allocate memory for packet";
        return false;
    }

    AVFrame *frame(av_frame_alloc());
    if(!frame)
    {
        std::cerr << "could not allocate memory for frame";
        return -1;
    }

    int response(0);
    int numberPacketsProcessed(0);
    while(av_read_frame(mFormatCtx, packet) >= 0)
    {
        if(packet->stream_index == mVideoStream) 
        {
            response = decode_packet2(
                mFormatCtx->streams[mVideoStream],
                mCodecCtx, 
                packet, 
                frame);

            if(response < 0)
            {
                break;
            }
        }

        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::findStreams()
{
    for(unsigned i = 0; i < mFormatCtx->nb_streams; i++)
    {
        auto codecParam = mFormatCtx->streams[i]->codecpar;
        logging("AVStream->time_base before open coded %d/%d", mFormatCtx->streams[i]->time_base.num, mFormatCtx->streams[i]->time_base.den);
        logging("AVStream->r_frame_rate before open coded %d/%d", mFormatCtx->streams[i]->r_frame_rate.num, mFormatCtx->streams[i]->r_frame_rate.den);

        if(codecParam->codec_type == AVMEDIA_TYPE_VIDEO) 
        {
            mVideoStream = i;
        } 
        else if(codecParam->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            logging("Audio Codec: %d channels, sample rate %d", codecParam->channels, codecParam->sample_rate);
            mAudioStream = i;
        }
    }

    return mAudioStream != -1 || mVideoStream != -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::openCodec()
{
    auto codec = avcodec_find_decoder(mFormatCtx->streams[mVideoStream]->codecpar->codec_id);
    if(!codec)
    {
        std::cerr << "unsupported codec";
        return false;
    }

    /**
    * Note that we must not use the AVCodecContext from the video stream
    * directly! So we have to use avcodec_copy_context() to copy the
    * context to a new location (after allocating memory for it, of
    * course).
    */

    // Copy context
    mCodecCtx = avcodec_alloc_context3(codec);
    if(!mCodecCtx)
    {
        std::cerr << "could not allocate memory for Codec Context";
        return false;
    }

    auto ret = avcodec_parameters_to_context(mCodecCtx, mFormatCtx->streams[mVideoStream]->codecpar);
    if(ret != 0)
    {
        printf("Could not copy codec context.\n");
        return false;
    }

    // Open codec
    if(avcodec_open2(mCodecCtx, codec, nullptr) < 0)
    {
        std::cerr << "Failed to open Codec";
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Packet *Prober::readNextPacket()
{
    AVPacket packet;
    if(av_read_frame(mFormatCtx, &packet) >= 0)
    {
        FFPacket *pkt(new FFPacket(packet));
        return pkt;

    }
    return nullptr;
}