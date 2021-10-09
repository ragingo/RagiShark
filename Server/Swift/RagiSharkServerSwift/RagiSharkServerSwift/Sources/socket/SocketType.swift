//
//  SocketType.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

enum SocketType: Int32 {
    case datagram
    case stream
    case raw

    var rawValue: Int32 {
        switch self {
        case .datagram:
            return SOCK_DGRAM
        case .stream:
            return SOCK_STREAM
        case .raw:
            return SOCK_RAW
        }
    }
}
