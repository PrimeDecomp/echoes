#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "rstl/iterator.hpp"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
template < typename T >
struct is_trivially_destructible {
  enum { value = false };
};

template < typename T >
static inline void construct_impl(void* dest, const T& src) {
  new (dest) T(src);
}

template < typename T >
static inline void construct(void* dest, const T& src) {
  construct_impl(dest, src);
}

template < typename T >
static inline void destroy_impl(T* in) {
  in->~T();
}

template < typename T >
static inline void destroy(T* in) {
  destroy_impl(in);
}

template < typename It >
static inline void destroy(It begin, It end);

template < typename It >
static inline void destroy_impl(It begin, It end) {
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}

template < typename It >
static inline void destroy(It begin, It end) {
  destroy_impl(begin, end);
}

template < typename It, typename T >
static T uninitialized_copy(It begin, It end, T out) {
  T tmp = out;
  It cur = begin;
  for (; cur != end; ++tmp, ++cur) {
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename T >
static inline T uninitialized_copy(S* begin, S* end, T out) {
  T tmp = out;
  S* cur = begin;
  for (; cur != end; ++tmp, ++cur) {
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename D >
static inline D uninitialized_copy_n(S src, int n, D dest) {
  S it = src;
  D cur = dest;
  for (int i = 0; i < n; ++cur, ++i, ++it) {
    construct(&*cur, *it);
  }

  return cur;
}

template < typename D, typename S >
static inline void uninitialized_fill_n(D dest, int n, const S& value) {
  D cur = dest;
  for (int i = 0; i < n; ++i, ++cur) {
    construct(&*cur, value);
  }
}
} // namespace rstl

#endif // _RSTL_CONSTRUCT
