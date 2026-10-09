#ifndef _CPAKFILE
#define _CPAKFILE

#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/IObjectStore.hpp"

class CDvdRequest;
class CMemoryInStream;

class CPakFile {
public:
  enum EAsyncPhase { kAP_Warmup, kAP_InitialHeaderLoad, kAP_DataLoad, kAP_Loaded };

#pragma pack(push, 1)
  struct CResInfo {
    CAssetId mId;
    uchar mData[7];

    CResInfo(uint id, uint fourCC, uint offset, uint size, uint flags, uint groupedSize);
    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
    void SetLookaheadAfterResourceSize(uint size);
    uint GetLookaheadAfterResourceSize() const;
    CAssetId GetId() const { return mId; }
    bool operator<(const CResInfo& other) const { return mId < other.mId; }
  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak);
  ~CPakFile();

  CDvdFile& DvdFile() { return mFile; }
  const CDvdFile& GetDvdFile() const { return mFile; }
  void AsyncIdle();
  bool IsARAMPak() const { return mAramFile; }
  bool IsWorldPak() const { return mWorldPak; }
  bool IsStashedInARAM() const { return mStashedInARAM; }
  bool IsCompletelyLoaded() const { return mAsyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  void sub_80323554();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return mNameList; }
  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& GetStringToObjectList() const {
    return mNameList;
  }
  const SObjectTag* GetResIdByName(const char* name) const;
  const CResInfo* GetResInfo(uint id) const;
  const CResInfo* GetResInfoForLoadDirectionless(uint id);
  const CResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

private:
  void UpdateFakeStaticSize();
  void RebuildResourceLists(const rstl::vector< CResInfo >& sortedResources);
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
  int mResTableCount;
  int mFakeStaticSize;
  const void* mpARAMHeader;
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > mNameList;
  rstl::vector< CAssetId > mDepList;
  rstl::vector< CResInfo > mResInfoBuckets;
  rstl::vector< uint > mBucketOffsets;
  mutable int mCurrentSeek;
};
CHECK_SIZEOF(CPakFile, 0x9c)
NESTED_CHECK_SIZEOF(CPakFile, CResInfo, 0xb)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPakFile::CResInfo)
} // namespace rstl

#endif // _CPAKFILE
