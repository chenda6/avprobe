#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "packet.h"

namespace avprobe 
{
struct ReadResult
{
    int status;
    Packet *packet;
};

class InputStream {
public:
    InputStream(const AVStream *stream, AVCodecContext *codecCtx)
    : mStream(stream), mCodecCtx(codecCtx)
    {
    }

    ~InputStream() 
    {
        avcodec_free_context(&this->mCodecCtx);
    }

private:
    const AVStream *mStream;
    AVCodecContext *mCodecCtx;
};

class Prober
{
public:
    Prober() = default;
    ~Prober();

    bool open(const char *path);
    ReadResult readNextPacket();

private:
    bool openCodecs();

    std::string mUrl;

    AVFormatContext *mFormatCtx{nullptr};

    std::vector<std::shared_ptr<InputStream>> mStreams;
};
}