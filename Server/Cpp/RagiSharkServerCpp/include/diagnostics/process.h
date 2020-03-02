#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <memory>
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

    class Process final
    {
    public:
        static std::shared_ptr<Process> Start(std::string_view name, std::vector<std::string_view> args);
        void close();

        static const int INVALID_PROCESS_ID = -1;
    private:
        int m_ProcessId = INVALID_PROCESS_ID;
        Handle m_ProcessHandle = INVALID_HANDLE;
        Handle m_ThreadHandle = INVALID_HANDLE;
        Handle m_StdOutHandle = INVALID_HANDLE;
    };
}
