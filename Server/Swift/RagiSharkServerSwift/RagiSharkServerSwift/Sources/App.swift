//
//  App.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class App {
    private let tshark = TShark()
    private var server: WebSocketServer?

    func run() {
        guard let server = WebSocketServer(port: 9877) else {
            exit(EXIT_FAILURE)
        }
        self.server = server
        server.delegate = self

        if !server.start() {
            exit(EXIT_FAILURE)
        }

        RunLoop.main.run()
        exit(EXIT_SUCCESS)
    }
}

enum RagiSharkClientMessage: String {
    case getIFList = "get if list"
}

extension App: WebSocketServerDelegate {
    func received(_ webSocketServer: WebSocketServer, text: String) {
        switch RagiSharkClientMessage(rawValue: text) {
        case .getIFList:
            onGetIFList()
        default:
            break
        }
    }

    private func onGetIFList() {
        let interfaces = tshark.interfaces()
        print("[App:onGetIFList] interfaces")
        print(interfaces)

        // TODO: json encoder
        var sendText = ""
        sendText += " { "
        sendText += #" "type": "get_if_list_response", "#
        sendText += #" "data": [ "#
        interfaces.enumerated().forEach { i, interface in
            if i > 0 {
                sendText += ","
            }
            sendText += #" { "no": $no, "name": "$name" } "#
                .replacingOccurrences(of: "$no", with: "\(i + 1)")
                .replacingOccurrences(of: "$name", with: interface)
        }
        sendText += #" ] "#
        sendText += #" } "#

        server?.send(text: sendText)
    }
}
