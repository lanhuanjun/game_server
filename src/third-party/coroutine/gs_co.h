#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


# pragma warning (disable:4819)
#include <libgo/libgo.h>


co::Scheduler* co_create();

/* 是否在协程中 */
bool is_in_co();

#define START_TASK go co_scheduler(co_create())
#define IN_CO is_in_co()