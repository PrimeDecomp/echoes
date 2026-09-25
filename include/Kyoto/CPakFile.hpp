#ifndef _CPAKFILE
#define _CPAKFILE

#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "rstl/pair.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/IObjectStore.hpp"

class CDvdRequest;
class CMemoryInStream;

class CPakFile {
public:
  enum EAsyncPhase { kAP_Warmup, kAP_InitialHeaderLoad, kAP_DataLoad, kAP_Loaded };

#pragma pack(push, 1)
  struct SResInfo {
    CAssetId mId;
    uchar mData[7];

    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags, uint groupedSize);
    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
    void SetGroupedSize(uint size);
    uint GetGroupedSize() const;
    CAssetId GetId() const { return mId; }
    bool operator<(const SResInfo& other) const { return mId < other.mId; }
  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak);
  ~CPakFile();

  CDvdFile& DvdFile() { return mFile; }
  const CDvdFile& GetDvdFile() const { return mFile; }
  void AsyncIdle();
  bool IsWorldPak() const { return mWorldPak; }
  bool IsCompletelyLoaded() const { return mAsyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  void sub_80323554();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return mNameList; }
  const SObjectTag* GetResIdByName(const char* name) const;
  const SResInfo* GetResInfo(uint id) const;
  const SResInfo* GetResInfoForLoadDirectionless(uint id);
  const SResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

private:
  void UpdateFakeStaticSize();
  void RebuildResourceLists(const rstl::vector< SResInfo >& sortedResources);
  void LoadResourceTable(CMemoryInStream& in);
  void Warmup();
  void InitialHeaderLoad();
  void DataLoad();

  CDvdFile mFile;
  bool mBuildDepList : 1;
  bool mAramFile : 1;
  bool mWorldPak : 1;
  bool mStashedInARAM : 1;
  EAsyncPhase mAsyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > mDvdReq;
  rstl::vector< uchar > mHeaderData;
  uint mResTableOffset;
  uint mResTableCount;
  int mFakeStaticSize;
  const void* mAramBase;
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > mNameList;
  rstl::vector< CAssetId > mDepList;
  rstl::vector< SResInfo > mResList;
  rstl::vector< uint > mBucketOffsets;
  mutable int mCurrentSeek;
};
CHECK_SIZEOF(CPakFile, 0x9c)
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, 0xb)

namespace rstl {
template <>
struct is_trivially_destructible< CPakFile::SResInfo > {
  enum { value = true };
};

template <>
inline void construct< CPakFile::SResInfo >(void* dest, const CPakFile::SResInfo& src) {
  *static_cast< CPakFile::SResInfo* >(dest) = src;
}
} // namespace rstl

#endif // _CPAKFILE
