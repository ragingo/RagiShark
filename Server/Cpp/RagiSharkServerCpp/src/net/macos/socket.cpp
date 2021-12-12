#include "common/common.h"

#ifdef RAGII_MAC

#include "net/socket.h"
#include <array>
#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace
{
    using namespace ragii::net;

    constexpr int INVALID_SOCKET = -1;

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

namespace ragii::net
{
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
        return true;
    }

    void Socket::uninitialize()
    {
    }

    std::string Socket::getLastError()
    {
        return std::string(strerror(errno));
    }

    bool Socket::bind(INADDR inaddr, int port)
    {
        auto sock = socket(AF_INET, SOCK_STREAM, 0);
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
        socklen_t len = sizeof(sockaddr_in);
        auto sock = ::accept(m_ServerSocket, reinterpret_cast<sockaddr*>(&addr), &len);
        if (sock == INVALID_SOCKET) {
            return false;
        }
        m_Socket = sock;
        return true;
    }

    bool Socket::send(std::string_view text)
    {
        if (m_Socket == INVALID_SOCKET) {
            return false;
        }
        int ret = ::send(m_Socket, text.data(), text.length(), 0);
        return ret > 0;
    }

    bool Socket::send(std::vector<uint8_t> data)
    {
        if (m_Socket == INVALID_SOCKET) {
            return false;
        }
        int ret = ::send(m_Socket, reinterpret_cast<char*>(data.data()), data.size(), 0);
        return ret > 0;
    }

    int Socket::receive(std::string& text, int size)
    {
        if (m_Socket == INVALID_SOCKET) {
            return 0;
        }

        std::vector<char> buf;
        buf.resize(size);
        int total_len = ::recv(m_Socket, buf.data(), size, 0);
        text.assign(buf.data());
        return total_len;
    }

    void Socket::close()
    {
        if (m_Socket == INVALID_SOCKET) {
            return;
        }
        ::shutdown(m_Socket, SHUT_RDWR);
        ::close(m_Socket);
        m_Socket = INVALID_SOCKET;
    }
}

#endif
