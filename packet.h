#pragma once

#include <cstdint>

namespace svs 
{
class Packet
{
public:
    virtual int64_t dts() const = 0;
    virtual int64_t pts() const = 0;
    virtual ~Packet() = default;
};
}