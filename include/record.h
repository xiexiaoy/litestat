#pragma once

#include <chrono>

namespace litestat
{
class TLSStat;
class Record
{
    friend class TLSStat;

public:
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

private:
    Record(TLSStat *tls_stat, int64_t *ret_code, bool defer_stat)
        : ends_(false), tls_stat_(tls_stat), ret_code_(ret_code)
    {
        if (!defer_stat)
        {
            BeginStat();
        }
    }

private:
    bool ends_;
    TLSStat *tls_stat_;
    int64_t *ret_code_;
    std::chrono::steady_clock::time_point begin_time_;
    std::chrono::steady_clock::time_point end_time_;
};
}  // namespace litestat
