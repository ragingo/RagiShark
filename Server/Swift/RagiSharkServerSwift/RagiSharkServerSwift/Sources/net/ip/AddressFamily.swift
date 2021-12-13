//
//  AddressFamily.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

enum AddressFamily: Int32 {
    case internetwork

    var rawValue: Int32 {
        switch self {
        case .internetwork:
            return AF_INET
        }
    }
}
