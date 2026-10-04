#ifndef _TRESERVEDAVERAGE
#define _TRESERVEDAVERAGE

#include "types.h"

#include "Kyoto/TAverage.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

template < typename T >
T GetMaxValue(const T* values, int count) {
  const T* end = values + count;
  T maximum = *values++;
  for (; values < end; ++values) {
    if (maximum < *values) {
      maximum = *values;
    }
  }
  return maximum;
}

template < typename T, int N >
class TReservedAverage : public rstl::reserved_vector< T, N > {
public:
  TReservedAverage() {}
  TReservedAverage(const T& value) { this->resize(N, value); }
  void AddValue(const T& value) {
    if (this->size() < N) {
      this->push_back(value);
    }
    for (int i = this->size() - 1; i > 0; --i) {
      this->operator[](i) = this->operator[](i - 1);
    }
    this->operator[](0) = value;
  }
  rstl::optional_object< T > GetAverage() const {
    if (this->empty()) {
      return rstl::optional_object_null();
    }
    return GetAverageValue(this->data(), this->size());
  }
  rstl::optional_object< T > GetMax() const {
    if (this->empty()) {
      return rstl::optional_object_null();
    }

    return GetMaxValue(this->data(), this->size());
  }
  rstl::optional_object< T > GetEntry(int idx) const {
    if (idx >= this->size()) {
      return rstl::optional_object_null();
    } else {
      return rstl::optional_object< T >(this->operator[](idx));
    }
  }
};

#endif // _TRESERVEDAVERAGE
