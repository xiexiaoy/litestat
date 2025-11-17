#include "litestat/record.h"

#include "litestat/cmd_stat.h"

namespace litestat
{
void Record::EndStat()
{
    end_time_ = std::chrono::steady_clock::now();
    ends_ = true;
    cmd_stat_->OnEndStat(*this);
}

void Record::EndStat(int64_t status_code)
{
    status_code_ = status_code;
    status_code_ptr_ = &status_code_;
    EndStat();
}

void Record::EndStat(int64_t status_code, int64_t value)
{
    value_ = value;
    value_ptr_ = &value_;
    EndStat(status_code);
}
}  // namespace litestat
