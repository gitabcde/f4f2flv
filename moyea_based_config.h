/*!
@brief MoyeaBased�ı��������ļ�
@note �Ǳ�MoyeaBased�����벻Ҫ���б༭
*/
#ifndef __MOYEA_BASED_CONFIG_H
#define __MOYEA_BASED_CONFIG_H

//�Ƿ�����ⲿ�����ļ�
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*!
һЩ��������ı�־��
_MSC_VER	VC++������
__GNUC__	GCC
__MINGW32__	GCC MINGW
__i386__	GCC ��ǰ����x86�Ĵ���
__amd64__   GCC ��ǰ����x64�Ĵ���
__ppc__		GCC ��ǰ����PPC�Ĵ���
__ppc64__	GCC ��ǰ����PPC64�Ĵ���
__APPLE__	MAC ƽ̨
_WIN32		WIN32 ƽ̨
_WIN64		WIN64 ƽ̨
NDEBUG		�Ƿ��ڷǵ��԰��±���
*/

//ƽ̨����
//#define ARCH_X86_32
//#define ARCH_X86_64
//#define ARCH_PPC
//#define ARCH_PPC64

//ͨ������ARCH_DETECTED���Ա���moyea_based_conf.h��ƽ̨���м��
#ifndef ARCH_DETECTED
#define ARCH_DETECTED

//ƽ̨���
#ifdef __GNUC__ //GCC������

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

#elif defined(_MSC_VER) //VC++ ������������ļ�ⷽ�����ܲ���ȷ
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

///ʹ��WIN32
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN //�Ƴ�windows������winsock.h��ֹ�������ط�������winsock2��ͻ
#include <windows.h>
#endif

///ϵͳ�Ƿ���_aligned_mallocϵ�к���
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

//�Ƿ���<inttypes>.h
#ifndef CONFIG_HAVE_INTTYPES

#ifdef _MSC_VER
	#define CONFIG_HAVE_INTTYPES 0
#else	
	#define CONFIG_HAVE_INTTYPES 1
	#include <inttypes.h>
#endif
#endif

//��Ȿ���ַ����Ƿ���utf8�ַ���
#ifndef CONFIG_LOCAL_IS_UTF8

#if defined(WIN32) || defined(_WIN64)
	#define CONFIG_LOCAL_IS_UTF8 0
#else
	#define CONFIG_LOCAL_IS_UTF8 1
#endif

#endif

#endif