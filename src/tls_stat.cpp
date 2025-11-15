#include "tls_stat.h"

#include <algorithm>

namespace litestat
{
void TLSStat::End(const Record &record)
{
    int64_t ret_code = record.ret_code_ ? *record.ret_code_ : 0;
    CodeStat &stat = TryEmplace(ret_code);

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
}

CodeStat &TLSStat::TryEmplace(int64_t ret_code)
{
    auto iter = std::lower_bound(
        ordered_vec_.begin(), ordered_vec_.end(), ret_code,
        [](const CodeStat &s, const int64_t &v) { return s.ret_code_ < v; });

    if (iter == ordered_vec_.end() || iter->ret_code_ != ret_code) {
        iter = ordered_vec_.emplace(iter, ret_code);
    }

    return *iter;
}
}  // namespace litestat
