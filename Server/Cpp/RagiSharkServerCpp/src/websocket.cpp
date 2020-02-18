#include "websocket.h"
#include "socket.h"
#include <cassert>

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
    return true;
}

bool WebSocket::accept()
{
    assert(m_Socket);
    return m_Socket->accept();
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
