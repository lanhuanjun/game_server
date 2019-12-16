#pragma once
//*****************************************************************************\
// Copyright (c) 2019 lanyeo
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// Author   : lanyeo

#include "os_macro.h"

#ifdef OS_WIN

#include <Windows.h>
typedef HINSTANCE HDLIB;
typedef FARPROC LIBFUNC;
#define INVALID_LIB_FUNC nullptr
#define INVALID_LIB_HANDLE NULL

#elif 

#include <dlfcn.h>
#define INVALID_LIB_HANDLE NULL
typedef void* HDLIB;
typedef void* LIBFUNC;
#define INVALID_LIB_FUNC NULL

#endif

/* 加载模块动态库 */
inline HDLIB lib_load(const char* path)
#ifdef OS_WIN
{
    return LoadLibraryA(path);
}
#elif
{
    return INVALID_LIB_HANDLE;
}
#endif

/* 加载模块动态库的函数 */
inline LIBFUNC lib_func(HDLIB lib_handle, const char* func_name)
#ifdef OS_WIN
{
    return GetProcAddress(lib_handle, func_name);
}
#elif
{
    return NULL;
}
#endif

/* 释放模块动态库 */
inline void lib_free(HDLIB lib_handle)
#ifdef OS_WIN
{
    if (lib_handle != INVALID_HANDLE_VALUE) {
        FreeLibrary(lib_handle);
    }
}
#elif
{
    
}
#endif