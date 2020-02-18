#pragma once
#include <stdint.h>

#pragma pack(push, 1)
struct WebSocketHeader
{
    uint8_t fin : 1;
    uint8_t rcv1 : 1;
    uint8_t rcv1 : 1;
    uint8_t rcv1 : 1;
    uint8_t opcode : 4;
    uint8_t mask : 1;
    uint8_t payload_length : 7;
};
#pragma pack(pop)
