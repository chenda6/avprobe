#include <iostream>
#include <string>
#include "ffpacket.h"

using avprobe::FFPacket;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FFPacket::FFPacket(const AVPacket &packet)
// TODO: see https://stackoverflow.com/questions/12929330/create-a-copy-of-an-avpacket-structure
: mPacket(packet)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FFPacket::~FFPacket()
{
    av_packet_unref(&mPacket);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int64_t FFPacket::dts() const
{
    return mPacket.dts;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int64_t FFPacket::pts() const
{
    return mPacket.pts;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int FFPacket::streamIndex() const
{
    return mPacket.stream_index;
}