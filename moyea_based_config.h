/*!
@brief MoyeaBased的编译配置文件
@note 非本MoyeaBased开发请不要进行编辑
*/
#ifndef __MOYEA_BASED_CONFIG_H
#define __MOYEA_BASED_CONFIG_H

//是否包含外部配置文件
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*!
一些编译器宏的标志宏
_MSC_VER	VC++编译器
__GNUC__	GCC
__MINGW32__	GCC MINGW
__i386__	GCC 当前编译x86的代码
__amd64__   GCC 当前编译x64的代码
__ppc__		GCC 当前编译PPC的代码
__ppc64__	GCC 当前编译PPC64的代码
__APPLE__	MAC 平台
_WIN32		WIN32 平台
_WIN64		WIN64 平台
NDEBUG		是否在非调试版下编译
*/

//平台代码
//#define ARCH_X86_32
//#define ARCH_X86_64
//#define ARCH_PPC
//#define ARCH_PPC64

//通过定义ARCH_DETECTED可以避免moyea_based_conf.h对平台进行检测
#ifndef ARCH_DETECTED
#define ARCH_DETECTED

//平台检测
#ifdef __GNUC__ //GCC编译器

#if defined(__i386__)
	#define ARCH_X86_32
#elif defined(__amd64__)
	#define ARCH_X86_64
#elif defined(__ppc__)
	#define ARCH_PPC
	#define ARCHI_BIGENDIAN
#elif defined(__ppc64__)
	#define ARCH_PPC64
	#define ARCHI_BIGENDIAN
#else
	#error Not detected platform
#endif

#elif defined(_MSC_VER) //VC++ 编译器，下面的检测方法可能不正确
	#ifdef _WIN64
		#define ARCH_X86_64
	#elif defined(_WIN32)
		#define ARCH_X86_32
	#else
		#error Not detected platform
	#endif
#else
	#error Not supported compiler
#endif

#endif

///使用WIN32
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //移除windows包含的winsock.h防止于其它地方包含的winsock2冲突
#include <windows.h>
#endif

///系统是否有_aligned_malloc系列函数
#ifndef CONFIG_HAVE_ALIGNED_MALLOC
#if defined(_MSC_VER)
	#if _MSC_VER > 1200 // VC6 above, not including VC6 
		#define CONFIG_HAVE_ALIGNED_MALLOC 1
	#else
		#define CONFIG_HAVE_ALIGNED_MALLOC 0
	#endif
#else
	#define CONFIG_HAVE_ALIGNED_MALLOC 0
#endif
#endif

//是否有<inttypes>.h
#ifndef CONFIG_HAVE_INTTYPES

#ifdef _MSC_VER
	#define CONFIG_HAVE_INTTYPES 0
#else	
	#define CONFIG_HAVE_INTTYPES 1
	#include <inttypes.h>
#endif
#endif

//检测本地字符串是否是utf8字符串
#ifndef CONFIG_LOCAL_IS_UTF8

#if defined(WIN32) || defined(_WIN64)
	#define CONFIG_LOCAL_IS_UTF8 0
#else
	#define CONFIG_LOCAL_IS_UTF8 1
#endif

#endif

#endif