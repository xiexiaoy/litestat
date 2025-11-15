#pragma once

#include <cstdint>
#include <vector>

#include "record.h"
#include "tls_stat.h"

namespace litestat
{
class Stat
{
    friend class StatManager;

public:
    Stat(int16_t thd_cnt) : stat_vec_(thd_cnt)
    {
    }
    Record New(int16_t thd_id, bool defer_stat = false)
    {
        return stat_vec_[thd_id].New(defer_stat);
    }

    Record NewRC(int16_t thd_id, int64_t *ret_code, bool defer_stat = false)
    {
        return stat_vec_[thd_id].NewRC(ret_code, defer_stat);
    }

private:
    Record MakeRecord();

private:
    std::vector<TLSStat> stat_vec_;
};
}  // namespace litestat
