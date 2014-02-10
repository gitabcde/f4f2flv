#ifndef __MOYEA_BASE_TYPES_H
#define __MOYEA_BASE_TYPES_H

#include <stdio.h>
#include <assert.h>
#include "moyea_based_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THISCALL	__thiscall
#define CCALL		__cdecl

#ifdef _MSC_VER
	#define STDCALL		__stdcall
#elif defined(__GNUC__)
    #if defined(__ppc__) || defined(__ppc64__)
		#define STDCALL 
	#else
		#define STDCALL __attribute__((stdcall))
	#endif
#else 
    #define STDCALL 
#endif

#define INLINE		__inline
#ifndef CALLBACK
	#define CALLBACK	STDCALL
#endif

#ifdef __APPLE__
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

#if !CONFIG_HAVE_INTTYPES

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

#ifdef _MSC_VER	
	typedef signed __int64   int64_t;
    typedef unsigned __int64 uint64_t;
#else			// other OS, Mac OS X
	typedef signed long long   int64_t;
	typedef unsigned long long uint64_t;
#endif

#endif

typedef struct 
{
	float x;
	float y;
} point_t;

typedef struct
{
    float left;		// 距离左边界的象素
    float top;		// 距离上边界的象素
    float right;		// 距离右边界的象素
    float bottom;	// 距离下边界的象素
} rect_t;

///颜色
typedef struct 
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
} color_t;

typedef struct  
{
	uint32_t startTime;	// ms
	uint32_t endTime;	// ms
}DURATION;

/*! 表明参数是入口参数 */
#ifndef IN
	#define IN
#endif

//! 表明参数是出口参数 */
#ifndef OUT
	#define OUT
#endif

#ifndef MEXP
	///表明这是一个会抛出异常的函数，为异常式编程
	#define MEXP
#endif

#ifndef MAPI
	///表明这是一个不抛出异常的函数，且支持漠野异常报告机制
	#define MAPI
#endif

	///说明这个接口方法或类是开发者(可能)需要使用的
#ifndef MDEV
#	define MDEV 
#endif

	///说明这个接口方法一定返回可用数据(一般是内部维护的数据)，外部无需校验即可使用
#ifndef MRES
#	define MRES
#endif

///最大字体名长度
#define MAX_FONT_NAME	64

///最大水印文字长度
#define MAX_WATER_MARK_TEXT  1024

#ifndef MAX_PATH
///最大路径长度
	#define MAX_PATH 260
#endif

#ifndef INFINITE
	#define INFINITE	0xFFFFFFFF  // Infinite timeout  
#endif

#ifdef __cplusplus
}
#endif

#endif
