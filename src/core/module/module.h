#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


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
    CLS(CLS&&) = delete;\

