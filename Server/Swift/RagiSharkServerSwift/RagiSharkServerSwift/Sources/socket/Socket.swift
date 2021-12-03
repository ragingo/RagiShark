//
//  Socket.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class Socket {
    let host: IPv4Address
    let port: Int
    let socketType: SocketType

    private var handle: SocketHandle = socketInvalidHandle

    init(host: IPv4Address, port: Int, socketType: SocketType) {
        self.host = host
        self.port = port
        self.socketType = socketType
    }

    func bind() -> Bool {
        let addressFamily = AddressFamily.internetwork.rawValue
        handle = socket(addressFamily, socketType.rawValue, 0)
        guard !handle.isInvalid else {
            print("[Socket] socket failed.")
            return false
        }

        let length = MemoryLayout<sockaddr_in>.size
        var addr = sockaddr_in()
        addr.sin_family = UInt8(addressFamily)
        addr.sin_port = CFSwapInt16HostToBig(UInt16(port))
        addr.sin_addr.s_addr = host.rawValue
        addr.sin_len = UInt8(length)

        let ret = withUnsafePointer(to: &addr) {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
                Darwin.bind(handle, $0, UInt32(length))
            }
        }
        guard ret == 0 else {
            print("[Socket] bind failed.")
            return false
        }

        return true
    }

    func listen() -> Bool {
        let ret = Darwin.listen(handle, 5)
        return ret == 0
    }

    func accept() -> SocketConnection? {
        var addr = sockaddr()
        var length = UInt32(MemoryLayout<sockaddr_in>.size)

        let sock = SocketHandle(Darwin.accept(handle, &addr, &length))
        if sock.isInvalid {
            print("[Socket] accept failed.")
            return nil
        }

        return .init(handle: .init(sock))
    }
}
