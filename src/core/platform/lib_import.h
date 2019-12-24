#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



#include "os_macro.h"

#ifdef OS_WIN

#include <Windows.h>
typedef HINSTANCE HDLIB;
typedef FARPROC LIBFUNC;
#define INVALID_LIB_FUNC nullptr
#define INVALID_LIB_HANDLE NULL

#else

#include <dlfcn.h>


#define INVALID_LIB_HANDLE NULL
#define INVALID_LIB_FUNC NULL

typedef void* HDLIB;
typedef void* LIBFUNC;

#endif

/* 加载模块动态库 */
inline HDLIB lib_load(const char* name)
#ifdef OS_WIN
{
    return LoadLibraryA(name);
}
#else
{
    return dlopen(name, RTLD_NOW | RTLD_GLOBAL);
}
#endif

/* 加载模块动态库的函数 */
inline LIBFUNC lib_func(HDLIB lib_handle, const char* func_name)
#ifdef OS_WIN
{
    return GetProcAddress(lib_handle, func_name);
}
#else
{
    return dlsym(lib_handle, func_name);
}
#endif

/* 释放模块动态库 */
inline void lib_free(HDLIB lib_handle)
#ifdef OS_WIN
{
    if (lib_handle != INVALID_LIB_FUNC) {
        FreeLibrary(lib_handle);
    }
}
#else
{
    if (lib_handle != INVALID_LIB_FUNC) {
        dlclose(lib_handle);
    }
}
#endif