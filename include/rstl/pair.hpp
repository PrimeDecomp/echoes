#ifndef _RSTL_PAIR
#define _RSTL_PAIR

#include "rstl/construct.hpp"
#include "rstl/functional.hpp"
#include "types.h"

class CInputStream;
class COutputStream;

namespace rstl {
template < typename L, typename R >
class pair {
public:
  pair() {}
  pair(CInputStream& in);
  pair(const L& first, const R& second) : first(first), second(second) {}
  void PutTo(COutputStream& out) const;

  bool operator==(const pair& other) const {
    return first == other.first && second == other.second;
  }

  bool operator!=(const pair& other) const {
    return first != other.first || second != other.second;
  }

  bool operator<(const pair& other) const {
    return first < other.first || (first == other.first && second < other.second);
  }

  L first;
  R second;
};

template <>
struct is_trivially_destructible< pair< int, int > > {
  enum { value = true };
};

inline void construct_impl(void* dest, const pair< int, int >& src) {
  *static_cast< pair< int, int >* >(dest) = src;
}

template <>
struct is_trivially_destructible< pair< uint, uint > > {
  enum { value = true };
};

inline void construct_impl(void* dest, const pair< uint, uint >& src) {
  *static_cast< pair< uint, uint >* >(dest) = src;
}

template < typename P >
struct select1st : unary_function< P, P > {
  const P& operator()(const P& it) const { return it; }
};

template < typename K, typename V >
struct select1st< pair< K, V > > : unary_function< pair< K, V >, K > {
  typedef K value_type;

  const K& operator()(const pair< K, V >& it) const { return it.first; }
};

} // namespace rstl

#endif // _RSTL_PAIR
