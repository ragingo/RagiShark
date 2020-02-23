#pragma once
#include <stdint.h>

struct WebSocketHeader
{
    bool fin;
    uint8_t rcv1;
    uint8_t rcv2;
    uint8_t rcv3;
    uint8_t opcode;
    bool mask;
    uint8_t payload_length;
};

inline void websocket_header_parse(WebSocketHeader& header, const uint8_t* data)
{
    if (!data) {
        return;
    }

    uint8_t b0 = *(data + 0);
    uint8_t b1 = *(data + 1);
    header.fin = (b0 & 0x80) == 0x80;
    header.rcv1 = (b0 & 0x40) == 0x40 ? 1 : 0;
    header.rcv2 = (b0 & 0x20) == 0x20 ? 1 : 0;
    header.rcv3 = (b0 & 0x10) == 0x10 ? 1 : 0;
    header.opcode = b0 & 0x0f;
    header.mask = (b1 & 0x80) == 0x80;
    header.payload_length = b1 & 0x7f;
}
