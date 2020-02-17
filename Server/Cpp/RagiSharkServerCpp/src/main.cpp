#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

int main()
{
    std::cout << "launch" << std::endl;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = 0;

        std::cout << "bind" << std::endl;
        bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        std::cout << "listen" << std::endl;
        listen(listen_sock, 5);
    }

    sockaddr_in client;
    int len;
    SOCKET sock;

    while (true)
    {
        len = sizeof(sockaddr_in);
        sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&client), &len);
        std::cout << "accepted" << std::endl;
        std::cout << "sending" << std::endl;
        send(sock, "HELLO", 5, 0);
        std::cout << "sent" << std::endl;

        closesocket(sock);
    }

    WSACleanup();

    return 0;
}