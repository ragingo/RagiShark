#include "websocket.h"
#include "socket.h"
#include <cassert>
#include <codecvt>
#include <iostream>

WebSocket::WebSocket()
{
}

WebSocket::~WebSocket()
{
}

void WebSocket::initialize()
{
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        m_Socket = std::make_unique<Socket>();
    }
}

bool WebSocket::listen(int port)
{
    assert(m_Socket);
    if (!m_Socket->bind(Socket::INADDR::Any, port)) {
        return false;
    }
    if (!m_Socket->listen()) {
        return false;
    }
    if (!m_Socket->accept()) {
        return false;
    }
    while (true)
    {
        std::string s;
        int len = m_Socket->receive(s);
        if (len == 0) {
            continue;
        }
        onMessageReceived(s);
    }
    return true;
}

bool WebSocket::sendText(std::string text)
{
    assert(m_Socket);
    std::string send_data;
    // TODO:
    const auto&& header = std::move(createHeader(true, OpCode::Text, 1));
    return m_Socket->send(send_data);
}

std::vector<uint8_t> WebSocket::createHeader(bool fin, OpCode opcode, int len)
{
    std::vector<uint8_t> ret;
    return ret;
}

void WebSocket::onMessageReceived(std::string_view msg)
{
    if (msg._Starts_with("GET /")) { // 良くないけどあったから使おう・・・
        onGetRequestReceived(msg);
    }
    else {
        onDataFrameReceived(msg);
    }
}

void WebSocket::onGetRequestReceived(std::string_view msg)
{
}

void WebSocket::onDataFrameReceived(std::string_view msg)
{
}
