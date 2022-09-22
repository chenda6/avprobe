#include <cstdio>
#include <iostream>
#include <string>
#include "prober.h"
#include "ffpacket.h"

using avprobe::Prober;
using avprobe::ReadResult;
using avprobe::InputStream;

static bool gErrorOccurred = false;

//------------------------------------------------------------------------------
// Used to trap error messages/conditions when certain errors, e.g., deocding,
// do not get propagated back up the call stack. 
//------------------------------------------------------------------------------
void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if(level <= AV_LOG_ERROR)
    {
        vprintf(fmt, vargs);
        gErrorOccurred = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
InputStream::InputStream(
        const AVStream *stream, 
        AVCodecContext *codecCtx, 
        AVCodecParserContext *parserCtx)
: mStream(stream), mCodecCtx(codecCtx), mParserCxt(parserCtx)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
InputStream::~InputStream() 
{
    avcodec_free_context(&this->mCodecCtx);
    av_parser_close(mParserCxt);
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
Prober::Prober()
{
    av_log_set_callback(my_log_callback);
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
void Prober::setLogLevel(int level)
{
    mLogLevel = level;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Prober::open(const char *path)
{
    av_log_set_level(mLogLevel); 

    if(avformat_open_input(&mFormatCtx, path, nullptr, nullptr) != 0) 
    {
        std::cerr << "prober could not open file";
        return false;
    }

    //logging("format %s, duration %lld us, bit_rate %lld", mFormatCtx->iformat->name, mFormatCtx->duration, mFormatCtx->bit_rate);

    if(avformat_find_stream_info(mFormatCtx, nullptr) < 0) 
    {
        std::cerr << "prober could not get stream info";
        return false;
    }

    // TODO:
    //av_dump_format(mFormatCtx, 0, path, 0);

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

        // logging("AVStream->time_base before open coded %d/%d", stream->time_base.num, stream->time_base.den);
        // logging("AVStream->r_frame_rate before open coded %d/%d", stream->r_frame_rate.num, stream->r_frame_rate.den);

        if(stream->codecpar->codec_id == AV_CODEC_ID_PROBE) 
        {
            std::cerr << "Failed to probe codec for input stream " << stream->index << std::endl;
            mStreams.emplace_back(std::make_shared<InputStream>(nullptr, nullptr, nullptr));
            continue;
        }

        auto codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if(!codec)
        {
            std::cerr 
                << "Unsupported codec with id " << stream->codecpar->codec_id
                << " for input stream " << stream->index << std::endl;
            mStreams.emplace_back(std::make_shared<InputStream>(nullptr, nullptr, nullptr));
            continue;
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

        auto parser = av_parser_init(stream->codecpar->codec_id);
        if(!parser) 
        {
            std::cerr << "parser not found\n";
            mStreams.emplace_back(std::make_shared<InputStream>(nullptr, nullptr, nullptr));
            continue;
        }
        parser->flags = PARSER_FLAG_COMPLETE_FRAMES;

        mStreams.emplace_back(std::make_shared<InputStream>(stream, codecCtx, parser));
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Prober::parse(AVPacket &pkt) 
{
    const auto stream = mFormatCtx->streams[pkt.stream_index];

    if(stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO && stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
    {
        return 0;
    }

    const auto avctx = mStreams[pkt.stream_index]->getAVCodecContext();
    if(!avctx)
    {
        return 0;
    }

    auto parser = mStreams[pkt.stream_index]->mParserCxt;
    if(!parser) 
    {
        std::cerr << "parser not found\n";
        return 0;
    }

    AVPacket temp;
    auto res = av_parser_parse2(
                parser, avctx,
                &temp.data, &temp.size, pkt.data, pkt.size,
                pkt.pts, pkt.dts, pkt.pos);

    return gErrorOccurred ? AVERROR_INVALIDDATA : res;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Prober::decode(AVPacket &packet) 
{
    int res(0);
    auto stream = mStreams[packet.stream_index];
    if(stream)
    {
        auto codecCtx = stream.get()->getAVCodecContext();
        auto res = avcodec_send_packet(codecCtx, &packet);
        if (res == AVERROR(EAGAIN))
        {
            res = 0;
        }

        AVFrame *frame = av_frame_alloc();
        while (res >= 0)
        {
            res = avcodec_receive_frame(codecCtx, frame);
            if(res >= 0)
            {
                const auto codecId = stream->getAVStream()->codecpar->codec_id;

                if(avcodec_get_type(codecId) == AVMEDIA_TYPE_VIDEO)
                {
                    printf("decoded frame: %s %d %d\n", avcodec_get_name(codecId), frame->width, frame->height);
                }
                else if(avcodec_get_type(codecId) == AVMEDIA_TYPE_AUDIO)
                {
                    printf("decoded frame: %s %u\n", avcodec_get_name(codecId), frame->channels);
                }
            }
            if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
            {
                res = 0;
                // std::cout << "EAGAIN | EOF\n";
                break;
            }
            else if (res < 0)
            {
                std::cout << "ERROR\n";
            }
        }
        av_free(frame);
    }

    return gErrorOccurred ? AVERROR_INVALIDDATA : res;
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

        //res = decode(packet);
        res = parse(packet);

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