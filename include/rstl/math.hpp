#ifndef _RSTL_MATH
#define _RSTL_MATH

#include "types.h"

namespace rstl {
template < typename T >
inline T min_val(T a, T b) {
  return (b < a) ? b : a;
}

template < typename T >
inline T max_val(T a, T b) {
  return (a < b) ? b : a;
}
} // namespace rstl

#endif // _RSTL_MATH
