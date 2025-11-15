#include "stat_manager.h"

#include "stat.h"

namespace litestat
{
StatManager &StatManager::Instance()
{
    static StatManager stat_manager;
    return stat_manager;
}

void StatManager::Init(int16_t thd_cnt, int16_t window_sec)
{
    thd_cnt_ = thd_cnt;
    window_sec_ = window_sec;
}

Stat *StatManager::DeclareCommand(const char *cmd)
{
    std::unique_lock<std::mutex> lock(cmds_stat_mux_);
    auto [iter, inserted] = cmds_stat_.try_emplace(cmd, thd_cnt_);
    Stat &stat = iter->second;
    return &stat;
}
}  // namespace litestat
