#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include "os_macro.h"

#ifdef OS_WIN

#define LIB_EXPORT __declspec(dllexport)

#endif

#ifdef OS_LINUX
#define LIB_EXPORT
#endif