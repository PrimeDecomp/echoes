#ifndef _CFACTORYMGR
#define _CFACTORYMGR

#include "types.h"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/map.hpp"

class CFactoryFnReturn {
public:
  template < typename T >
  CFactoryFnReturn(T* ptr) : obj(TToken< T >::GetIObjObjectFor(ptr).release()) {}

  const rstl::auto_ptr< IObj >& GetObjForTransfer() const { return obj; }

private:
  rstl::auto_ptr< IObj > obj;
};

typedef CFactoryFnReturn (*FFactoryFunc)(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer);
typedef CFactoryFnReturn (*FMemFactoryFunc)(const SObjectTag& tag,
                                            const rstl::auto_ptr< uchar >& buffer, int size,
                                            const CVParamTransfer& xfer);

class CFactoryMgr {
public:
  CFactoryMgr();
  ~CFactoryMgr();

  void AddFactory(FourCC type, FFactoryFunc factory);
  void AddFactory(FourCC type, FMemFactoryFunc factory);
  bool CanMakeMemory(const SObjectTag& tag) const;
  rstl::auto_ptr< IObj > MakeObject(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params);
  rstl::auto_ptr< IObj > MakeObjectFromMemory(const SObjectTag& tag,
                                              const rstl::auto_ptr< uchar >& buffer, int size,
                                              bool compressed, const CVParamTransfer& params);

  static uint FourCCToTypeIdx(uint fourCC);
  static uint TypeIdxToFourCC(uint typeIdx);

private:
  rstl::map< int, FFactoryFunc > mFactories;
  rstl::map< int, FMemFactoryFunc > mMemFactories;
};
CHECK_SIZEOF(CFactoryMgr, 0x28)

CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer);

CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer);

#endif // _CFACTORYMGR
