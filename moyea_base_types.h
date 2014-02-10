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
    float left;		// ������߽������
    float top;		// �����ϱ߽������
    float right;		// �����ұ߽������
    float bottom;	// �����±߽������
} rect_t;

///��ɫ
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

/*! ������������ڲ��� */
#ifndef IN
	#define IN
#endif

//! ���������ǳ��ڲ��� */
#ifndef OUT
	#define OUT
#endif

#ifndef MEXP
	///��������һ�����׳��쳣�ĺ�����Ϊ�쳣ʽ���
	#define MEXP
#endif

#ifndef MAPI
	///��������һ�����׳��쳣�ĺ�������֧��ĮҰ�쳣�������
	#define MAPI
#endif

	///˵������ӿڷ��������ǿ�����(����)��Ҫʹ�õ�
#ifndef MDEV
#	define MDEV 
#endif

	///˵������ӿڷ���һ�����ؿ�������(һ�����ڲ�ά��������)���ⲿ����У�鼴��ʹ��
#ifndef MRES
#	define MRES
#endif

///�������������
#define MAX_FONT_NAME	64

///���ˮӡ���ֳ���
#define MAX_WATER_MARK_TEXT  1024

#ifndef MAX_PATH
///���·������
	#define MAX_PATH 260
#endif

#ifndef INFINITE
	#define INFINITE	0xFFFFFFFF  // Infinite timeout  
#endif

#ifdef __cplusplus
}
#endif

#endif
