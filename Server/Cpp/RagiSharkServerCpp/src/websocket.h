#pragma once
#include <stdint.h>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <functional>

class Socket;
struct WebSocketHeader;

enum class OpCode
{
    Continuation = 0,
    Text = 1,
    Binary = 2,
    Close = 8,
    Ping = 9,
    Pong = 10,
};

class WebSocket final
{
public:
    WebSocket();
    ~WebSocket();
    void initialize();
    bool listen(int port);
    bool sendText(std::string text);

    struct Handlers
    {
        std::function<void(std::string_view)> received;
    };

    void setHandlers(const Handlers& handlers) {
        m_Handlers = handlers;
    }
private:
    static std::vector<uint8_t> createHeader(bool fin, OpCode opcode, int len);
    std::unique_ptr<Socket> m_Socket;
    Handlers m_Handlers;

    void onMessageReceived(std::string_view msg);
    void onGetRequestReceived(std::string_view msg);
    void onDataFrameReceived(std::string_view msg);
    void onTextFrameReceived(const WebSocketHeader& header, std::string_view msg);
    void onCloseReceived();
};
