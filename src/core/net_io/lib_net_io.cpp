#include "lib_net_io.h"

#include "net_io_win32.h"
#include "net_io_linux.h"

INetIO* lib_export()
{
#ifdef OS_WIN
    return new CNetIOWin32();
#endif

#ifdef OS_LINUX
    return new CNetIOLinux();
#endif
    return nullptr;
}