//
//  WebSocketServer.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/10/31.
//

import Foundation
import CryptoKit

protocol WebSocketServerDelegate {
    func received(_ webSocketServer: WebSocketServer, text: String)
}

class WebSocketServer {
    private static let webSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
    private let httpServer: HttpServer?
    private var connection: SocketConnection?

    var delegate: WebSocketServerDelegate?

    init?(port: Int) {
        httpServer = HttpServer(port: port)
        httpServer?.delegate = self
    }

    func start() -> Bool {
        print("[WebSocketServer] start")
        return httpServer?.start() ?? false
    }

    func send(data: Data) -> Bool {
        guard let connection = connection else {
            return false
        }
        let header = WebSocketHeader(fin: true, opCode: .text, payloadLength: data.count)

        var bytes = Data()
        bytes.append(header.toBinary())
        bytes.append(data)

        return connection.send(data: bytes)
    }

    func send(text: String) -> Bool {
        guard let data = text.data(using: .utf8) else {
            return false
        }
        return send(data: data)
    }
}

extension WebSocketServer: HttpServerDelegate {
    func websocketClientHandshakeRequest(_: HttpServer, version: String, key: String, connection: SocketConnection) {
        print("[WebSocketServer] client handshake version: \(version), key: \(key)")

        self.connection = connection

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

    func websocketDataFrameReceived(_ httpServer: HttpServer, data: Data) {
        print("[WebSocketServer] data frame received")
        let header = WebSocketHeader.parse(from: data)
        if header.mask {
            let decodedText = Self.decodeText(from: data)
            delegate?.received(self, text: decodedText)
        }
    }
}

private extension WebSocketServer {
    static func decodeText(from data: Data) -> String {
        let maskingKeyOffset = 2
        let maskingKeyLength = 4
        let maskingKey = data.subdata(in: .init(uncheckedBounds: (lower: maskingKeyOffset, upper: maskingKeyOffset + maskingKeyLength)))

        let dataOffset = maskingKeyOffset + maskingKeyLength
        let data = data.subdata(in: .init(uncheckedBounds: (lower: dataOffset, upper: data.count)))
        var bytes = [UInt8](repeating: 0, count: data.count)
        data.enumerated().forEach { i, value in
            bytes[i] = (value ^ maskingKey[i % 4])
        }

        return String(bytes: bytes, encoding: .utf8) ?? ""
    }
}
