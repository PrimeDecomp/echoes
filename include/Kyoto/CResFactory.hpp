#ifndef _CRESFACTORY
#define _CRESFACTORY

#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/map.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CVParamTransfer.hpp"

class CDvdRequest;

class IFactory {
public:
  virtual ~IFactory() = 0;
  virtual rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) = 0;
  virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) = 0;
  virtual void CancelBuild(const SObjectTag&) = 0;
  virtual bool CanBuild(const SObjectTag&) = 0;
  virtual const SObjectTag* GetResourceIdByName(const char* name) const = 0;
};

inline IFactory::~IFactory() {}

class CResFactory : public IFactory {
public:
  struct SLoadingData {
    SObjectTag mTag;
    rstl::auto_ptr< CDvdRequest > mDvdReq;
    IObj** mTarget;
    rstl::auto_ptr< uchar > mBuffer;
    uint mCompressedPos;
    rstl::auto_ptr< uchar > mDecompBuffer;
    uint mDecompressedPos;
    int mSize;
    CResLoader::ECompressionType mCompression;
    CVParamTransfer mParams;

    SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                 const rstl::auto_ptr< uchar >& buffer, int size,
                 CResLoader::ECompressionType compression, const CVParamTransfer& params);
    ~SLoadingData();
    bool PumpDecompression(uint time);
  };

  CResFactory();
  ~CResFactory() override;
  rstl::auto_ptr< IObj > Build(const SObjectTag&, const CVParamTransfer&) override;
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, IObj**) override;
  void CancelBuild(const SObjectTag&) override;

  bool CanBuild(const SObjectTag& tag) override { return mResLoader.ResourceExists(tag); }

  const SObjectTag* GetResourceIdByName(const char* name) const override {
    return mResLoader.GetResourceIdByName(name);
  }

  uint ResourceSize(const SObjectTag& tag) const { return mResLoader.ResourceSize(tag); }

  void AsyncIdle(uint time, bool);

  CResLoader& GetResLoader() { return mResLoader; }
  CFactoryMgr& GetFactoryMgr() { return mFactoryMgr; }
  FourCC GetResourceTypeById(CAssetId id) { return GetResLoader().GetResourceTypeById(id); }
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;

private:
  typedef rstl::list< SLoadingData > LoadList;

  void AddToLoadList(const SLoadingData& data);
  void EraseFromLoadList(const LoadList::iterator& it);
  LoadList::iterator FindInLoadList(const SObjectTag& tag);
  rstl::auto_ptr< IObj > BuildSync(const SObjectTag& tag, const CVParamTransfer& params);
  bool PumpResource(const LoadList::iterator& it, uint time);

  CResLoader mResLoader;
  CFactoryMgr mFactoryMgr;
  LoadList mLoadList;
  rstl::map< SObjectTag, LoadList::iterator > mLoadMap;
  LoadList mCancelledList;
};
CHECK_SIZEOF(CResFactory, 0xe0)

extern CResFactory* gpResourceFactory;

#endif // _CRESFACTORY
