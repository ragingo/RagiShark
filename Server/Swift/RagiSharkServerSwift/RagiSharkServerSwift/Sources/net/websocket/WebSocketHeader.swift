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

    init() {}

    init(fin: Bool, opCode: OpCode, payloadLength: Int) {
        self.init()
        self.fin = fin
        self.opCode = opCode
        self.payloadLength = payloadLength
    }

    func toBinary() -> Data {
        let opCode = UInt8(opCode.rawValue)

        if payloadLength <= 125 {
            var bytes = Data(repeating: 0, count: 2)
            bytes[0] = UInt8(((fin ? 1 : 0) << 7) | opCode)
            bytes[1] = UInt8(payloadLength)
            return bytes
        }

        if payloadLength <= 0xffff {
            var bytes = Data(repeating: 0, count: 2 + 2)
            bytes[0] = UInt8(((fin ? 1 : 0) << 7) | opCode)
            bytes[1] = 126
            bytes[2] = UInt8((payloadLength & 0xff00) >> 8)
            bytes[3] = UInt8(payloadLength & 0x00ff)
            return bytes
        }

        var bytes = Data(repeating: 0, count: 2 + 8)
        bytes[0] = UInt8(((fin ? 1 : 0) << 7) | opCode)
        bytes[1] = 127
        bytes[2] = UInt8(payloadLength >> 56)
        bytes[3] = UInt8(payloadLength >> 48) & 0xff
        bytes[4] = UInt8(payloadLength >> 40) & 0xff
        bytes[5] = UInt8(payloadLength >> 32) & 0xff
        bytes[6] = UInt8(payloadLength >> 24) & 0xff
        bytes[7] = UInt8(payloadLength >> 16) & 0xff
        bytes[8] = UInt8(payloadLength >> 8) & 0xff
        bytes[9] = UInt8(payloadLength >> 0) & 0xff
        return bytes
    }

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

