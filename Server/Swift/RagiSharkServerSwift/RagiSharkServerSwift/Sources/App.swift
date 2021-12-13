//
//  App.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class App {
    private let tshark = TShark()

    func run(arguments: [String: String]) {
        guard let listenPort = Int(arguments["-p"] ?? "8888") else {
            exit(EXIT_FAILURE)
        }
        guard let server = WebSocketServer(port: listenPort) else {
            exit(EXIT_FAILURE)
        }
        server.delegate = self

        if !server.start() {
            exit(EXIT_FAILURE)
        }

        RunLoop.main.run()
        exit(EXIT_SUCCESS)
    }
}

extension App: WebSocketServerDelegate {
    func received(_ webSocketServer: WebSocketServer, text: String) {
        switch text {
        case "get if list":
            onGetIFList(webSocketServer)
        default:
            break
        }
    }

    private func onGetIFList(_ webSocketServer: WebSocketServer) {
        print("[App:onGetIFList] interfaces")

        tshark.interfaces {
            let interfaces = $0.enumerated().map { i, interface in
                Interface(no: i + 1, name: interface)
            }

            let response = GetInterfaceListResponse(data: interfaces)
            let data: Data
            do {
                data = try JSONEncoder().encode(response)
            } catch {
                print(error)
                return
            }

            _ = webSocketServer.send(data: data)
        }
    }
}
