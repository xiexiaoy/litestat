#include "record.h"

#include "tls_stat.h"

namespace litestat
{
void Record::EndStat()
{
    end_time_ = std::chrono::steady_clock::now();
    ends_ = true;
    tls_stat_->End(*this);
}
}  // namespace litestat
