#pragma once

#include <string>
#include <cstdint>
#include <tuple>

extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "packet.h"

namespace avprobe 
{
class Prober
{
public:
    Prober() = default;
    ~Prober();

    struct ReadResult
    {
        int status;
        Packet *packet;
    };

    bool open(const char *path);
    ReadResult readNextPacket();

private:
    bool findStreams();
    bool openCodec();

    std::string mUrl;

    int mVideoStream {-1};
    int mAudioStream {-1};

    AVFormatContext *mFormatCtx{nullptr};

    AVCodecParameters *mCodecParameters{nullptr};
    AVCodecParameters *mAudioCodecParameters{nullptr};

    AVCodecContext *mCodecCtx{nullptr};
    // AVCodecContext *mAudioCodecCtx{nullptr};

    AVCodec *mCodec{nullptr};
    AVCodec *mAudioCodec{nullptr};
};
}