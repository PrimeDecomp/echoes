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

template < typename L, typename R >
struct is_trivially_destructible< pair< L, R > > {
  enum { value = is_trivially_destructible< L >::value && is_trivially_destructible< R >::value };
};

template < typename L, typename R >
struct use_assignment_for_construction< pair< L, R > > {
  enum {
    value = use_assignment_for_construction< L >::value &&
    use_assignment_for_construction< R >::value
  };
};

// These legacy element policies retain the native construction null checks and
// destruction loops even though the individual members have trivial lifetimes.
template <>
struct is_trivially_destructible< pair< int, float > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< int, float > > {
  enum { value = false };
};

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
