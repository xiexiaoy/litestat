#include "commands_stat.h"
#include "litestat/stat_manager.h"

using namespace example;

void get_stat_test() {
    static CmdStat_GET get_stat;
    
}

int main(void)
{
    litestat::StatManager::Instance().Init(nullptr, 5);

    CmdStat_GET get_stat;
    // litestat::CmdStat<"eloqdata.eloqdoc.getkv"> getkv_stat;

    return 0;
}