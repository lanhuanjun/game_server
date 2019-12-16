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

#include "gs_time.h"

#ifdef OS_WIN
#include "Windows.h"
#endif

enum GSFileError
{
    GS_FILE_NO_ERROR = 0,
    GS_FILE_NOT_FOUND = 1,
};

inline int32_t __g_gs_file_last_error__ = GS_FILE_NO_ERROR;

constexpr int32_t gs_file_last_error()
{
    return __g_gs_file_last_error__;
}

constexpr void gs_file_clear_last_error()
{
    __g_gs_file_last_error__ = GS_FILE_NO_ERROR;
}

/* 返回文件最后修改unix时间戳 文件不存在则返回0*/
inline sec_t file_last_modify(const char* path)
#ifdef OS_WIN
{
    WIN32_FIND_DATA file_info;
    HANDLE hdFile = FindFirstFileA(path, &file_info);
    if (hdFile == INVALID_HANDLE_VALUE) {
        __g_gs_file_last_error__ = GS_FILE_NOT_FOUND;
        return 0;
    } else {
        FindClose(hdFile);
        return file_time_to_unix_epoch(&file_info.ftLastWriteTime);
    }
}
#elif 
{
    return 0;
}
#endif