#include <cstdio>
#include <iostream>
#include <string>
#include "prober.h"
#include "ffpacket.h"

using avprobe::Prober;
using avprobe::ReadResult;
using avprobe::InputStream;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
InputStream::InputStream(const AVStream *stream, AVCodecContext *codecCtx)
: mStream(stream), mCodecCtx(codecCtx)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
InputStream::~InputStream() 
{
    avcodec_free_context(&this->mCodecCtx);
}

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::open(const char *path)
{
    if(avformat_open_input(&mFormatCtx, path, nullptr, nullptr) != 0) 
    {
        std::cerr << "prober could not open file";
        return false;
    }

    logging("format %s, duration %lld us, bit_rate %lld", 
        mFormatCtx->iformat->name, mFormatCtx->duration, mFormatCtx->bit_rate);

    if(avformat_find_stream_info(mFormatCtx, nullptr) < 0) 
    {
        std::cerr << "prober could not get stream info";
        return false;
    }

    // TODO:
    av_dump_format(mFormatCtx, 0, path, 0);

    if(!openCodecs())
    {
        std::cerr << "prober could not open codecs on input";
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::openCodecs()
{
    for(unsigned i = 0; i < mFormatCtx->nb_streams; i++)
    {
        const auto stream = mFormatCtx->streams[i];

        logging("AVStream->time_base before open coded %d/%d", stream->time_base.num, stream->time_base.den);
        logging("AVStream->r_frame_rate before open coded %d/%d", stream->r_frame_rate.num, stream->r_frame_rate.den);

        if(stream->codecpar->codec_id == AV_CODEC_ID_PROBE) 
        {
            std::cerr << "Failed to probe codec for input stream " << stream->index << std::endl;
            mStreams.emplace_back(std::make_shared<InputStream>(nullptr, nullptr));
            continue;
        }

        auto codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if(!codec)
        {
            std::cerr 
                << "Unsupported codec with id " << stream->codecpar->codec_id
                << " for input stream " << stream->index << std::endl;
            return false;
        }

        auto codecCtx = avcodec_alloc_context3(codec);
        if(!codecCtx)
        {
            std::cerr << "could not allocate memory for Codec Context";
            return false;
        }

        if(avcodec_parameters_to_context(codecCtx, stream->codecpar) < 0)
        {
            printf("Could not copy codec context.\n");
            return false;
        }

        if(avcodec_open2(codecCtx, codec, nullptr) < 0)
        {
            std::cerr << "Failed to open codec";
            return false;
        }

        mStreams.emplace_back(std::make_shared<InputStream>(stream, codecCtx));
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ReadResult Prober::readNextPacket()
{
    AVPacket packet;
    auto res = av_read_frame(mFormatCtx, &packet);
    if(res >= 0)
    {
        FFPacket *pkt(new FFPacket(packet));
        return ReadResult{
            .status = res,
            .packet = pkt
        };
    }

    return ReadResult{
        .status = res,
        .packet = nullptr
    };
}