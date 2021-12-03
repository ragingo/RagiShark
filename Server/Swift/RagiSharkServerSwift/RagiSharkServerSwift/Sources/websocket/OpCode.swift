//
//  OpCode.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/04.
//

import Foundation

enum OpCode: Int {
    case continuation = 0
    case text = 1
    case binary = 2
    case close = 8
    case ping = 9
    case pong = 10
}
