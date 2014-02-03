#ifndef CX_PRIMITIVE_TYPE_H
#define CX_PRIMITIVE_TYPE_H




#ifdef unix
#include <sys/types.h>
typedef __int8_t CX_INT8;
typedef __uint8_t CX_UINT8;
typedef __int32_t CX_INT32;
typedef __uint32_t CX_UINT32;
typedef __int64_t CX_INT64;
typedef __uint64_t CX_UINT64;
#elif defined(_WIN32_)
#include <BaseTsd.h>
typedef INT8 CX_INT8
typedef UINT8 CX_UINT8
typedef INT32 CX_INT32
typedef UINT32 CX_UINT32
typedef INT64 CX_INT64
typedef UINT64 CX_UINT64
#endif




#endif
















