#include <iostream>
#include "prober.h"

static void probeFile(const char *fname)
{
    avprobe::Prober prober;
    if(prober.open(fname))
    {
        auto count(0);
        while (true)
        {
            auto result = prober.readNextPacket();
            if (result.status == 0)
            {
                auto packet(result.packet);
                printf("pkt: %d stream: %d dts: %ld\n", count++, packet->streamIndex(), packet->dts());
                delete packet;
            }
            else
            {
                printf("status: %d\n", result.status);
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    const char *fname = argv[1];
    while(true)
    {

        std::cout << "Probing: " << fname << std::endl;
        probeFile(fname);
    }
}