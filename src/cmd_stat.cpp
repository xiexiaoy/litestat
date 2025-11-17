#include "litestat/cmd_stat.h"

#include "litestat/cmd_registry.h"

namespace litestat
{
CmdStatBase::CmdStatBase(const char *cmd_name) : cmd_name_(cmd_name)
{
    CmdRegistry::Instance().RegisterCommand(this);
}
}  // namespace litestat
