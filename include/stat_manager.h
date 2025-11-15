#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

namespace litestat
{
class Stat;
class StatManager
{
public:
    StatManager &Instance();
    void Init(int16_t thd_cnt, int16_t window_sec = 5);
    Stat *DeclareCommand(const char *cmd);

private:
    StatManager() = default;

private:
    int16_t thd_cnt_;
    int16_t window_sec_;
    std::mutex cmds_stat_mux_;
    std::unordered_map<std::string, Stat> cmds_stat_;
};
}  // namespace litestat
