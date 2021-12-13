//
//  SocketCommonTypes.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

typealias SocketHandle = Int32
let socketInvalidHandle = SocketHandle(-1)

extension SocketHandle {
    var isInvalid: Bool {
        self == socketInvalidHandle
    }
}
