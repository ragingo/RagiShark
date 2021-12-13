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

struct Interface: Encodable {
    let no: Int
    let name: String
}
struct GetInterfaceListResponse: Encodable {
    let type = "get_if_list_response"
    let data: [Interface]
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
        print("[App:onGetIFList] interfaces")

        let interfaces = tshark.interfaces().enumerated().map { i, interface in
            Interface(no: i + 1, name: interface)
        }

        let response = GetInterfaceListResponse(data:interfaces)
        let sendText: String
        do {
            let json = try JSONEncoder().encode(response)
            sendText = String(data: json, encoding: .utf8) ?? "" // TODO: Data をそのまま送信する
        } catch {
            print(error)
            return
        }

        _ = server?.send(text: sendText)
    }
}
