#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#define WORD_MAKE(HIGH, LOW) ((((int64_t)(((int32_t)(HIGH))&0xffffffff)) << 32 ) | ((int64_t)(((int32_t)(LOW))&0xffffffff)))
#define WORD_HIGH(WORD) ((int32_t)(((int64_t)(WORD)) >> 32))
#define WORD_LOW(WORD) ((int32_t)(WORD))