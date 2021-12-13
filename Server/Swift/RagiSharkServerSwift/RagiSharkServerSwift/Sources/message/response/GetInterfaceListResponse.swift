//
//  GetInterfaceListResponse.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/13.
//

import Foundation

struct Interface: Encodable {
    let no: Int
    let name: String
}

struct GetInterfaceListResponse: Encodable {
    let type = "get_if_list_response"
    let data: [Interface]
}
