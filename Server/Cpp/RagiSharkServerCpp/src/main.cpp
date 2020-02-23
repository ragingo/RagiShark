#include <iostream>
#include <chrono>
#include <thread>
#include "socket.h"
#include "websocket.h"

int main()
{
    std::cout << "launch" << std::endl;

    if (!Socket::initialize()) {
        std::cout << "initialize error: " << Socket::getLastError() << std::endl;
        return EXIT_FAILURE;
    }

    WebSocket ws;
    ws.initialize();
    ws.listen(8080);

    Socket::uninitialize();

    return EXIT_SUCCESS;
}
