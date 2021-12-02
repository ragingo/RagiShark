//
//  HttpServer.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation
import CryptoKit

class HttpServer {
    private let tcpServer: TcpServer?
    private let websocketServer: WebSocketServer = .init()
    private static let webSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

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
        print("[HttpServer] connected!")

        while true {
            let (length, data) = connection.receive()
            if length <= 0 {
                continue
            }
            guard let data = data else {
                continue
            }

            let str = String(decoding: data, as: UTF8.self)
            if isValidRequestLine(string: str) {
                onHttpRequestReceived(request: str, connection: connection)
            } else {
                onDataReceived(data: data, connection: connection)
            }
        }
    }

    private func onHttpRequestReceived(request: String, connection: SocketConnection) {
        guard let headers = parseRequest(string: request) else {
            return
        }

        if headers["upgrade"] == "websocket" {
            //let version = headers["sec-websocket-version"] ?? ""
            let key = (headers["sec-websocket-key"] ?? "").trimmingCharacters(in: .whitespacesAndNewlines)
            guard let data = (key + Self.webSocketGuid).data(using: .utf8) else { return }
            let hash = Insecure.SHA1.hash(data: data)
            let hashString = Data(hash).base64EncodedString()
            let responseHeaders =
                "HTTP/1.1 101 Switching Protocols\r\n" +
                "Connection: Upgrade\r\n" +
                "Upgrade: websocket\r\n" +
                "Sec-WebSocket-Accept: \(hashString)\r\n" +
                "\r\n"
            _ = connection.send(string: responseHeaders)
        }
    }

    private func onDataReceived(data: Data, connection: SocketConnection) {

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
