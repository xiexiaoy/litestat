#include "litestat/stat_manager.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <mutex>
#include <thread>

#include "litestat/cmd_stat.h"
#include "litestat/file_ostream.h"
#include "litestat/cmd_registry.h"

namespace litestat
{
StatManager &StatManager::Instance()
{
    static StatManager stat_manager;
    return stat_manager;
}

void StatManager::Init(const char *filepath, int16_t window_sec)
{
    filepath = filepath;
    if (filepath != nullptr)
    {
        FILE *file = fopen(filepath, "a");
        if (file == nullptr)
        {
            perror(filepath);
        }
        else
        {
            file_ = file;
        }
    }

    window_sec_ = window_sec;

    shutdown_ = false;
    output_thread_ = std::thread([this]() { OutputLoop(); });
    initialized_.store(true, std::memory_order_release);
}

void StatManager::OutputLoop()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(window_sec_));
        std::vector<ExportCmdStat> exports = CmdRegistry::Instance().ExportAndReset();
        PrettyPrint(exports);
    }
}

void StatManager::PrettyPrint(const std::vector<ExportCmdStat> &exports)
{
    FileOStream os(file_);

    // 列标题
    const std::vector<std::string> headers = {"CMD",
                                              "STATUS",
                                              "REQUESTS",
                                              "MAX(us)",
                                              "MIN(us)",
                                              "AVG(us)",
                                              "MAX(n)",
                                              "MIN(n)",
                                              "AVG(n)"};

    // 每列宽度
    std::vector<size_t> widths(headers.size());
    for (size_t i = 0; i < headers.size(); i++)
        widths[i] = headers[i].size();

    // 根据数据更新列宽
    for (const auto &s : exports)
    {
        int64_t avg_latency =
            s.requests_ > 0 ? s.sum_latency_.count() / s.requests_ : 0;
        int64_t avg_value = s.requests_ > 0 ? s.sum_value_ / s.requests_ : 0;

        std::vector<std::string> cols = {s.cmd_name_ ? s.cmd_name_ : "",
                                         std::to_string(s.status_code_),
                                         std::to_string(s.requests_),
                                         std::to_string(s.max_latency_.count()),
                                         std::to_string(s.min_latency_.count()),
                                         std::to_string(avg_latency),
                                         std::to_string(s.max_value_),
                                         std::to_string(s.min_value_),
                                         std::to_string(avg_value)};

        for (size_t i = 0; i < cols.size(); i++)
            widths[i] = std::max(widths[i], cols[i].size());
    }

    // 计算表格总宽度（包括每列后面的 2 个空格）
    size_t total_width = 0;
    for (auto w : widths)
        total_width += w + 2;

    // ================= 时 间 标 题 行 ====================
    {
        // 生成时间字符串
        char ts[32];
        std::time_t t = std::time(nullptr);
        std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", std::localtime(&t));

        std::string mid = std::string(" ") + ts + " ";

        // 左边“==”
        os << "==";

        // 剩余宽度 = 总宽度 - "=="*2 - 时间段长度
        size_t remaining =
            total_width > mid.size() + 4 ? (total_width - mid.size() - 4) : 0;

        // 中间部分
        os << mid;

        // 右侧填 '='
        os << std::string(remaining, '=');

        os << "==" << "\n";
    }

    // 打印表头
    for (size_t i = 0; i < headers.size(); i++)
        os << std::left << std::setw(widths[i] + 2) << headers[i];
    os << "\n";

    // 分隔线
    os << std::string(total_width, '-') << "\n";

    // 内容
    for (const auto &s : exports)
    {
        float avg_latency =
            s.requests_ > 0 ? s.sum_latency_.count() / (float) s.requests_ : 0;
        float avg_value =
            s.requests_ > 0 ? s.sum_value_ / (float) s.requests_ : 0;

        std::vector<std::string> cols = {s.cmd_name_ ? s.cmd_name_ : "",
                                         std::to_string(s.status_code_),
                                         std::to_string(s.requests_),
                                         std::to_string(s.max_latency_.count()),
                                         std::to_string(s.min_latency_.count()),
                                         std::to_string(avg_latency),
                                         std::to_string(s.max_value_),
                                         std::to_string(s.min_value_),
                                         std::to_string(avg_value)};

        for (size_t i = 0; i < cols.size(); i++)
            os << std::left << std::setw(widths[i] + 2) << cols[i];
        os << "\n";
    }
}

void StatManager::Shutdown()
{
    {
        std::lock_guard<std::mutex> lk(sleep_mux_);
        shutdown_ = true;
        sleep_cv_.notify_one();
    }

    if (file_ != stdout && file_ != stderr)
    {
        fclose(file_);
    }
}
}  // namespace litestat
