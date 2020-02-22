#include "websocket.h"
#include "socket.h"
#include <cassert>
#include <codecvt>
#include <iostream>
#include <regex>

namespace {
    constexpr std::string_view WS_KEY_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
}

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
    const auto pattern = std::regex("Sec-WebSocket-Key: (.*)");
    std::cmatch m;
    if (!std::regex_search(msg.data(), m, pattern)) {
        return;
    }
    if (m.empty()) {
        return;
    }
    const auto result = m[0];
    if (!result.matched) {
        return;
    }

    auto key = std::string_view(result.first);
    int pos1 = std::min(key.find_first_not_of(" \r\n"), key.size());
    int pos2 = std::min(key.find_first_of("\r\n"), key.size());
    key.remove_prefix(pos1);
    key.remove_suffix(key.size() - pos2);
    // std::cout << key << std::endl;

    auto new_key = std::string(key).append(WS_KEY_GUID);
    // std::cout << new_key << std::endl;
}

void WebSocket::onDataFrameReceived(std::string_view msg)
{
}
