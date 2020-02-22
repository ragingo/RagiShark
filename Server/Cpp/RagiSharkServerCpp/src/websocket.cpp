#include "websocket.h"
#include "socket.h"
#include <cassert>
#include <codecvt>
#include <iostream>
#include <regex>
#define NOMINMAX
#include <Windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32")

namespace {
    constexpr std::string_view WS_KEY_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string createSha1Hash(std::string text)
    {
        HCRYPTPROV prov = 0;
        HCRYPTHASH hash = 0;

        auto release = [&]()-> void {
            if (hash) {
                CryptDestroyHash(hash);
            }
            if (prov) {
                CryptReleaseContext(prov, 0);
            }
        };

        int ret = CryptAcquireContext(&prov, 0, 0, PROV_RSA_FULL, 0);
        if (!ret) {
            release();
            return "";
        }

        ret = CryptCreateHash(prov, CALG_SHA1, 0, 0, &hash);
        if (!ret) {
            release();
            return "";
        }

        ret = CryptHashData(hash, reinterpret_cast<const uint8_t*>(text.c_str()), text.size(), 0);
        if (!ret) {
            release();
            return "";
        }

        DWORD len = 0;
        ret = CryptGetHashParam(hash, HP_HASHVAL, nullptr, &len, 0);
        if (!ret) {
            release();
            return "";
        }

        std::vector<uint8_t> hash_value;
        hash_value.resize(len);
        ret = CryptGetHashParam(hash, HP_HASHVAL, hash_value.data(), &len, 0);
        if (!ret) {
            release();
            return "";
        }

        DWORD len2 = 1024;
        char buf2[1024] = {0};
        ret = CryptBinaryToString(hash_value.data(), hash_value.size(), CRYPT_STRING_BASE64, buf2, &len2);
        if (!ret) {
            release();
            return "";
        }

        release();
        return std::string(buf2);
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
}

void WebSocket::onDataFrameReceived(std::string_view msg)
{
}
