#pragma once

#include "types.h"

#ifdef __linux
# include <endian.h>
# define __bswap16(x) __bswap_16(x)
# define __bswap32(x) __builtin_bswap32(x)
# define __bswap64(x) __builtin_bswap64(x)
#elif defined(__FreeBSD__)
# include <machine/endian.h>
#elif defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
# define __bswap16(x) OSSwapInt16(x)
# define __bswap32(x) OSSwapInt32(x)
# define __bswap64(x) OSSwapInt64(x)
#elif defined(WIN32)
#include <sys/param.h> 
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
# define __bswap16(W) ((int16_t)((((W) << 8) & 0xFF00) | (((W) >> 8) & 0x00FF)))
//# define __bswap16(x) __builtin_bswap16(x)
# define __bswap32(x) __builtin_bswap32(x)
# define __bswap64(x) __builtin_bswap64(x)
#endif
#include <math.h>

float32_t _floatSwap(float32_t value);
double64_t _doubleSwap(double64_t value);

#ifndef BYTE_ORDER
#error "no BYTE_ORDER macro"
#endif

#if BYTE_ORDER == BIG_ENDIAN
# define vmtohll(x)       (x)
# define vmtohl(x)       (x)
# define vmtohs(x)       (x)
# define vmtohf(x)       (x)
# define vmtohd(x)       (x)
# define htovmll(x)       (x)
# define htovml(x)       (x)
# define htovms(x)       (x)
# define htovmf(x)       (x)
# define htovmd(x)       (x)
#elif BYTE_ORDER == LITTLE_ENDIAN
# define vmtohll(x)    __bswap64 (x)
# define vmtohl(x)     __bswap32 (x)
# define vmtohs(x)     __bswap16 (x)
# define vmtohf(x)     _floatSwap(x)
# define vmtohd(x)     _doubleSwap(x)
# define htovmll(x)    __bswap64 (x)
# define htovml(x)     __bswap32 (x)
# define htovms(x)     __bswap16 (x)
# define htovmf(x)     _floatSwap(x)
# define htovmd(x)     _doubleSwap(x)
#endif
