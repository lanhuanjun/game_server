#pragma once
/*****************************************************************************\
    *  @COPYRIGHT NOTICE
    *  @Copyright (c)2019 - 2030 lanyeo
    *  @file	 : gs_assert.h
    *  @version  : ver 1.0
    
    *  @author   : lanyeo
    *  @date     : 2019年10月27日 12:00:00
    *  @brief    : 断言包装
\*****************************************************************************/

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
}																			  \

#elif

#define lightAssert(Expression)												  \
if (!(Expression))															  \
{																			  \
	LOG(ERROR) << "file:" << __FILE__ << " have error. line:" << __LINE__;	  \
}

#endif

#define AlwaysAssert(Expression)											  \
if (!(Expression))															  \
{																			  \
	LOG(FATAL) << "file:" << __FILE__ << " have error. line:" << __LINE__;	  \
}