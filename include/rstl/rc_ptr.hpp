#ifndef _RSTL_RC_PTR
#define _RSTL_RC_PTR

#include "types.h"
#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
class CRefData {
public:
  CRefData() : mRefCount(0) {}
  int AddRef() { return ++mRefCount; }
  int DelRef() { return --mRefCount; }

  int mRefCount;

  static CRefData sNull;
};

template < typename T >
class rc_ptr {
public:
  rc_ptr() : mPtr(nullptr), mRefCount(&CRefData::sNull.mRefCount) { ++*mRefCount; }
  rc_ptr(const T* ptr) : mPtr(ptr), mRefCount(rs_new int(1)) {}
  rc_ptr(const rc_ptr& other) : mPtr(other.mPtr), mRefCount(other.mRefCount) {
    ++*mRefCount;
  }
  ~rc_ptr() { ReleaseData(); }
  rc_ptr& operator=(const rc_ptr& other) {
    if (mRefCount != other.mRefCount) {
      ReleaseData();
      mPtr = other.mPtr;
      mRefCount = other.mRefCount;
      ++*mRefCount;
    }
    return *this;
  }
  T* GetPtr() const { return const_cast< T* >(mPtr); }
  bool IsNull() const { return GetPtr() == nullptr; }
  template < typename U >
  void Assign(const U* ptr) {
    const T* base = ptr;
    ReleaseData();
    mPtr = base;
    mRefCount = rs_new int(1);
  }
  void ReleaseData();
  void reset() {
    ReleaseData();
    mPtr = nullptr;
    mRefCount = &CRefData::sNull.mRefCount;
    ++*mRefCount;
  }
  T* operator->() const { return GetPtr(); }
  T& operator*() const { return *GetPtr(); }
  operator bool() const { return GetPtr() != nullptr; }

private:
  const T* mPtr;
  int* mRefCount;
};

template < typename T >
void rc_ptr< T >::ReleaseData() {
  if (--*mRefCount <= 0) {
    delete GetPtr();
    delete mRefCount;
  }
}

template < typename T >
class ncrc_ptr : public rc_ptr< T > {
public:
  ncrc_ptr() {}
  ncrc_ptr(T* ptr) : rc_ptr< T >(ptr) {}
  ncrc_ptr(const rc_ptr< T >& other) : rc_ptr< T >(other) {}
  ncrc_ptr& operator=(const rc_ptr< T >& other) {
    rc_ptr< T >::operator=(other);
    return *this;
  }
  template < typename U >
  ncrc_ptr& operator=(const U* ptr) {
    rc_ptr< T >::Assign(ptr);
    return *this;
  }
};

template < typename T >
bool operator==(const rc_ptr< T >& left, const rc_ptr< T >& right) {
  return left.GetPtr() == right.GetPtr();
}

} // namespace rstl

#endif // _RSTL_RC_PTR
