#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.



enum
{
    RMI_CODE_OK         = 0,
    RMI_CODE_TIMEOUT    = 1,
    RMI_CODE_UNREACHABLE = 2,
};

inline int32_t __g_last_rmi_error_code__ = RMI_CODE_OK;

inline const int32_t& rmi_last_err()
{
    return __g_last_rmi_error_code__;
}

inline void rmi_clear_err()
{
    __g_last_rmi_error_code__ = RMI_CODE_OK;
}

inline void __rmi_set_last_err__(const int32_t& code)
{
    __g_last_rmi_error_code__ = code;
}