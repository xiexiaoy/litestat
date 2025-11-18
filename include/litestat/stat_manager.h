#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "litestat/cmd_stat.h"

namespace litestat
{
class CmdStatBase;
class StatManager
{
    friend class CmdStatBase;

public:
    static StatManager &Instance();

    void Init(const char *filepath = nullptr, int16_t window_sec = 5);

    void Shutdown();

private:
    StatManager() = default;

    void OutputLoop();

    void PrettyPrint(const std::vector<ExportCmdStat> &exports);

    ExportCmdStat Aggregate(const std::vector<ExportCmdStat> &exports);

    struct CStrLess
    {
        bool operator()(const char *lhs, const char *rhs) const
        {
            return strcmp(lhs, rhs) == -1;
        }
    };

private:
    std::atomic<bool> initialized_{false};
    pid_t pid_;
    const char *filepath_{nullptr};
    FILE *file_{stdout};
    int16_t window_sec_{-1};

    std::atomic<bool> shutdown_;
    std::mutex sleep_mux_;
    std::condition_variable sleep_cv_;
    std::thread output_thread_;
};
}  // namespace litestat
