#ifndef _TYPES
#define _TYPES

#include "GameVersions.h"

#ifdef __cplusplus
#include "static_assert.hpp"

// MWCC 1.3.2 rejects template-dependent alignment attributes. Keep its original
// layouts, and enforce the stored type's alignment on modern compilers.
#if defined(__MWERKS__) || defined(CLANGD)
#define ALIGNAS(N)
#else
#define ALIGNAS(N) alignas(N)
#endif

extern "C" {
#endif

#include <dolphin/types.h>

// Dolphin u32 is unsigned long
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

// Pointer to unknown, to be determined at a later date.
typedef void* unkptr;

#define SBig(x) x

#define ARRAY_SIZE(arr) static_cast< int >(sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
}
#endif

#if (defined(__cplusplus) && __cplusplus >= 201103L) || defined(__clang__)
// Use C++11 auto keyword
#define AUTO(name, val) auto name = val
#define AUTO_REF(name, val) auto& name = val
#define AUTO_CONST_REF(name, val) const auto& name = val
#else
// Use __typeof__ extension
#define AUTO(name, val) __typeof__(val) name = val
#define AUTO_REF(name, val) __typeof__(val)& name = val
#define AUTO_CONST_REF(name, val) const __typeof__(val)& name = val
#endif

#endif // _TYPES
