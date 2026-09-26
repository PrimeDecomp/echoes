#ifndef _CRESFACTORY
#define _CRESFACTORY

#include "types.h"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResLoader.hpp"

class IFactory {
public:
  virtual ~IFactory() {}
  virtual CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&) = 0;
  virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) = 0;
  virtual void CancelBuild(const SObjectTag&) = 0;
  virtual bool CanBuild(const SObjectTag&) = 0;
  virtual const SObjectTag* GetResourceIdByName(const char* name) const = 0;
  // TODO
};

class CResFactory : public IFactory {
public:
  CResFactory();

  ~CResFactory() {}
  CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&);
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**);
  void CancelBuild(const SObjectTag&);

  bool CanBuild(const SObjectTag& tag) { return mResLoader.ResourceExists(tag); }

  const SObjectTag* GetResourceIdByName(const char* name) const;

  uint ResourceSize(const SObjectTag& tag) const { return mResLoader.ResourceSize(tag); }

  void AsyncIdle(uint time, bool);

  CResLoader& GetResLoader() { return mResLoader; }
  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

private:
  CResLoader mResLoader;
  CFactoryMgr mFactoryMgr;
  uint x94_;
  uint x98_;
  uint x9c_;
  uint xa0_;
  uint xa4_;
  uint xa8_;
  uint xac_;
  rstl::list< unkptr > xb0_;
};
CHECK_SIZEOF(CResFactory, 0xc8);

extern CResFactory* gpResourceFactory;

#endif // _CRESFACTORY
