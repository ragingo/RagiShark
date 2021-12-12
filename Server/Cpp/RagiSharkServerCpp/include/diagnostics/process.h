#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include "common/common.h"

namespace ragii::diagnostics
{
#if defined(RAGII_WINDOWS)
    using Handle = void*;
    static const Handle INVALID_HANDLE = (Handle)-1;
#else
    using Handle = int;
    static const Handle INVALID_HANDLE = -1;
#endif

    struct ProcessStartInfo
    {
        bool RedirectStdOut = false;
        bool RedirectStdErr = false;
        Handle StdOutReadPipe = INVALID_HANDLE;
        Handle StdOutWritePipe = INVALID_HANDLE;
        std::string_view Name;
        std::vector<std::string_view> Args;
    };

    class Process final
    {
    public:
        static std::shared_ptr<Process> Start(const ProcessStartInfo& psi);
        void close();

        int getProcessId() const { return m_ProcessId; }
        Handle getProcessHandle() const { return m_ProcessHandle; }
        Handle getThreadHandle() const { return m_ThreadHandle; }

        void setStdOutReceivedHandler(std::function<void(const std::string&)> handler) {
            m_StdOutReceivedHandler = handler;
        }

        static const int INVALID_PROCESS_ID = -1;
    private:
        void receive();

        int m_ProcessId = INVALID_PROCESS_ID;
        Handle m_ProcessHandle = INVALID_HANDLE;
        Handle m_ThreadHandle = INVALID_HANDLE;
        std::unique_ptr<ProcessStartInfo> m_StartInfo;
        std::function<void(std::string)> m_StdOutReceivedHandler;
        std::thread m_ReceiveThread;
    };
}
