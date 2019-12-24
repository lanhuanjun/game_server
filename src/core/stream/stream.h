#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.

struct IStream;
struct ISerialize
{
    virtual void __to__(IStream* stream) const = 0;
    virtual void __from__(IStream* stream) = 0;
};


struct IStream
{
    virtual void write(const char* data, const size_t& len) = 0;
    virtual void read(char* data, const size_t& len) = 0;
};