//
//  TShark.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/12.
//

import Foundation

final class TShark {
    private static let appPath = "/Applications/Wireshark.app/Contents/MacOS/tshark"

    private let process = Process(applicationPath: appPath)

    private func run(arguments: [String], completion: (() -> Void)? = nil) {
        process.runCompletionHandler = completion
        process.run(arguments: arguments)
    }

    func interfaces(completion: @escaping ([String]) -> Void) {
        run(arguments: ["-D"]) { [weak self] in
            guard let self = self else {
                return
            }
            let stdout = self.process.readStandardOutput()
            let interfaces = self.parseInterfaceListString(string: stdout)
            completion(interfaces)
        }
    }

    private func parseInterfaceListString(string: String) -> [String] {
        guard let regex = try? NSRegularExpression(pattern: #"^(\d+)\. (.+)$"#) else {
            return []
        }

        var interfaces: [String] = []

        string.enumerateLines { line, _ in
            let matches = regex.matches(in: line, options: [], range: .init(location: 0, length: line.count))
            matches.forEach { result in
                let nsrange = result.range(at: 2)
                guard let range = Range(nsrange, in: line) else {
                    return
                }
                interfaces.append(String(line[range]))
            }
        }

        return interfaces
    }
}
