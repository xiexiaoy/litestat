#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "litestat/record.h"
#include "litestat/tls_cmd_stat.h"

namespace litestat
{
struct ExportCmdStat
{
    explicit ExportCmdStat(const char *cmd_name, const CodeStat &code_stat)
        : cmd_name_(cmd_name),
          status_code_(code_stat.status_code_),
          requests_(code_stat.requests_),
          max_latency_(code_stat.max_latency_),
          min_latency_(code_stat.min_latency_),
          sum_latency_(code_stat.sum_latency_),
          max_value_(code_stat.max_value_),
          min_value_(code_stat.min_value_),
          sum_value_(code_stat.sum_value_)
    {
    }

    ExportCmdStat &operator<<(const CodeStat &code_stat)
    {
        requests_ += code_stat.requests_;
        max_latency_ = std::max(max_latency_, code_stat.max_latency_);
        min_latency_ = std::min(min_latency_, code_stat.min_latency_);
        sum_latency_ += code_stat.sum_latency_;
        max_value_ = std::max(max_value_, code_stat.max_value_);
        min_value_ = std::min(min_value_, code_stat.min_value_);
        sum_value_ += code_stat.sum_value_;
        return *this;
    }

    ExportCmdStat &operator<<(const ExportCmdStat &other)
    {
        requests_ += other.requests_;
        max_latency_ = std::max(max_latency_, other.max_latency_);
        min_latency_ = std::min(min_latency_, other.min_latency_);
        sum_latency_ += other.sum_latency_;
        max_value_ = 0;
        min_value_ = 0;
        sum_value_ = 0;
        return *this;
    }

    const char *cmd_name_{nullptr};
    int64_t status_code_{0};
    int64_t requests_{0};
    std::chrono::microseconds max_latency_;
    std::chrono::microseconds min_latency_{INT64_MAX};
    std::chrono::microseconds sum_latency_;
    int64_t max_value_{0};
    int64_t min_value_{INT64_MAX};
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

    Record Create()
    {
        return Record(this, false);
    }

    Record CreateDefer()
    {
        return Record(this, true);
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
        std::map<int64_t, ExportCmdStat> aggregate_map;
        for (TLSCmdStat *tls_stat : cmd_stat_)
        {
            std::vector<CodeStat> code_stat_vec;
            tls_stat->Swap(code_stat_vec);
            for (const CodeStat &code_stat : code_stat_vec)
            {
                auto [it, inserted] = aggregate_map.try_emplace(
                    code_stat.status_code_, CmdName(), code_stat);
                if (!inserted)
                {
                    ExportCmdStat &export_cmd_stat = it->second;
                    export_cmd_stat << code_stat;
                }
            }
        }
        for (const auto &[status_code, export_cmd_stat] : aggregate_map)
        {
            *out++ = export_cmd_stat;
        }
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
