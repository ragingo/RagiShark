//
//  IPv4Address.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

enum IPv4Address: UInt32 {
    case any
    case lookback

    var rawValue: UInt32 {
        switch self {
        case .any:
            return INADDR_ANY
        case .lookback:
            return INADDR_LOOPBACK
        }
    }
}
