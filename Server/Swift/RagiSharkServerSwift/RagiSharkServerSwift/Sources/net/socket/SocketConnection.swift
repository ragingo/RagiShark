//
//  SocketConnection.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class SocketConnection {
    private static let receiveBufferSize = 1024

    private(set) var handle: SocketHandle = socketInvalidHandle

    init(handle: SocketHandle) {
        self.handle = handle
    }

    func close() {
        guard !handle.isInvalid else { return }
        shutdown(handle, SHUT_RDWR)
        Darwin.close(handle)
        handle = socketInvalidHandle
    }

    func send(data: Data) -> Bool {
        guard !handle.isInvalid else { return false }
        let ret = data.withUnsafeBytes { ptr -> Int in
            guard let baseAddr = ptr.baseAddress else {
                return -1
            }
            return Darwin.send(handle, baseAddr, data.count, 0)
        }
        return ret > 0
    }

    func send(string: String) -> Bool {
        guard !handle.isInvalid else { return false }
        guard let data = string.cString(using: .utf8) else {
            return false
        }
        let ret = Darwin.send(handle, data, data.count - 1, 0)
        return ret > 0
    }

    func receive() -> (length: Int, data: Data?) {
        guard !handle.isInvalid else {
            return (length: 0, data: nil)
        }

        var buffer = [UInt8](repeating: 0, count: Self.receiveBufferSize)

        let length = recv(handle, &buffer, Self.receiveBufferSize, 0)
        if length == 0 {
            return (length: 0, data: nil)
        }
        if length == -1 {
            print("[SocketConnection] errno = \(errno)")
            return (length: -1, data: nil)
        }

        let data = Data(buffer.prefix(length))
        return (length: length, data: data)
    }
}
