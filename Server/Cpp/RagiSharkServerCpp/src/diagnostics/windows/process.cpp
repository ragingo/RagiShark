#include "util/util.h"

#ifdef RAGII_WINDOWS

#include <iostream>
#include <thread>
#include <chrono>

#include "diagnostics/process.h"

namespace
{
    using namespace ragii::diagnostics;

    bool createProcess(const ProcessStartInfo& psi, PROCESS_INFORMATION& pi, STARTUPINFO& si)
    {
        auto cmdline = psi.Args;
        cmdline.insert(cmdline.begin(), psi.Name);

        BOOL ret = CreateProcess(
            nullptr,
            const_cast<char*>(ragii::util::join(" ", cmdline).c_str()),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!ret) {
            std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            return false;
        }

        return true;
    }
}

namespace ragii::diagnostics
{
    std::shared_ptr<Process> Process::Start(const ProcessStartInfo& psi)
    {
        ProcessStartInfo startInfo = psi;
        PROCESS_INFORMATION pi = {};
        STARTUPINFO si = {};
        si.cb = sizeof(STARTUPINFO);
        si.wShowWindow = SW_HIDE;

        if (startInfo.RedirectStdOut || startInfo.RedirectStdErr) {
            SECURITY_ATTRIBUTES sa = {};
            sa.bInheritHandle = TRUE;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);

            BOOL ret = CreatePipe(&startInfo.StdOutReadPipe, &startInfo.StdOutWritePipe, &sa, 0);
            if (ret) {
                si.dwFlags = STARTF_USESTDHANDLES;
                if (startInfo.RedirectStdOut) {
                    si.hStdOutput = startInfo.StdOutWritePipe;
                }
                if (startInfo.RedirectStdErr) {
                    si.hStdError = startInfo.StdOutWritePipe; // とりあえずまとめる
                }
            }
        }

        if (!createProcess(psi, pi, si)) {
            return {};
        }

        auto ret = std::make_shared<Process>();
        ret->m_ProcessId = pi.dwProcessId;
        ret->m_ProcessHandle = pi.hProcess;
        ret->m_ThreadHandle = pi.hThread;
        ret->m_StartInfo = std::make_unique<ProcessStartInfo>(startInfo);
        ret->m_ReceiveThread = std::thread(&Process::receive, ret);
        ret->m_ReceiveThread.detach();

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
        if (m_StartInfo->StdOutReadPipe != INVALID_HANDLE) {
            CloseHandle(m_StartInfo->StdOutReadPipe);
            m_StartInfo->StdOutReadPipe = INVALID_HANDLE;
        }
        if (m_StartInfo->StdOutWritePipe != INVALID_HANDLE) {
            CloseHandle(m_StartInfo->StdOutWritePipe);
            m_StartInfo->StdOutWritePipe = INVALID_HANDLE;
        }
    }

    void Process::receive()
    {
        if (!m_StartInfo->RedirectStdOut) {
            return;
        }
        if (m_StartInfo->StdOutReadPipe == INVALID_HANDLE) {
            return;
        }

        SetConsoleOutputCP(CP_UTF8);

        std::string buf;
        std::string stock;

        while (true) {
            buf.clear();
            buf.resize(1024);
            DWORD read = 0;
            BOOL ret = ReadFile(m_StartInfo->StdOutReadPipe, buf.data(), buf.size(), &read, nullptr);
            if (!ret || read == 0) {
                continue;
            }

            std::string tmp = stock + buf;
            std::string_view sv(tmp);
            stock.clear();

            std::string_view::size_type offset = 0;
            while (true) {
                int terminator_pos = sv.find_first_of("\r\n", offset);
                if (terminator_pos == std::string_view::npos) {
                    stock += std::string(sv.substr(offset));
                    break;
                }

                auto line = sv.substr(offset, terminator_pos - offset);
                if (m_StdOutReceivedHandler) {
                    m_StdOutReceivedHandler(std::string(line));
                }
                offset = sv.find_first_not_of("\r\n", terminator_pos);
                if (offset == std::string_view::npos) {
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

#endif
