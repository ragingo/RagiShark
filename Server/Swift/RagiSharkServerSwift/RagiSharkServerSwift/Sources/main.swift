//
//  main.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/09/23.
//

import Foundation

func main() {
    guard let httpServer = HttpServer(port: 9877) else {
        exit(EXIT_FAILURE)
    }

    guard httpServer.start() else {
        exit(EXIT_FAILURE)
    }

    //dispatchMain()
    RunLoop.main.run()
    exit(EXIT_SUCCESS)
}

main()
