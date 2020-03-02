#include "util/util.h"

#ifdef RAGII_WINDOWS

#include "diagnostics/process.h"

namespace
{
    bool createProcess(std::string_view app, std::vector<std::string_view> args, PROCESS_INFORMATION& pi, STARTUPINFO& si)
    {
        args.insert(args.begin(), app);
        auto commadline = ragii::util::join(" ", args);

        si.cb = sizeof(STARTUPINFO);
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

        BOOL ret = CreateProcess(
            nullptr,
            const_cast<char*>(commadline.c_str()),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!ret) {
            return false;
        }

        return true;
    }
}

namespace ragii::diagnostics
{
    std::shared_ptr<Process> Process::Start(std::string_view name, std::vector<std::string_view> args)
    {
        PROCESS_INFORMATION pi = {};
        STARTUPINFO si = {};

        if (!createProcess(name, std::move(args), pi, si)) {
            return {};
        }

        auto ret = std::make_shared<Process>();
        ret->m_ProcessId = pi.dwProcessId;
        ret->m_ProcessHandle = pi.hProcess;
        ret->m_ThreadHandle = pi.hThread;
        ret->m_StdOutHandle = si.hStdOutput;

        return ret;
    }

    void Process::close()
    {
        if (m_ProcessHandle != INVALID_HANDLE) {
            CloseHandle(m_ProcessHandle);
            m_ProcessHandle = INVALID_HANDLE;
        }
        if (m_ThreadHandle != INVALID_HANDLE) {
            CloseHandle(m_ThreadHandle);
            m_ThreadHandle = INVALID_HANDLE;
        }
        if (m_StdOutHandle != INVALID_HANDLE) {
            CloseHandle(m_StdOutHandle);
            m_StdOutHandle = INVALID_HANDLE;
        }
    }
}

#endif
