#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
void __assertion_failed(const char* condition, const char* file, const char* function, int line);
#define assert(condition) \
  ((condition) ? (void)0 : __assertion_failed(#condition, __FILE__, __FUNCTION__, __LINE__))
#endif

#if __STDC_VERSION__ >= 201112L
// The C11 way
#define static_assert(cond, msg) _Static_assert(cond, #msg)
#else
// The old, hacky way
#define static_assert(cond, msg) typedef char static_assertion_##msg[(cond) ? 1 : -1]
#endif

#ifdef __cplusplus
}
#endif

#endif
