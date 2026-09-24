#ifndef _CVPARAMTRANSFER
#define _CVPARAMTRANSFER

#include "Kyoto/IObj.hpp"
#include <rstl/rc_ptr.hpp>

class IVParamObj : public IObj {};

template < typename T >
class TObjOwnerParam : public IVParamObj {
public:
  ~TObjOwnerParam() {}
  TObjOwnerParam(T data) : x4_data(data) {}
  T GetData() const { return static_cast<T>(x4_data); }

private:
  T x4_data;
};

class CVParamTransfer {
public:
  CVParamTransfer() : x0_obj(nullptr), x4_refCount(&rstl::CRefData::sNull.x4_refCount) {
    ++*x4_refCount;
  }
  template < typename T >
  CVParamTransfer(TObjOwnerParam< T >* obj) : x0_obj(obj), x4_refCount(rs_new int(1)) {}
  CVParamTransfer(const CVParamTransfer& other)
  : x0_obj(other.x0_obj), x4_refCount(other.x4_refCount) {
    ++*x4_refCount;
  }
  ~CVParamTransfer() { ReleaseData(); }
  void ReleaseData();
  CVParamTransfer& operator=(const CVParamTransfer& other) {
    if (x4_refCount != other.x4_refCount) {
      if (--*x4_refCount <= 0) {
        delete x0_obj;
        delete x4_refCount;
      }
      x0_obj = other.x0_obj;
      x4_refCount = other.x4_refCount;
      ++*x4_refCount;
    }
    return *this;
  }
  static CVParamTransfer Null();

  const IVParamObj& operator*() const { return *x0_obj; }

private:
  IVParamObj* x0_obj;
  int* x4_refCount;
};

#endif // _CVPARAMTRANSFER
