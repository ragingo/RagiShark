﻿#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <deque>
#include "net/socket.h"
#include "net/websocket.h"
#include "util/util.h"

#include "diagnostics/process.h"

using namespace ragii::net;
using namespace ragii::util;
using namespace ragii::diagnostics;

std::mutex mtx;

int main()
{
    std::deque<std::string> sendQueue;

    std::string nic = "any";
    // Windows版は any 非対応なので、特定のインターフェースを指定する必要がある
    // https://tshark.dev/capture/sources/
    #if defined(RAGII_WINDOWS)
    nic = "6"; // 動作環境依存
    #endif

    ProcessStartInfo psi;
    psi.Name = "tshark";
    psi.Args = { "-i", nic, "-T ek", "-e frame.number", "-e ip.proto", "-e ip.src", "-e ip.dst", "-e tcp.srcport", "-e tcp.dstport" };
    psi.RedirectStdOut = true;

    std::cout << ragii::util::join(" ", psi.Args) << std::endl;

    auto proc = Process::Start(psi);
    proc->setStdOutReceivedHandler([&sendQueue] (const std::string& s) -> void {
        std::lock_guard<std::mutex> lock(mtx);
        sendQueue.push_back(s);
    });


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

    std::thread t([&ws, &sendQueue]()-> void {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (sendQueue.empty()) {
                    continue;
                }
                auto item = sendQueue.front();
                ws.sendText(std::move(item));
                sendQueue.pop_front();
            }
        }
    });

    ws.listen(8080);

    Socket::uninitialize();

    return EXIT_SUCCESS;
}
