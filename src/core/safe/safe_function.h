#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <stdint.h>
#include <string.h>
#include <glog/logging.h>
#include <core/platform/os_macro.h>

/**
 * 返回成功拷贝的字节数
 */
inline int safe_memcpy(void* det, size_t det_size, const void* src, size_t src_size)
{
    if (det == NULL || det_size == 0 || src == NULL || src_size == 0) {
        LOG_IF(FATAL, true) << " safe_memcpy param err!";
        return 0;
    }
#ifdef OS_WIN
    if (memcpy_s(det, det_size, src, src_size) ==0) {
        return det_size;
    }
#endif
    if (det_size >= src_size) {
        memcpy(det, src, src_size);
        return src_size;
    } else {
        memcpy(det, src, det_size);
        return det_size;
    }
}