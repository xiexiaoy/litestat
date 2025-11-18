#pragma once

#include <chrono>

namespace litestat
{
class CmdStatBase;
class Record
{
    friend class TLSCmdStat;

public:
    Record(CmdStatBase *cmd_stat,
           bool defer_stat)
        : cmd_stat_(cmd_stat)
    {
        if (!defer_stat)
        {
            BeginStat();
        }
    }

    ~Record()
    {
        if (!ends_)
        {
            EndStat();
        }
    }

    void BeginStat()
    {
        begin_time_ = std::chrono::steady_clock::now();
    }

    void EndStat();

    void EndStat(int64_t status_code);

    void EndStat(int64_t status_code, int64_t value);

private:
    bool ends_{false};
    CmdStatBase *cmd_stat_{nullptr};
    int64_t status_code_{0};
    int64_t value_{0};
    std::chrono::steady_clock::time_point begin_time_;
    std::chrono::steady_clock::time_point end_time_;
};
}  // namespace litestat
