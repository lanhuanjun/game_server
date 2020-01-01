#pragma once
// Copyright (c) 2019-2040 lanyeo
// Licensed under the MIT license.


#include <cassert>
#include <glog/logging.h>

#ifdef _WIN32
#define DEBUG _DEBUG
#endif

#ifdef DEBUG

#define LightAssert(Expression)												  \
if (!(Expression))															  \
{																			  \
	LOG(FATAL) << "file:" << __FILE__ << " have error. line:" << __LINE__;	  \
}																			  

#else

#define LightAssert(Expression)												  \
if (!(Expression))															  \
{																			  \
	LOG(ERROR) << "file:" << __FILE__ << " have error. line:" << __LINE__;	  \
}\


#endif

#define AlwaysAssert(Expression)											  \
if (!(Expression))															  \
{																			  \
	LOG(FATAL) << "file:" << __FILE__ << " have error. line:" << __LINE__;	  \
}\


