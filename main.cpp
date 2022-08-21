#include "prober.h"

int main(int argc, char *argv[])
{
    avprobe::Prober prober;
    if(prober.open(argv[1]))
    {
        #if 0
        prober.readPackets();
        #else 
        auto count(0);
        while(true)
        {
            auto tuple = prober.readNextPacket();
            if(std::get<0>(tuple) == 0)
            {
                auto packet(std::get<1>(tuple));
                printf("pkt: %d stream: %d dts: %ld\n", count++, packet->streamIndex(), packet->dts());
                delete packet;
            }
            else
            {
                break;
            }
        }
        #endif
    }

}