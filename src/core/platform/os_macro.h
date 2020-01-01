#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#ifdef _WIN32

#define OS_WIN
#define OS_WIN32

#endif

#ifdef _WIN64

#define OS_WIN64

#endif

#ifdef __linux

#define OS_LINUX

#endif

#ifdef __unix
#define OS_UNIX
#endif

#ifdef __APPLE__
#define OS_MAC
#endif