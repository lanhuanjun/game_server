#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : lib_export.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019Äê10ÔÂ30ÈÕ 12:00:00
    *  @brief    : 
\*****************************************************************************/

#include "os_macro.h"

#ifdef OS_WIN

#define LIB_EXPORT __declspec(dllexport)

#endif