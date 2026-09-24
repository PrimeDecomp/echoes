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
    CAssetId x0_id;
    uchar x4_data[7];

    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags, uint groupedSize);
    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
    void SetGroupedSize(uint size);
    uint GetGroupedSize() const;
    CAssetId GetId() const { return x0_id; }
    bool operator<(const SResInfo& other) const { return x0_id < other.x0_id; }
  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak);
  ~CPakFile();

  CDvdFile& DvdFile() { return x0_file; }
  const CDvdFile& GetDvdFile() const { return x0_file; }
  void AsyncIdle();
  bool IsWorldPak() const { return x28_26_worldPak; }
  bool IsCompletelyLoaded() const { return x2c_asyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  void sub_80323554();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return x58_nameList; }
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

  CDvdFile x0_file;
  bool x28_24_buildDepList : 1;
  bool x28_25_aramFile : 1;
  bool x28_26_worldPak : 1;
  bool x28_27_stashedInARAM : 1;
  EAsyncPhase x2c_asyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > x30_dvdReq;
  rstl::vector< uchar > x38_headerData;
  uint x48_resTableOffset;
  uint x4c_resTableCount;
  int x50_fakeStaticSize;
  const void* x54_aramBase;
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > x58_nameList;
  rstl::vector< CAssetId > x68_depList;
  rstl::vector< SResInfo > x78_resList;
  rstl::vector< uint > x88_bucketOffsets;
  mutable int x98_currentSeek;
};
CHECK_SIZEOF(CPakFile, 0x9c)
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, 0xb)

#endif // _CPAKFILE
