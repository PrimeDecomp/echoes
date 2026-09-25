#ifndef _CVPARAMTRANSFER
#define _CVPARAMTRANSFER

#include "Kyoto/IObj.hpp"
#include <rstl/rc_ptr.hpp>

class IVParamObj : public IObj {};

template < typename T >
class TObjOwnerParam : public IVParamObj {
public:
  ~TObjOwnerParam() {}
  TObjOwnerParam(T data) : mData(data) {}
  T GetData() const { return static_cast<T>(mData); }

private:
  T mData;
};

class CVParamTransfer {
public:
  CVParamTransfer() {}
  template < typename T >
  CVParamTransfer(TObjOwnerParam< T >* obj) : mObj(obj) {}
  static CVParamTransfer Null();

  const IVParamObj& operator*() const { return *mObj; }
  const rstl::rc_ptr< IVParamObj >& GetObj() const { return mObj; }

private:
  rstl::rc_ptr< IVParamObj > mObj;
};

#endif // _CVPARAMTRANSFER
