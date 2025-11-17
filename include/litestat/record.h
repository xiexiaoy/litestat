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
           int64_t *status_code_ptr,
           int64_t *value_ptr,
           bool defer_stat)
        : cmd_stat_(cmd_stat),
          status_code_ptr_(&status_code_),
          value_ptr_(&value_)
    {
        if (status_code_ptr)
        {
            status_code_ptr_ = status_code_ptr;
        }
        if (value_ptr)
        {
            value_ptr_ = value_ptr;
        }
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
    int64_t *status_code_ptr_{nullptr};
    int64_t status_code_{0};
    int64_t *value_ptr_{nullptr};
    int64_t value_{0};
    std::chrono::steady_clock::time_point begin_time_;
    std::chrono::steady_clock::time_point end_time_;
};
}  // namespace litestat
