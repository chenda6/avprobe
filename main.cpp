#include "prober.h"

int main111(int argc, char *argv[])
{
    avprobe::Prober prober;
    if(prober.open(argv[1]))
    {
        auto count(0);
        while(true)
        {
            auto result = prober.readNextPacket();
            if(result.status == 0)
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