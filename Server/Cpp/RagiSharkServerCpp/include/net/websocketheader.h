#pragma once
#include <stdint.h>
#include <vector>

namespace ragii::net
{
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

    inline void websocket_header_create(std::vector<uint8_t>& header, bool fin, OpCode opcode, int len)
    {
        if (len <= 125) {
            header.resize(2);
            header[0] = (((fin ? 1 : 0) << 7) | static_cast<uint8_t>(opcode));
            header[1] = static_cast<uint8_t>(len);
        }
        else if (len <= 0xffff) {
            header.resize(4);
            header[0] = (((fin ? 1 : 0) << 7) | static_cast<uint8_t>(opcode));
            header[1] = 126;
            header[2] = (len & 0xff00) >> 8;
            header[3] = len & 0x00ff;
        }
        else {
            header.resize(10);
            header[0] = (((fin ? 1 : 0) << 7) | static_cast<uint8_t>(opcode));
            header[1] = 127;
            header[2] = (len >> 56);
            header[3] = ((len >> 48) & 0xff);
            header[4] = ((len >> 40) & 0xff);
            header[5] = ((len >> 32) & 0xff);
            header[6] = ((len >> 24) & 0xff);
            header[7] = ((len >> 16) & 0xff);
            header[8] = ((len >> 8) & 0xff);
            header[9] = ((len >> 0) & 0xff);
        }
    }
}
