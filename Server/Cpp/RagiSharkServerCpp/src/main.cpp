#include <iostream>
#include <chrono>
#include <thread>
#include "socket.h"

int main()
{
    std::cout << "launch" << std::endl;

    if (!Socket::initialize()) {
        std::wcout << L"initialize error: " << Socket::getLastError() << std::endl;
        return EXIT_FAILURE;
    }

    Socket sock;
    if (!sock.bind(Socket::INADDR::Any, 8080)){
        std::wcout << L"bind error: " << Socket::getLastError() << std::endl;
        return EXIT_FAILURE;
    }
    if (!sock.listen()) {
        std::wcout << L"listen error: " << Socket::getLastError() << std::endl;
        return EXIT_FAILURE;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (!sock.accept()) {
            std::wcout << L"accept error: " << Socket::getLastError() << std::endl;
            break;
        }

        if (!sock.send("hello!")) {
            std::cout << "send failed." << std::endl;
        }

        sock.close();
    }

    Socket::uninitialize();

    return EXIT_SUCCESS;
}
