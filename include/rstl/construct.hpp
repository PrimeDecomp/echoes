#ifndef _RSTL_CONSTRUCT
#define _RSTL_CONSTRUCT

#include "rstl/iterator.hpp"
#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"

// Release rstl keeps its precondition checks as empty statements. They still count
// toward MWCC's inline size limit, which decides where uninitialized_copy is outlined.
#define RSTL_PRECONDITION(cond) ((void)0)

#define RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(T) \
  template <> \
  struct is_trivially_destructible< T > { \
    enum { value = true }; \
  };

#define RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(T) \
  template <> \
  struct use_assignment_for_construction< T > { \
    enum { value = true }; \
  };

// This describes rstl's copy policy, not trivial default construction.
#define RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(T) \
  RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(T) \
  RSTL_DECLARE_ASSIGNMENT_CONSTRUCTION(T)

namespace rstl {
template < typename T >
struct is_trivially_destructible {
  enum { value = false };
};

template < typename T >
struct is_trivially_destructible< T* > {
  enum { value = true };
};

// Opt in only when assignment implements the required copy behavior. A trivial
// destructor alone does not establish that the type is copyable or assignable.
template < typename T >
struct use_assignment_for_construction {
  enum { value = false };
};

template < typename T >
struct use_assignment_for_construction< T* > {
  enum { value = true };
};

template < typename T, bool UseAssignment >
struct construction_policy {
  static void construct(void* dest, const T& src) { new (dest) T(src); }
};

template < typename T >
struct construction_policy< T, true > {
  static void construct(void* dest, const T& src) { *static_cast< T* >(dest) = src; }
};

RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(char)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(signed char)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(uchar)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(bool)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(float)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(int)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(uint)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(unsigned long)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(short)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(ushort)

template < typename T >
inline void construct(void* dest, const T& src) {
  construction_policy< T, use_assignment_for_construction< T >::value >::construct(dest, src);
}

template < typename T >
inline void destroy_impl(T* in) {
  if (is_trivially_destructible< T >::value) {
    return;
  }
  in->~T();
}

template < typename T >
inline void destroy(T* in) {
  destroy_impl(in);
}

template < typename It >
inline void destroy(It begin, It end);

template < typename It >
inline void destroy_impl(It begin, It end) {
  if (is_trivially_destructible< typename iterator_traits< It >::value_type >::value) {
    return;
  }
  It cur = begin;
  for (; cur != end; ++cur) {
    destroy(&*cur);
  }
}

template < typename It >
inline void destroy(It begin, It end) {
  destroy_impl(begin, end);
}

template < typename It, typename T >
static inline T uninitialized_copy(It begin, It end, T out) {
  RSTL_PRECONDITION(begin <= end);
  RSTL_PRECONDITION(begin == end || out != nullptr);
  T tmp = out;
  It cur = begin;
  for (; cur != end; ++cur, ++tmp) {
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename T >
static inline T uninitialized_copy(S* begin, S* end, T out) {
  T tmp = out;
  S* cur = begin;
  for (; cur != end; ++cur, ++tmp) {
    construct(tmp, *cur);
  }

  return tmp;
}

template < typename S, typename D >
static inline D uninitialized_copy_n(S src, int n, D dest) {
  RSTL_PRECONDITION(n >= 0);
  S it = src;
  D cur = dest;
  for (int remaining = n; remaining != 0; --remaining, ++it, ++cur) {
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
