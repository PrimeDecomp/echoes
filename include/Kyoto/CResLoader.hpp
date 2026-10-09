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
class CLookaheadRes;
class CResLoader;

// Echoes-only async read returned by CResLoader::LoadResourceAsync; it may own its destination
// buffer or point into a CLookaheadRes. Name is a guess; methods live in the TU at 0x8034317C
// (after CInputStream.cpp).
class CBufferedDvdRequest : public CDvdRequest {
public:
  CBufferedDvdRequest(CLookaheadRes* cache, CDvdRequest* request, uchar* buffer);
  ~CBufferedDvdRequest();
  void WaitUntilComplete();
  bool IsComplete();
  void PostCancelRequest();
  int GetMediaType() const;

  bool OwnsBuffer() const { return mBuffer.owner(); }
  uchar* GetBufferPtr() const { return mBuffer.get(); }
  rstl::auto_ptr< uchar >& GetBuffer();

private:
  CLookaheadRes* mCache;
  rstl::auto_ptr< CDvdRequest > mRequest;
  rstl::auto_ptr< uchar > mBuffer;
  int mMediaType;
};
CHECK_SIZEOF(CBufferedDvdRequest, 0x1c)

// Echoes-only shared read of a run of grouped pak resources; requests for resources inside it
// are served from its buffer. Name from a Corruption diagnostic; methods live in the same TU as
// CBufferedDvdRequest.
class CLookaheadRes {
public:
  CLookaheadRes(uchar* buffer, CDvdFile* file, uint offset, uint size,
                const rstl::auto_ptr< CDvdRequest >& request, CResLoader* owner);
  ~CLookaheadRes();

  bool IsInvalid() const { return mInvalid; }
  bool Contains(const CDvdFile* file, uint offset, uint size) const;
  rstl::auto_ptr< CBufferedDvdRequest > MakeRequest(uint offset);
  void RequestHasDied();
  bool Cancel();

private:
  rstl::auto_ptr< uchar > mBuffer;
  CDvdFile* mFile;
  uint mOffset;
  uint mSize;
  uint mReferenceCount : 16;
  uint mInvalid : 1;
  rstl::auto_ptr< CDvdRequest > mRequest;
  CResLoader* mOwner;
};
CHECK_SIZEOF(CLookaheadRes, 0x24)

class CResLoader {
public:
  typedef rstl::list< rstl::auto_ptr< CPakFile > > PakList;
  typedef rstl::list< CLookaheadRes > GroupCacheList;

  enum ECompressionType {
    kCompressionType_Uncompressed,
    kCompressionType_Compressed,
  };

  CResLoader();
  ~CResLoader();
  void MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak);
  bool CacheFromPak(const CPakFile& pak, CAssetId asset) const;
  bool CacheFromPakForLoad(CPakFile& pak, CAssetId asset);
  CPakFile* FindResourceForLoad(const SObjectTag& tag);
  CPakFile* FindResourceForLoad(CAssetId asset);
  CPakFile* FindResource(const SObjectTag& tag);
  void ClearCache();
  void AsyncIdlePakLoading();
  bool AreAllPaksLoaded() const;
  const SObjectTag* GetResourceIdByName(const char* name) const;
  FourCC GetResourceTypeById(CAssetId asset) const;
  bool ResourceExists(const SObjectTag& tag) const;
  uint ResourceSize(const SObjectTag& tag) const;
  uint GetResourceOffset(const SObjectTag& tag) const;
  ECompressionType GetResourceCompression(const SObjectTag& tag) const;
  CDvdRequest* LoadResourceAsync(const SObjectTag& tag, char* extBuf);
  CBufferedDvdRequest* LoadResourceAsync(const SObjectTag& tag);
  CDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, char* extBuf);
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, char* extBuf);
  CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf);
  void LoadMemResourceSync(const SObjectTag& tag, char** extBuf, int* len);
  void AddPakFileAsync(const rstl::string&, bool, bool);
  void RemovePakFile(const rstl::string&);
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;
  int GetPakCount() const;
  // Same body as GetPakFile; identity unknown.
  CPakFile* sub_802FBB64(int idx) const;
  CPakFile* GetPakFile(int idx) const;
  void KillLookahead(const CLookaheadRes* cache);

private:
  GroupCacheList::iterator FindGroupCache(const CDvdFile* file, uint offset, uint size);
  GroupCacheList::iterator AddGroupCache(CDvdFile* file, uint offset, uint size);

  GroupCacheList mGroupCaches;
  PakList mAramList;
  PakList mPakLoadedList;
  PakList mPakLoadingList;
  PakList::iterator mCurPak;
  mutable CAssetId mCachedResId;
  mutable const CPakFile::CResInfo* mCachedResInfo;
  bool mForwardSeek;
};
CHECK_SIZEOF(CResLoader, 0x70)

#endif // _CRESLOADER
