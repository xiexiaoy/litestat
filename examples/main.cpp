#include "commands_stat.h"
#include "litestat/stat_manager.h"

using namespace eloqdata;

void get_stat_test()
{
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = get_stat.Create();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = get_stat.Create();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        record.EndStat(-1);
    }
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = get_stat.Create();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        record.EndStat(1);
    }
}

void set_stat_test()
{
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = set_stat.Create();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        record.EndStat(200);
    }
}

void scan_stat_test()
{
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = scan_stat.Create();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        record.EndStat(0, 100);
    }
}

void commit_stat_test()
{
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = commit_stat.CreateDefer();
        record.BeginStat();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        record.EndStat(-1);
    }
}

void abort_stat_test()
{
    for (int i = 0; i < 100; ++i)
    {
        litestat::Record record = abort_stat.CreateDefer();
        record.BeginStat();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(void)
{
    litestat::StatManager::Instance().Init(nullptr, 5);

    get_stat_test();
    set_stat_test();
    scan_stat_test();
    commit_stat_test();
    abort_stat_test();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    litestat::StatManager::Instance().Shutdown();
    return 0;
}