#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

#include "litestat/record.h"
#include "litestat/tls_cmd_stat.h"

namespace litestat
{
struct ExportCmdStat
{
    const char *cmd_name_{nullptr};
    int64_t status_code_{0};
    int64_t requests_{0};
    std::chrono::microseconds max_latency_;
    std::chrono::microseconds min_latency_;
    std::chrono::microseconds sum_latency_;
    int64_t max_value_{0};
    int64_t min_value_{0};
    int64_t sum_value_{0};
};

class CmdStatBase
{
public:
    using Uptr = std::unique_ptr<CmdStatBase>;

    explicit CmdStatBase(const char *cmd_name);

    const char *CmdName() const
    {
        return cmd_name_;
    }

    Record Create(bool defer_stat = false)
    {
        return Record(this, nullptr, nullptr, defer_stat);
    }

    Record Create(int64_t *ret_code, bool defer_stat = false)
    {
        return Record(this, ret_code, nullptr, defer_stat);
    }

    virtual void OnEndStat(const Record &record) = 0;

    virtual void ExportAndReset(
        std::back_insert_iterator<std::vector<ExportCmdStat>> out) = 0;

private:
    const char *cmd_name_;
};

template <const char *cmd>
class CmdStat final : public CmdStatBase
{
    friend class Record;

public:
    CmdStat() : CmdStatBase(cmd)
    {
    }

    void OnEndStat(const Record &record) override
    {
        EnsureTLSCmdStat(std::this_thread::get_id());
        tls_cmd_stat_->OnEndStat(record);
    }

    void ExportAndReset(
        std::back_insert_iterator<std::vector<ExportCmdStat>> out) override
    {
        std::lock_guard<std::mutex> lk(cmd_stat_mux_);
    }

private:
    void EnsureTLSCmdStat(std::thread::id thread_id)
    {
        if (tls_cmd_stat_ == nullptr)
        {
            std::scoped_lock<std::mutex> lk(cmd_stat_mux_);
            auto less = [](const TLSCmdStat *tls_cmd_stat,
                           const std::thread::id thread_id)
            { return tls_cmd_stat->ThreadID() < thread_id; };

            auto iter = std::lower_bound(
                cmd_stat_.begin(), cmd_stat_.end(), thread_id, less);
            if (iter == cmd_stat_.end() || iter->ThreadID() != thread_id)
            {
                tls_cmd_stat_ = std::make_unique<TLSCmdStat>(thread_id, this);
                cmd_stat_.emplace(iter, tls_cmd_stat_.get());
            }
        }
    }

private:
    std::mutex cmd_stat_mux_;
    std::vector<TLSCmdStat *> cmd_stat_;
    static thread_local TLSCmdStat::Uptr tls_cmd_stat_;
};
}  // namespace litestat
