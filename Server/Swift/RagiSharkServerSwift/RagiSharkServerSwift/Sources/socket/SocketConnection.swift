//
//  SocketConnection.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

class SocketConnection {
    private static let receiveBufferSize = Int(UInt16.max)

    private(set) var handle: SocketHandle = socketInvalidHandle
    private var receiveBuffer = UnsafeMutablePointer<UInt8>.allocate(capacity: receiveBufferSize)

    init(handle: SocketHandle) {
        self.handle = handle
        receiveBuffer.initialize(to: 0)
    }

    deinit {
        receiveBuffer.deinitialize(count: Self.receiveBufferSize)
        receiveBuffer.deallocate()
    }

    func close() {
        guard !handle.isInvalid else { return }
        shutdown(handle, SHUT_RDWR)
        Darwin.close(handle)
        handle = socketInvalidHandle
    }

    func send(data: Data) -> Bool {
        guard !handle.isInvalid else { return false }

        var _data = data
        let ret = withUnsafePointer(to: &_data) {
            Darwin.send(handle, $0, data.count, 0)
        }
        return ret > 0
    }

    func receive() -> (length: Int, data: Data?) {
        guard !handle.isInvalid else {
            return (length: 0, data: nil)
        }

        let length = recv(handle, receiveBuffer, Self.receiveBufferSize, 0)
        if length == 0 {
            return (length: 0, data: nil)
        }
        if length == -1 {
            print("error! errno = \(errno)")
            return (length: -1, data: nil)
        }

        // ここでクラッシュ
        let mutableData = NSMutableData(bytes: receiveBuffer, length: length)
        let data = Data(bytes: mutableData.bytes, count: length)
        return (length: length, data: data)
    }
}
