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
    InputStream(const AVStream *stream, AVCodecContext *codecCtx, AVCodecParserContext *parserCtx);
    ~InputStream();

    const AVStream *getAVStream() const
    {
        return mStream;
    }

    AVCodecContext *getAVCodecContext() 
    {
        return mCodecCtx;
    }

public:
    AVCodecParserContext *mParserCxt {nullptr};
private:
    const AVStream *mStream {nullptr};
    AVCodecContext *mCodecCtx {nullptr};
};

class Prober
{
public:
    Prober();
    ~Prober();

    void  setLogLevel(int);
    bool open(const char *path);
    ReadResult readNextPacket();

    int decode(AVPacket &packet);
    int parse(AVPacket &packet);

private:
    bool openCodecs();

    std::string mUrl;

    AVFormatContext *mFormatCtx{nullptr};

    std::vector<std::shared_ptr<InputStream>> mStreams;

    int mLogLevel {AV_LOG_ERROR};
};
}