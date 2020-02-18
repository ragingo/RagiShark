#pragma once
#include <stdint.h>
#include <string_view>
#include <memory>
#include <vector>

class Socket;

enum class OpCode
{
    Continuation = 0,
    Text = 1,
    Binary = 2,
    Close = 8,
    Ping = 9,
    Pong = 10,
};

class WebSocket
{
public:
    WebSocket();
    ~WebSocket();
    void initialize();
    bool listen(int port);
    bool accept();
    bool sendText(std::string text);

private:
    static std::vector<uint8_t> createHeader(bool fin, OpCode opcode, int len);
    std::unique_ptr<Socket> m_Socket;
};
