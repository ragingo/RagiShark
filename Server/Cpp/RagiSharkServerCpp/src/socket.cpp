#include "socket.h"

#include <cstdint>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32")

namespace {
    uint64_t convertINADDR(Socket::INADDR inaddr)
    {
        switch (inaddr)
        {
        case Socket::INADDR::Any:
            return INADDR_ANY;

        default:
            return INADDR_ANY;
        }
    }
}

bool Socket::s_Initialized = false;

Socket::Socket() :
    m_ServerSocket(INVALID_SOCKET),
    m_Socket(INVALID_SOCKET)
{
}

Socket::~Socket()
{
    close();
}

bool Socket::initialize()
{
    WSADATA data;
    int ret = WSAStartup(MAKEWORD(2, 0), &data);
    s_Initialized = ret == 0;
    return ret == 0;
}

void Socket::uninitialize()
{
    WSACleanup();
}

std::wstring_view Socket::getLastError()
{
    int err = WSAGetLastError();
    const int buf_size = 512;
    wchar_t buf[buf_size] = {0};
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, buf_size, nullptr);
    return std::wstring_view(buf);
}

bool Socket::bind(INADDR inaddr, int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    m_ServerSocket = sock;

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = convertINADDR(inaddr);

    int ret = ::bind(m_ServerSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (ret != 0) {
        return false;
    }

    return true;
}

bool Socket::listen(int backlog)
{
    int ret = ::listen(m_ServerSocket, backlog);
    return ret == 0;
}

bool Socket::accept()
{
    sockaddr_in addr = {};
    int len = sizeof(sockaddr_in);
    SOCKET sock = ::accept(m_ServerSocket, reinterpret_cast<sockaddr*>(&addr), &len);
    if (sock == INVALID_SOCKET) {
        return false;
    }
    m_Socket = sock;
    return true;
}

bool Socket::send(std::string_view text)
{
    int ret = ::send(m_Socket, text.data(), text.length(), 0);
    return ret > 0;
}

void Socket::close()
{
    if (m_Socket == INVALID_SOCKET) {
        return;
    }
    ::closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
}
