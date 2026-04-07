#ifndef PACKET_H
#include <stdint.h>

struct Packet{
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[32];
    uint8_t crc;
};

#endif