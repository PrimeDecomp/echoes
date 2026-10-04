#ifndef _RSTL_PRERESERVED_VECTOR
#define _RSTL_PRERESERVED_VECTOR

#include "types.h"

namespace rstl {

template < typename T >
class prereserved_vector {
public:
  prereserved_vector() : mSize(0), mData(nullptr) {}
  int size() const { return mSize; }
  T* data() { return mData; }
  const T* data() const { return mData; }
  void set_size(int size) { mSize = size; }
  void set_data(T* data) { mData = data; }
  T& operator[](int idx) { return mData[idx]; }
  const T& operator[](int idx) const { return mData[idx]; }
  T& back() { return mData[mSize - 1]; }
  const T& back() const { return mData[mSize - 1]; }

private:
  int mSize;
  T* mData;
};

} // namespace rstl

#endif // _RSTL_PRERESERVED_VECTOR
