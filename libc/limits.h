#ifndef _LIMITS_H_
#define _LIMITS_H_

#define CHAR_BIT 8
#define SCHAR_MAX 0x7f
#define SCHAR_MIN (~SCHAR_MAX)
#define UCHAR_MAX 0xffU

#if defined(__MWERKS__)
#if __option(unsigned_char)
#define CHAR_MIN 0U
#define CHAR_MAX UCHAR_MAX
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif

#define SHRT_MAX 0x7fff
#define SHRT_MIN (~SHRT_MAX)

#define USHRT_MAX 0xffffU

#define INT_MAX 0x7fffffff
#define INT_MIN (~INT_MAX)
#define UINT_MAX 0xffffffffU

#define LONG_MAX 0x7fffffffL
#define LONG_MIN (~LONG_MAX)
#define ULONG_MAX 0xffffffffUL

#endif
