#include "util/util.h"

#ifdef RAGII_MAC

#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <mutex>
#include <memory>
#include <unistd.h>
#include "diagnostics/process.h"

namespace
{
    using namespace ragii::diagnostics;

    std::mutex outputs_mutex;
    std::deque<std::string>& outputs;

    struct StartupInfo {
        std::shared_ptr<FILE> m_ReadPipeHandle;
    };

    bool createProcess(const ProcessStartInfo& psi, StartupInfo& si)
    {
        auto cmdline = psi.Args;
        cmdline.insert(cmdline.begin(), psi.Name);

        auto name = ragii::util::join(" ", cmdline);

        std::shared_ptr<FILE> pipe(
            popen(name.c_str(), "r"),
            [](FILE* fp) {
                pclose(fp);
            }
        );
        si.m_ReadPipeHandle.swap(pipe);

        return true;
    }
}

namespace ragii::diagnostics
{
    std::shared_ptr<Process> Process::Start(const ProcessStartInfo& psi)
    {
        StartupInfo si = {};
        if (!createProcess(psi, si)) {
            return {};
        }

        // TODO:
        psi.StdOutReadPipe = si.m_ReadPipeHandle.get();

        auto ret = std::make_shared<Process>();
        ret->m_ProcessId = 9999999; // TODO:
        ret->m_ProcessHandle = 9999999; // TODO:
        ret->m_ThreadHandle = 9999999; // TODO:
        ret->m_StartInfo = std::make_unique<ProcessStartInfo>(psi);
        ret->m_ReceiveThread = std::thread(&Process::receive, ret);
        ret->m_ReceiveThread.detach();

        return ret;
    }

    void Process::close()
    {
    }

    void Process::receive()
    {
        if (!m_StartInfo->RedirectStdOut) {
            return;
        }
        if (m_StartInfo->StdOutReadPipe == INVALID_HANDLE) {
            return;
        }

        std::string buf;
        std::string stock;

        while (!feof(pipe.get())) {
            buf.clear();
            buf.resize(1024);
            if (fgets(buf, sizeof(buf), pipe.get()) == nullptr) {
                continue;
            }
            std::lock_guard<std::mutex> lock(outputs_mutex);
            outputs.push_back(buf);

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
