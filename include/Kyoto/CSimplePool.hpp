#ifndef _CSIMPLEPOOL
#define _CSIMPLEPOOL

#include "types.h"

#include "rstl/map.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CToken.hpp"
#include "Kyoto/IObjectStore.hpp"

class IFactory;

class CSimplePool : public IObjectStore {
  struct TagIdLess {
    bool operator()(const SObjectTag& a, const SObjectTag& b) const { return a.id < b.id; }
  };
  typedef rstl::map< SObjectTag, CObjectReference*, TagIdLess > ResourceMap;

public:
  CSimplePool(IFactory& factory);
  ~CSimplePool();

  void DebugDumpPool() const;
  virtual CToken GetObj(const SObjectTag& tag, const CVParamTransfer& xfer);
  virtual CToken GetObj(const SObjectTag& tag);
  virtual CToken GetObj(const char* name);
  virtual CToken GetObj(const char* name, const CVParamTransfer& xfer);
  virtual bool HasObject(const SObjectTag& tag);
  virtual bool ObjectIsLive(const SObjectTag& tag);
  virtual IFactory& GetFactory() { return *mFactory; }
  virtual void Flush();
  virtual void ObjectUnreferenced(const SObjectTag& tag);
  rstl::vector< SObjectTag > GetReferencedTags();

  void fn_8029c7e8(const SObjectTag& tag);

private:
  ResourceMap mResources;
  IFactory* mFactory;
  CVParamTransfer mParamXfr;
};
CHECK_SIZEOF(CSimplePool, 0x24)

extern CSimplePool* gpSimplePool;

#endif // _CSIMPLEPOOL
