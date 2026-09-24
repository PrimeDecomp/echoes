#ifndef _RSTL_RC_PTR
#define _RSTL_RC_PTR

#include "types.h"
#include "Kyoto/Alloc/CMemory.hpp"

namespace rstl {
class CRefData {
public:
  CRefData() : x0_refCount(0) {}
  int AddRef() { return ++x0_refCount; }
  int DelRef() { return --x0_refCount; }

  int x0_refCount;

  static CRefData sNull;
};

template < typename T >
class rc_ptr {
public:
  rc_ptr() : x0_ptr(nullptr), x4_refCount(&CRefData::sNull.x0_refCount) { ++*x4_refCount; }
  rc_ptr(const T* ptr) : x0_ptr(ptr), x4_refCount(rs_new int(1)) {}
  rc_ptr(const rc_ptr& other) : x0_ptr(other.x0_ptr), x4_refCount(other.x4_refCount) {
    ++*x4_refCount;
  }
  ~rc_ptr() { ReleaseData(); }
  rc_ptr& operator=(const rc_ptr& other) {
    if (x4_refCount != other.x4_refCount) {
      ReleaseData();
      x0_ptr = other.x0_ptr;
      x4_refCount = other.x4_refCount;
      ++*x4_refCount;
    }
    return *this;
  }
  T* GetPtr() const { return const_cast< T* >(x0_ptr); }
  bool IsNull() const { return GetPtr() == nullptr; }
  template < typename U >
  void Assign(const U* ptr) {
    const T* base = ptr;
    ReleaseData();
    x0_ptr = base;
    x4_refCount = rs_new int(1);
  }
  void ReleaseData();
  void reset() {
    ReleaseData();
    x0_ptr = nullptr;
    x4_refCount = &CRefData::sNull.x0_refCount;
    ++*x4_refCount;
  }
  T* operator->() const { return GetPtr(); }
  T& operator*() const { return *GetPtr(); }
  operator bool() const { return GetPtr() != nullptr; }

private:
  const T* x0_ptr;
  int* x4_refCount;
};

template < typename T >
void rc_ptr< T >::ReleaseData() {
  if (--*x4_refCount <= 0) {
    delete GetPtr();
    delete x4_refCount;
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
