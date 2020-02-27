#include <iostream>
#include <chrono>
#include <thread>
#include "net/socket.h"
#include "net/websocket.h"

#include "util/util.h"

int main()
{
    // std::vector<std::string_view> args = { "-D" };
    // createProcess("tshark", std::move(args));
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

    std::thread t([&ws]()-> void {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ws.sendText("aaa");
        }
    });

    ws.listen(8080);

    Socket::uninitialize();

    return EXIT_SUCCESS;
}
