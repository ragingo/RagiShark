//
//  HttpServer.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class HttpServer {
    private let tcpServer: TcpServer?

    init?(port: Int) {
        guard let tcpServer = TcpServer(host: .any, port: port) else {
            return nil
        }
        self.tcpServer = tcpServer
        tcpServer.onConnected = onClientConnected
    }

    func start() -> Bool {
        guard let tcpServer = tcpServer else { return false }
        guard tcpServer.start() else { return false }
        return true
    }

    private func onClientConnected(connection: SocketConnection) {
        print("connected!")

        while true {
            let (length, data) = connection.receive()
            if length <= 0 {
                break
            }
            guard let data = data else {
                break
            }
            guard let str = String(bytes: data, encoding: .utf8) else {
                continue
            }
            print("received!")

            guard let headers = parseRequest(string: str) else {
                continue
            }
            if headers["upgrade"] == "websocket" {
                // WebSocket
                let websocketVersion = headers["sec-websocket-version"]
                let websocketKey = headers["sec-websocket-key"]
                print("websocket version: \(websocketVersion), key: \(websocketKey)")
            } else {
                // HTTP
            }
        }
    }

    typealias Headers = [String: String]

    private func parseRequest(string: String) -> Headers? {
        let lines = string.split(separator: "\r\n", omittingEmptySubsequences: true).map { String($0) }
        guard isValidRequestLine(string: lines[0]) else {
            return nil
        }

        let headerLines = lines[1...]
        var headers: Headers = [:]

        headerLines.forEach { headerLine in
            guard let separatorIndex = headerLine.firstIndex(of: ":") else {
                return
            }
            let name = String(headerLine[headerLine.startIndex..<separatorIndex]).lowercased()
            let value = String(headerLine[headerLine.index(after: separatorIndex)...]).trimmingCharacters(in: .whitespaces)
            headers.updateValue(value, forKey: name)
        }

        return headers
    }

    // リクエストラインが正しいかどうか大まかに判定
    private func isValidRequestLine(string: String) -> Bool {
        let parts = string.split(separator: " ", omittingEmptySubsequences: true).map { String($0) }
        let methods = ["GET", "POST", "PUT", "DELETE", "OPTIONS", "PATCH"]

        let method = parts[0]
        guard methods.contains(method) else {
            return false
        }

        let path = parts[1]
        guard path == "/" else {
            return false
        }

        let version = parts[2]
        guard version.starts(with: "HTTP/") else {
            return false
        }

        return true
    }
}
