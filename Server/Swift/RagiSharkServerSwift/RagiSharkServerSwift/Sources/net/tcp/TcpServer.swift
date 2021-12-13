//
//  TcpServer.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class TcpServer {
    static let defaultReceiveBufferLength = 1024

    private let serverSocket: Socket

    typealias ConnectionHandler = (SocketConnection) -> Void
    var onConnected: (ConnectionHandler)?

    init?(host: IPv4Address, port: Int) {
        if port < 0 {
            return nil
        }
        serverSocket = .init(host: host, port: port, socketType: .stream)
    }

    func start() -> Bool {
        print("[TcpServer] start")
        guard serverSocket.bind() else {
            return false
        }
        guard serverSocket.listen() else {
            return false
        }
        guard let connection = serverSocket.accept() else {
            return false
        }
        onConnected?(connection)
        return true
    }
}
