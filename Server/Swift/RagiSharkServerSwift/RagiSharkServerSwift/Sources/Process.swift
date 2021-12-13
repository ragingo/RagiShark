//
//  Process.swift
//  RagiSharkServerSwift
//
//  Created by ragingo on 2021/12/13.
//

import Foundation

final class Process {
    private let applicationPath: String
    private var _process = Foundation.Process()
    private var _pipe = Foundation.Pipe()

    typealias Arguments = [String]
    private var runQueue: [Arguments] = []

    var runCompletionHandler: (() -> Void)?

    init(applicationPath: String) {
        self.applicationPath = applicationPath
    }

    private func setup(arguments: Arguments) {
        _pipe = Foundation.Pipe()
        _process = Foundation.Process()
        _process.executableURL = URL(fileURLWithPath: applicationPath)
        _process.arguments = arguments
        _process.standardOutput = _pipe
        _process.terminationHandler = onTerminated(_:)
    }

    private func onTerminated(_: Foundation.Process) {
        var slice = ArraySlice(runQueue)
        guard let arguments = slice.popFirst() else {
            return
        }

        setup(arguments: arguments)

        do {
            try _process.run()
            runCompletionHandler?()
            _process.waitUntilExit()
        } catch {
        }
    }

    func run(arguments: Arguments) {
        if _process.isRunning {
            runQueue.append(arguments)
            _process.terminate()
            return
        }

        setup(arguments: arguments)

        do {
            try _process.run()
            runCompletionHandler?()
            _process.waitUntilExit()
        } catch {
        }
    }

    func readStandardOutput() -> String {
        guard let data = try? _pipe.fileHandleForReading.readToEnd() else {
            return ""
        }
        let text = String(data: data, encoding: .utf8) ?? ""
        return text
    }
}
