//
//  WebSocketServer.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/10/31.
//

import Foundation

class WebSocketServer {
    var connection: SocketConnection? = nil

    init() {
    }

    func handshake(version: String, key: String) {
        print("[WebSocketServer] version: \(version), key: \(key)")
        
    }
}
