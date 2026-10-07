#ifndef _TAVERAGE
#define _TAVERAGE

#include "types.h"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

template < typename T >
T GetAverageValue(const T* ptr, int count) {
  const T* end = ptr + count;
  T sum = *ptr++;
  for (; ptr < end; ++ptr) {
    sum = sum + *ptr;
  }
  return sum * (1.f / count);
}

template < typename T >
class TAverage {
public:
  TAverage() {}
  explicit TAverage(int capacity) { mValues.reserve(capacity); }
  TAverage(int capacity, const T& value);

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
    if (mValues.empty()) {
      return rstl::optional_object_null();
    } else {
      return GetAverageValue(mValues.data(), mValues.size());
    }
  }

private:
  rstl::vector< T > mValues;
};

template < typename T >
TAverage< T >::TAverage(int capacity, const T& value) {
  mValues.resize(capacity, value);
}

template < typename T >
void TAverage< T >::AddValue(const T& value) {
  if (mValues.size() == mValues.capacity()) {
    // TODO ?
    mValues.mCount -= 1;
  }
  mValues.insert(mValues.begin(), value);
}

#endif // _TAVERAGE
