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
    int status {-1};
    Packet *packet {nullptr};
};

class InputStream 
{
public:
    InputStream(const AVStream *stream, AVCodecContext *codecCtx);
    ~InputStream();

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