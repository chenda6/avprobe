#pragma once

#include <string>
#include <cstdint>

extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "packet.h"

namespace avprobe 
{
class FFPacket : public Packet
{
public:
    FFPacket(const AVPacket &packet);

    ~FFPacket();

    virtual int64_t dts() const override;
    virtual int64_t pts() const override;

private:
    AVPacket mPacket;
};
}