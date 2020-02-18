#pragma once
#include <string_view>

class Socket
{
public:
    enum class INADDR {
        Any
    };

    Socket();
    ~Socket();
    static bool initialize();
    static void uninitialize();
    static std::wstring_view getLastError();
    bool bind(INADDR inaddr, int port);
    bool listen(int backlog = DEFAULT_BACKLOG);
    bool accept();
    bool send(std::string_view text);
    void close();

    static const int DEFAULT_BACKLOG = 5;
private:
    static bool s_Initialized;
    int64_t m_ServerSocket;
    int64_t m_Socket;
};
