#pragma once
#include <string_view>
#include <string>
#include <vector>

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
    bool send(std::vector<uint8_t> data);
    int receive(std::string& text, int size = DEFAULT_RECEIVE_BUFFER_SIZE);
    void close();

    static const int DEFAULT_BACKLOG = 5;
    static const int DEFAULT_RECEIVE_BUFFER_SIZE = 1024;
private:
    static bool s_Initialized;
    int64_t m_ServerSocket;
    int64_t m_Socket;
};
