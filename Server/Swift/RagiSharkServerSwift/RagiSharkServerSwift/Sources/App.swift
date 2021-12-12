//
//  App.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class App {
    private let tshark = TShark()

    func run() {
        guard let server = WebSocketServer(port: 9877) else {
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
    }
}
