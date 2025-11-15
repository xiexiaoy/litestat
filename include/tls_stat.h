#pragma once

#include <vector>

#include "record.h"

namespace litestat
{
class Record;

struct CodeStat
{
    CodeStat(int64_t ret_code)
        : ret_code_(ret_code),
          requests_(0),
          max_latency_(0),
          min_latency_(0),
          sum_latency_(0)
    {
    }

    int64_t ret_code_;
    int64_t requests_;
    std::chrono::microseconds max_latency_;
    std::chrono::microseconds min_latency_;
    std::chrono::microseconds sum_latency_;
};

class TLSStat
{
    friend class Stat;
    friend class Record;

private:
    Record New(bool defer_stat)
    {
        return Record(this, nullptr, defer_stat);
    }

    Record NewRC(int64_t *ret_code, bool defer_stat)
    {
        return Record(this, ret_code, defer_stat);
    }

    void End(const Record &record);

    CodeStat &TryEmplace(int64_t ret_code);

private:
    std::vector<CodeStat> ordered_vec_;  // Not thread safe.
};
}  // namespace litestat
