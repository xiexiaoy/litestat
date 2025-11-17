#pragma once

#include <cstring>
#include <iterator>
#include <map>
#include <mutex>
#include <vector>

#include "litestat/cmd_stat.h"

namespace litestat
{
class CmdRegistry
{
public:
    struct CStrLess
    {
        bool operator()(const char *lhs, const char *rhs) const
        {
            return strcmp(lhs, rhs) == -1;
        }
    };

public:
    static CmdRegistry &Instance()
    {
        // Use call_once to ensure thread-safe lazy initialization
        static std::once_flag flag;
        static std::unique_ptr<CmdRegistry> instance;
        std::call_once(flag,
                       []() { instance = std::make_unique<CmdRegistry>(); });
        return *instance;
    }

    void RegisterCommand(CmdStatBase *cmd_stat)
    {
        std::lock_guard<std::mutex> lk(mutex_);
        stat_map_.emplace(cmd_stat->CmdName(), cmd_stat);
    }

    std::vector<ExportCmdStat> ExportAndReset()
    {
        std::lock_guard<std::mutex> lk(mutex_);
        std::vector<ExportCmdStat> exports;
        for (auto &[cmd_name, cmd_stat] : stat_map_)
        {
            cmd_stat->ExportAndReset(std::back_inserter(exports));
        }
        return {};
    }

private:
    mutable std::mutex mutex_;
    std::map<const char *, CmdStatBase *, CStrLess> stat_map_;
};
}  // namespace litestat