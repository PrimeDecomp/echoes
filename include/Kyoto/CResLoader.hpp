#ifndef _CRESLOADER
#define _CRESLOADER

#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/IObjectStore.hpp"

class CInputStream;

// Echoes-only async read returned by CResLoader::LoadResourceAsync; it may own its destination
// buffer. Name is a guess; methods live in the TU at 0x8034317C (after CInputStream.cpp).
class CBufferedDvdRequest : public CDvdRequest {
public:
  bool OwnsBuffer() const { return mBuffer.owner(); }
  uchar* GetBufferPtr() const { return mBuffer.get(); }
  rstl::auto_ptr< uchar >& GetBuffer();

private:
  void* x4_;
  rstl::auto_ptr< CDvdRequest > x8_;
  rstl::auto_ptr< uchar > mBuffer;
  int mMediaType;
};
CHECK_SIZEOF(CBufferedDvdRequest, 0x1c)

class CResLoader {
public:
  enum ECompressionType {
    kCompressionType_Uncompressed,
    kCompressionType_Compressed,
  };

  CResLoader();
  ~CResLoader();

  int GetPakCount() const;
  CPakFile& GetPakFile(int idx) const;
  void AddPakFileAsync(const rstl::string&, bool, bool);
  void AsyncIdlePakLoading();
  bool AreAllPaksLoaded() const;
  const SObjectTag* GetResourceIdByName(const char* name) const;
  FourCC GetResourceTypeById(CAssetId) const;
  uint ResourceSize(const SObjectTag& tag) const;
  bool ResourceExists(const SObjectTag& tag) const;
  ECompressionType GetResourceCompression(const SObjectTag& tag) const;
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, char* extBuf);
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, int, int, char* extBuf);
  void LoadMemResourceSync(const SObjectTag& tag, char** extBuf, int* len);
  CBufferedDvdRequest* LoadResourceAsync(const SObjectTag& tag);
  CARAMDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int, int, char*);

private:
  rstl::list< unkptr > x0_;
  rstl::list< unkptr > mAramList;
  rstl::list< unkptr > mPakLoadedList;
  rstl::list< unkptr > mPakLoadingList;
  rstl::list< unkptr >::iterator mCurPak;
  mutable CAssetId mCachedResId;
  mutable const CPakFile::SResInfo* mCachedResInfo;
  bool mForwardSeek;
};
CHECK_SIZEOF(CResLoader, 0x70)

#endif // _CRESLOADER
