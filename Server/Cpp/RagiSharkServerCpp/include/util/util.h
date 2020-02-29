#pragma once
#include "common/common.h"
#include <string>
#include <string_view>
#include <vector>

namespace ragii::util
{
    inline std::string join(std::string_view delimiter, std::vector<std::string_view> items) {
        std::string s;
        for (auto&& item : items) {
            if (!s.empty()) {
                s += delimiter;
            }
            s += item;
        }
        return s;
    }

    std::string createSha1Hash(std::string_view text);

    void createProcess(std::string_view path, std::vector<std::string_view> args);

    void outputSystemLastError();
}
