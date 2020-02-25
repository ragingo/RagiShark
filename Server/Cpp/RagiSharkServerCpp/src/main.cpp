#include <iostream>
#include <chrono>
#include <thread>
#include "socket.h"
#include "websocket.h"

#include "util/util.h"

int main()
{
    // createProcess("tshark", { "-D" });
    std::cout << "launch" << std::endl;

    if (!Socket::initialize()) {
        std::cout << "initialize error: " << Socket::getLastError() << std::endl;
        return EXIT_FAILURE;
    }

    WebSocket ws;
    WebSocket::Handlers handlers;
    handlers.received = [&](std::string_view msg)->void {
        std::cout << "received: " << msg << std::endl;
    };
    ws.setHandlers(handlers);
    ws.initialize();
    ws.listen(8080);

    Socket::uninitialize();

    return EXIT_SUCCESS;
}
