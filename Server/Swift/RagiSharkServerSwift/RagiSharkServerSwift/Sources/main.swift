//
//  main.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/12.
//

import Foundation

func main() {
    let arguments = ["-p": "9877"] // TODO: CommandLine.arguments
    let app = App()
    app.run(arguments: arguments)
}

main()
