#include "lib_net_io.h"

#include "net_io_win32.h"

INetIO* lib_export()
{
#ifdef OS_WIN
    return new CNetIOWin32();
#endif
    return nullptr;
}