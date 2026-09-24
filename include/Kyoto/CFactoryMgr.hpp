#ifndef _CFACTORYMGR
#define _CFACTORYMGR

#include "types.h"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"

class CFactoryMgr {
public:
  static uint FourCCToTypeIdx(uint fourCC);
  static uint TypeIdxToFourCC(uint typeIdx);
private:
  uchar pad[0x38];
};

class CFactoryFnReturn {
public:
  template < typename T >
  CFactoryFnReturn(T* ptr);

  const rstl::auto_ptr<CObjOwnerDerivedFromIObjUntyped>& GetObjForTransfer() const { return obj; }
private:
  rstl::auto_ptr< CObjOwnerDerivedFromIObjUntyped > obj;
};

template < typename T >
CFactoryFnReturn::CFactoryFnReturn(T* ptr)
: obj(TToken< T >::GetIObjObjectFor(ptr).release()) {}

CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);

#endif // _CFACTORYMGR
