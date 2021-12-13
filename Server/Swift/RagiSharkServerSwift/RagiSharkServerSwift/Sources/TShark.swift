//
//  TShark.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/12.
//

import Foundation

final class TShark {
    private static let appPath = "/Applications/Wireshark.app/Contents/MacOS/tshark"

    private var process = Process()
    private var pipe = Pipe()

    typealias Arguments = [String]
    typealias QueueItem = Arguments

    private var runQueue: [QueueItem] = []

    var isRunning: Bool {
        process.isRunning
    }

    init() {
        setupProcess()
    }

    private func setupProcess() {
        pipe = Pipe()
        process.executableURL = URL(fileURLWithPath: Self.appPath)
        process.standardOutput = pipe
        process.terminationHandler = onTerminated(process:)
    }

    private func onTerminated(process: Process) {
        if process.isRunning {
            return
        }

        var slice = ArraySlice(runQueue)
        guard let arguments = slice.popFirst() else {
            return
        }

        self.process = Process()
        self.process.arguments = arguments
        setupProcess()

        do {
            try process.run()
            process.waitUntilExit()
        } catch {
        }
    }

    private func run(arguments: [String]) -> Bool {
        if process.isRunning {
            runQueue.append(arguments)
            process.terminate()
            return true
        }

        process = Process()
        process.arguments = arguments
        setupProcess()

        do {
            try process.run()
            process.waitUntilExit()
            return true
        } catch {
            return false
        }
    }

    private func readStandardOutput() -> String {
        guard let data = try? pipe.fileHandleForReading.readToEnd() else {
            return ""
        }
        let text = String(data: data, encoding: .utf8) ?? ""
        return text
    }

    func interfaces() -> [String] {
        if !run(arguments: ["-D"]) {
            return []
        }

        guard let regex = try? NSRegularExpression(pattern: #"^(\d+)\. (.+)$"#) else {
            return []
        }

        var interfaces: [String] = []
        let stdout = readStandardOutput()

        stdout.enumerateLines { line, _ in
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
