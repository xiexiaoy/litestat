#include "litestat/tls_cmd_stat.h"

#include <algorithm>
#include <mutex>

#include "litestat/spin_lock.h"

namespace litestat
{
void TLSCmdStat::OnEndStat(const Record &record)
{
    std::lock_guard<SpinLock> lk(spin_lock_);

    CodeStat &stat = TryEmplace(*record.status_code_ptr_);

    auto time_delta = std::chrono::duration_cast<std::chrono::microseconds>(
        record.end_time_ - record.begin_time_);

    stat.requests_ += 1;
    if (time_delta > stat.max_latency_)
    {
        stat.max_latency_ = time_delta;
    }
    if (time_delta < stat.min_latency_)
    {
        stat.min_latency_ = time_delta;
    }
    stat.sum_latency_ += time_delta;

    stat.sum_value_ += *record.value_ptr_;
    if (*record.value_ptr_ > stat.max_value_)
    {
        stat.max_value_ = *record.value_ptr_;
    }
    if (*record.value_ptr_ < stat.min_value_)
    {
        stat.min_value_ = *record.value_ptr_;
    }
}

void TLSCmdStat::Swap(std::vector<CodeStat> &ordered_vec)
{
    std::lock_guard<SpinLock> lk(spin_lock_);
    ordered_vec_.swap(ordered_vec);
}

CodeStat &TLSCmdStat::TryEmplace(int64_t status_code)
{
    auto less = [](const CodeStat &s, const int64_t &v)
    { return s.status_code_ < v; };
    auto iter = std::lower_bound(
        ordered_vec_.begin(), ordered_vec_.end(), status_code, less);

    if (iter == ordered_vec_.end() || iter->status_code_ != status_code)
    {
        iter = ordered_vec_.emplace(iter, status_code);
    }

    return *iter;
}
}  // namespace litestat
