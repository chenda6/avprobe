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
            auto packet = prober.readNextPacket();
            if(packet)
            {
                //printf("stream_index %d\n", packet->stream_index);
                //printf("pts %d\n", packet->pts);
                printf("pkt: %d dts %d\n", count++, packet->dts());
                printf("pkt: %d stream %d\n", count++, packet->streamIndex());
                //printf("duration %d\n", packet->duration);
            }
            else
            {
                break;
            }
        }
        #endif
    }

}