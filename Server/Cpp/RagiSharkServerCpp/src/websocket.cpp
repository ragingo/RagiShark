#include "websocket.h"
#include "websocketheader.h"
#include "socket.h"
#include "util/util.h"
#include <cassert>
#include <iostream>
#include <regex>


namespace {
    constexpr std::string_view WS_KEY_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string decodeText(std::string_view msg)
    {
        const int MASKING_KEY_OFFSET = 2;
        const int MASKING_KEY_LENGTH = 4;
        auto maskingKey = msg.substr(MASKING_KEY_OFFSET, MASKING_KEY_LENGTH);

        const int DATA_OFFSET = MASKING_KEY_OFFSET + MASKING_KEY_LENGTH;
        auto data = msg.substr(DATA_OFFSET);

        std::string result;
        result.resize(data.size());

        for (int i = 0; i < data.size(); i++) {
            result[i] = data[i] ^ maskingKey[i % 4];
        }

        return result;
    }
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

    std::vector<uint8_t> header(createHeader(true, OpCode::Text, text.size()));
    send_data.append(header.begin(), header.end());
    send_data.append(text);

    return m_Socket->send(send_data);
}

std::vector<uint8_t> WebSocket::createHeader(bool fin, OpCode opcode, int len)
{
    std::vector<uint8_t> ret;
    websocket_header_create(ret, fin, opcode, len);
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
    if (m.empty() || m.size() < 2) {
        return;
    }
    const auto result = m[1];
    if (!result.matched) {
        return;
    }

    auto key = std::string_view(result.first);
    int pos1 = std::min(key.find_first_not_of(" \r\n"), key.size());
    int pos2 = std::min(key.find_first_of("\r\n"), key.size());
    key.remove_prefix(pos1);
    key.remove_suffix(key.size() - pos2);
    std::cout << "key: " << key << std::endl;

    auto new_key = std::string(key).append(WS_KEY_GUID);
    std::cout << "new key: " << new_key << std::endl;

    auto hash = createSha1Hash(new_key);
    std::cout << "sha1 hash: " << hash << std::endl;

    std::string header;
    header += "HTTP/1.1 101 Switching Protocols\r\n";
    header += "Connection: Upgrade\r\n";
    header += "Upgrade: websocket\r\n";
    header += "Sec-WebSocket-Accept: " + hash + "\r\n";

    m_Socket->send(header);
}

void WebSocket::onDataFrameReceived(std::string_view msg)
{
    if (msg.length() < 2) {
        return;
    }

    auto bytes = reinterpret_cast<const uint8_t*>(msg.data());
    WebSocketHeader header = {};
    websocket_header_parse(header, bytes);

    switch (static_cast<OpCode>(header.opcode)) {
        case OpCode::Text:
            onTextFrameReceived(header, msg);
            break;
        case OpCode::Close:
            onCloseReceived();
            break;
    }
}

void WebSocket::onTextFrameReceived(const WebSocketHeader& header, std::string_view msg)
{
    if (header.payload_length <= 125) {
        if (header.mask) {
            auto text = decodeText(msg);
            if (m_Handlers.received) {
                m_Handlers.received(text);
            }
        }
    }
    else if (header.payload_length == 126) {
    }
    else if (header.payload_length == 127) {
    }
}

void WebSocket::onCloseReceived()
{
}