#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

#include "gs_time.h"

#ifdef OS_WIN
#include "Windows.h"
#endif

#ifdef OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

enum GSFileError
{
    GS_FILE_NO_ERROR = 0,
    GS_FILE_NOT_FOUND = 1,
};

inline int32_t __g_gs_file_last_error__ = GS_FILE_NO_ERROR;

inline int32_t gs_file_last_error()
{
    return __g_gs_file_last_error__;
}

inline void gs_file_clear_last_error()
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
#else
{
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        __g_gs_file_last_error__ = GS_FILE_NOT_FOUND;
        return 0;
    }
    return file_stat.st_mtime;
}
#endif