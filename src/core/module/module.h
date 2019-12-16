#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : module.h
    *  @version  : ver 1.0

    *  @author   : lanyeo
    *  @date     : 2019/10/20 12:36:30
    *  @brief    :
\*****************************************************************************/
class IModule
{
public:
    virtual ~IModule() = default;
    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Destroy() = 0;
};

#define DISALLOW_COPY_AND_ASSIGN(CLS)             \
    CLS(const CLS&) = delete;                     \
    CLS& operator=(const CLS&) = delete;          \
    CLS(CLS&&) = delete;