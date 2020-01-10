#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include "os_macro.h"
#include <cstdint>

#ifdef OS_WIN
#include <Windows.h>
#endif


inline int32_t sys_last_err()
{
#ifdef OS_WIN
    return GetLastError();
#elif OS_LINUX
    return errno;
#endif
    return 0;
}
