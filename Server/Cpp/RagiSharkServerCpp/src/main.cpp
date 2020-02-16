#include <WinSock2.h>
#include <WS2tcpip.h>
// #pragma comment(lib,"ws2_32.lib")

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 0), &wsaData);

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3001);
        addr.sin_addr.s_addr = 0;

        bind(listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        listen(listen_sock, 5);
    }

    sockaddr_in client;
    int len;
    SOCKET sock;

    while (1)
    {
        len = sizeof(sockaddr_in);
        sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&client), &len);
        send(sock, "HELLO", 5, 0);

        closesocket(sock);
    }

    WSACleanup();

    return 0;
}