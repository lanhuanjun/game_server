#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.
//
#include "os_macro.h"
#ifdef OS_WIN

#include <windows.h>
#include <DbgHelp.h>
#include <cstdlib>

#ifndef _M_IX86
#error "The following code only works for x86!"
#endif

inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{
    if (pModuleName == NULL) {
        return FALSE;
    }
    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR ext[_MAX_EXT];
    WCHAR szFileName[_MAX_FNAME] = L"";
    _wsplitpath_s(pModuleName, drive, _MAX_DRIVE, dir, _MAX_DIR, szFileName, _MAX_FNAME, ext, _MAX_EXT);
    if (_wcsicmp(szFileName, L"ntdll") == 0) {
        return TRUE;

    }
    return FALSE;
}

inline BOOL CALLBACK MiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, 
    PMINIDUMP_CALLBACK_OUTPUT pOutput)
{
    if (pInput == 0 || pOutput == 0)
        return FALSE;

    switch (pInput->CallbackType) {
    case ModuleCallback:
        if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
            if (!IsDataSectionNeeded(pInput->Module.FullPath))
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
    case IncludeModuleCallback:
    case IncludeThreadCallback:
    case ThreadCallback:
    case ThreadExCallback:
        return TRUE;
    default:;
    }

    return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)
{
    HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE)) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = NULL;

        MINIDUMP_CALLBACK_INFORMATION mci;
        mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
        mci.CallbackParam = 0;

        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != NULL) ? &mdei : 0, NULL, &mci);

        CloseHandle(hFile);
    }
}

inline LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    CreateMiniDump(pExceptionInfo, "core.dmp");

    return EXCEPTION_EXECUTE_HANDLER;
}

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效
inline void DisableSetUnhandledExceptionFilter()
{
    FARPROC addr = GetProcAddress(LoadLibrary("kernel32.dll"), "SetUnhandledExceptionFilter");

    if (addr) {
        unsigned char code[16];
        int size = 0;

        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag, dwTempFlag;
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
    }
}

inline void init_mini_dump()
{
    //注册异常处理函数
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    //使SetUnhandledExceptionFilter
    DisableSetUnhandledExceptionFilter();
}
#endif