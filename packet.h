#pragma once

#include <cstdint>

namespace avprobe 
{
class Packet
{
public:
    virtual int64_t dts() const = 0;
    virtual int64_t pts() const = 0;
    virtual int streamIndex() const;

    virtual ~Packet() = default;
};
}