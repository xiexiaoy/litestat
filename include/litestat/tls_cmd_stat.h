#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "litestat/record.h"
#include "litestat/spin_lock.h"

namespace litestat
{
struct CodeStat
{
    explicit CodeStat(int64_t status_code)
        : status_code_(status_code),
          requests_(0),
          max_latency_(0),
          min_latency_(INT64_MAX),
          sum_latency_(0),
          max_value_(0),
          min_value_(INT64_MAX),
          sum_value_(0)
    {
    }

    int64_t status_code_;
    int64_t requests_;
    std::chrono::microseconds max_latency_;
    std::chrono::microseconds min_latency_;
    std::chrono::microseconds sum_latency_;
    int64_t max_value_;
    int64_t min_value_;
    int64_t sum_value_;
};

class TLSCmdStat
{
public:
    using Uptr = std::unique_ptr<TLSCmdStat>;

    TLSCmdStat(std::thread::id thread_id, CmdStatBase *stat)
        : thread_id_(thread_id), stat_(stat)
    {
    }

    std::thread::id ThreadID() const
    {
        return thread_id_;
    }

    void OnEndStat(const Record &record);

    void Swap(std::vector<CodeStat> &rhs);

private:
    CodeStat &TryEmplace(int64_t status_code);

private:
    std::thread::id thread_id_;
    CmdStatBase *stat_;
    std::vector<CodeStat> ordered_vec_;
    SpinLock spin_lock_;
};
}  // namespace litestat
