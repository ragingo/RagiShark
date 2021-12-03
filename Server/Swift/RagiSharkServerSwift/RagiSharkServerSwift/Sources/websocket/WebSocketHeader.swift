//
//  WebSocketHeader.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/04.
//

import Foundation

struct WebSocketHeader {
    var fin: Bool = false
    var rcv1: UInt8 = 0
    var rcv2: UInt8 = 0
    var rcv3: UInt8 = 0
    var opCode: OpCode = .continuation
    var mask: Bool = false
    var payloadLength: Int = 0

    static func parse(from data: Data) -> WebSocketHeader {
        var header = WebSocketHeader()
        if data.count < 2 {
            return header
        }
        let bytes = data.prefix(2)
        header.fin = (bytes[0] >> 7) == 1
        header.rcv1 = bytes[0] >> 6 & 1
        header.rcv2 = bytes[0] >> 5 & 1
        header.rcv3 = bytes[0] >> 4 & 1
        header.opCode = .init(rawValue: Int(bytes[0] & 0x0f)) ?? .continuation
        header.mask = (bytes[1] >> 7) == 1
        header.payloadLength = Int(bytes[1] & 0x7f)
        return header
    }
}

