#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


# pragma warning (disable:4819)
#include <libgo/libgo.h>
#include <core/platform/sys_last_err.h>

co::Scheduler* co_create();

/* 是否在协程中 */
bool is_in_co();

#define IN_TASK is_in_co()
//  LOG(ERROR) << "start task fail! err:" << ex.what() << " sys_err:" << sys_last_err();
#define START_TASK(FN,...) \
do {\
    try {\
        go co_scheduler(co_create()) std::bind(FN, __VA_ARGS__);\
    } catch (const std::exception & ex) {\
        LOG_IF(INFO, co_create()->TaskCount() > 2000) << "co too many!" << co_create()->TaskCount();\
    }\
} while (false)\

