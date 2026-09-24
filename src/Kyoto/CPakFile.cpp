#include "Kyoto/CPakFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

#include <stdio.h>

CPakFile::SResInfo::SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags,
                            uint groupedSize)
: x0_id(id) {
  const uint typeIdx = CFactoryMgr::FourCCToTypeIdx(fourCC);
  x4_data[0] = static_cast< uchar >(typeIdx | (flags != 0 ? 0x80 : 0));
  x4_data[1] = static_cast< uchar >(offset >> 5);
  x4_data[2] = static_cast< uchar >(offset >> 13);
  x4_data[3] = static_cast< uchar >((offset >> 21) | ((size << 2) & 0x80));
  x4_data[4] = static_cast< uchar >(size >> 6);
  x4_data[5] = static_cast< uchar >(size >> 14);
  x4_data[6] = static_cast< uchar >(groupedSize >> 5);
}

uint CPakFile::SResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(x4_data[0] & 0x7f); }

uint CPakFile::SResInfo::GetOffset() const {
  return ((x4_data[1] | (x4_data[2] << 8) | (x4_data[3] << 16)) & 0x7fffff) << 5;
}

uint CPakFile::SResInfo::GetSize() const {
  return ((x4_data[3] >> 7) | (x4_data[4] << 1) | (x4_data[5] << 9)) << 5;
}

bool CPakFile::SResInfo::IsCompressed() const { return (x4_data[0] & ~0x7f) != 0; }

void CPakFile::SResInfo::SetGroupedSize(uint size) { x4_data[6] = static_cast< uchar >(size >> 5); }

uint CPakFile::SResInfo::GetGroupedSize() const { return x4_data[6] << 5; }

CPakFile::CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak)
: x0_file(filename.data())
, x28_24_buildDepList(buildDepList)
, x28_25_aramFile(x0_file.IsARAMFile())
, x28_26_worldPak(worldPak)
, x28_27_stashedInARAM(false)
, x2c_asyncLoadPhase(kAP_Warmup)
, x48_resTableOffset(0)
, x4c_resTableCount(0)
, x50_fakeStaticSize(0)
, x54_aramBase(CARAMManager::GetInvalidAlloc())
, x98_currentSeek(-1) {}

CPakFile::~CPakFile() {
  while (x2c_asyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }
  CMemory::OffsetFakeStatics(-x50_fakeStaticSize);
  CARAMManager::Free(x54_aramBase);
}

uint CPakFile::GetFakeStaticSize() const {
  return x4c_resTableCount * sizeof(SResInfo) + x68_depList.size() * sizeof(CAssetId) +
         x78_resList.size() * sizeof(SResInfo) + x88_bucketOffsets.size() * sizeof(uint);
}

void CPakFile::UpdateFakeStaticSize() {
  const int newSize = GetFakeStaticSize();
  CMemory::OffsetFakeStatics(newSize - x50_fakeStaticSize);
  x50_fakeStaticSize = newSize;
}

void CPakFile::AsyncIdle() {
  if (x2c_asyncLoadPhase != kAP_Loaded && x0_file.IsARAMFileLoaded() &&
      (x30_dvdReq.null() || x30_dvdReq->IsComplete())) {
    switch (x2c_asyncLoadPhase) {
    case kAP_Warmup:
      Warmup();
      break;
    case kAP_InitialHeaderLoad:
      InitialHeaderLoad();
      break;
    case kAP_DataLoad:
      DataLoad();
      break;
    default:
      break;
    }
  }
}

void CPakFile::Warmup() {
  const int length = rstl::min_val< int >(x0_file.Length(), 8192);
  x38_headerData.resize(length);
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >(x0_file.SyncRead(x38_headerData.data(), length));
  x2c_asyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream in(x38_headerData.data(), x38_headerData.size());
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >();

  const int version = in.ReadInt32();
  if (version != 0x30005) {
    char buf[248];
    sprintf(buf, "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            x0_file.GetFilename().data(), 0x30005, version);
    return;
  }

  in.ReadInt32();
  const int nameCount = in.ReadInt32();
  x58_nameList.reserve(nameCount);
  for (int i = 0; i < nameCount; ++i) {
    const FourCC type = in.ReadInt32();
    const CAssetId id = in.ReadInt32();
    const rstl::string name = CStringExtras::ReadString(in);
    x58_nameList.push_back_unsafe(
        rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
  }

  x4c_resTableCount = in.ReadInt32();
  x48_resTableOffset = in.GetReadPosition();
  x2c_asyncLoadPhase = kAP_DataLoad;

  const int oldSize = x38_headerData.size();
  const uint resourceBytes = x4c_resTableCount * 20;
  const int newSize = (resourceBytes + x48_resTableOffset + 31) & ~31;
  if (newSize > oldSize) {
    x38_headerData.resize(newSize);
    x30_dvdReq = rstl::auto_ptr< CDvdRequest >(x0_file.AsyncSeekRead(
        x38_headerData.data() + oldSize, x38_headerData.size() - oldSize, kSO_Set, oldSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  x30_dvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&x38_headerData[x48_resTableOffset],
                     x38_headerData.size() - x48_resTableOffset);
  LoadResourceTable(in);
  x2c_asyncLoadPhase = kAP_Loaded;

  if (x28_26_worldPak) {
    const uint size = (x4c_resTableCount * sizeof(SResInfo) + 31) & ~31;
    x54_aramBase = CARAMManager::Alloc(size);
    const uint handle = CARAMManager::DMAToARAM(x78_resList.data(),
                                                 const_cast< void* >(x54_aramBase), size,
                                                 CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(handle);
  }

  x38_headerData = rstl::vector< uchar >();
  UpdateFakeStaticSize();
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
  rstl::vector< SResInfo > sortedResources;
  sortedResources.reserve(x4c_resTableCount);
  if (x28_24_buildDepList)
    x68_depList.reserve(x4c_resTableCount);

  for (int i = 0; i < x4c_resTableCount; ++i) {
    const uint flags = in.ReadInt32();
    const uint type = in.ReadInt32();
    const uint id = in.ReadInt32();
    const uint size = in.ReadInt32();
    const uint offset = in.ReadInt32();
    sortedResources.push_back_unsafe(SResInfo(id, type, offset, size, flags, 0));
    if (x28_24_buildDepList)
      x68_depList.push_back_unsafe(id);
  }

  for (int i = 0; i < x4c_resTableCount; ++i) {
    if (sortedResources[i].GetSize() <= 8192) {
      uint groupedSize = 0;
      for (int j = i + 1; j < x4c_resTableCount &&
                          groupedSize + sortedResources[j].GetSize() < 8192;
           ++j) {
        groupedSize += sortedResources[j].GetSize();
      }
      sortedResources[i].SetGroupedSize(groupedSize);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(sortedResources.begin(), sortedResources.end(), compare);
  RebuildResourceLists(sortedResources);
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const {
  if (!x28_27_stashedInARAM) {
    for (rstl::vector< rstl::pair< rstl::string, SObjectTag > >::const_iterator it =
             x58_nameList.begin();
         it != x58_nameList.end(); ++it) {
      const int cmp = CStringExtras::CompareCaseInsensitive(it->first, rstl::string_l(name));
      if (cmp == 0)
        return &it->second;
    }
  }
  return nullptr;
}

const CPakFile::SResInfo* CPakFile::GetResInfo(uint id) const {
  if (!IsCompletelyLoaded())
    return nullptr;
  if (x28_27_stashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::const_iterator first(x78_resList.data() + x88_bucketOffsets[bucket]);
  rstl::vector< SResInfo >::const_iterator last(x78_resList.data() + x88_bucketOffsets[bucket + 1]);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;
  return &*it;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id) {
  if (x28_27_stashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::iterator first(x78_resList.data() + x88_bucketOffsets[bucket]);
  rstl::vector< SResInfo >::iterator last(x78_resList.data() + x88_bucketOffsets[bucket + 1]);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;

  const SResInfo* best = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - x98_currentSeek));
  for (++it; it != last && it->GetId() == id; ++it) {
    const int delta = CMath::AbsI(static_cast< int >(it->GetOffset() - x98_currentSeek));
    if (delta < bestDelta) {
      best = &*it;
      bestDelta = delta;
    }
  }
  x98_currentSeek = best->GetOffset() + best->GetSize();
  return best;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id) {
  if (x28_27_stashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::iterator first(x78_resList.data() + x88_bucketOffsets[bucket]);
  rstl::vector< SResInfo >::iterator last(x78_resList.data() + x88_bucketOffsets[bucket + 1]);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;

  const SResInfo* best = &*it;
  int bestDelta = x98_currentSeek - static_cast< int >(it->GetOffset());
  for (++it; it != last && it->GetId() == id; ++it) {
    const int delta = x98_currentSeek - static_cast< int >(it->GetOffset());
    if ((bestDelta < 0 && (delta > 0 || delta > bestDelta)) ||
        (bestDelta >= 0 && delta > 0 && delta < bestDelta)) {
      best = &*it;
      bestDelta = delta;
    }
  }
  x98_currentSeek = best->GetOffset() + best->GetSize();
  return best;
}

void CPakFile::RebuildResourceLists(const rstl::vector< SResInfo >& sortedResources) {
  rstl::reserved_vector< uint, 256 > bucketCounts(0);

  const SResInfo emptyInfo(0, 'TXTR', 0, 0, 0, 0);
  x78_resList.clear();
  x78_resList.resize(x4c_resTableCount, emptyInfo);
  x88_bucketOffsets.clear();
  x88_bucketOffsets.reserve(257);
  for (rstl::vector< SResInfo >::const_iterator it = sortedResources.begin();
       it != sortedResources.end(); ++it)
    ++bucketCounts[it->GetId() & 0xff];
  x88_bucketOffsets.push_back_unsafe(0);
  uint offset = 0;
  for (uint i = 0; i < 256; ++i) {
    offset += bucketCounts[i];
    x88_bucketOffsets.push_back_unsafe(offset);
    bucketCounts[i] = 0;
  }

  for (int i = 0; i < sortedResources.size(); ++i) {
    const SResInfo& info = sortedResources[i];
    const uint bucket = info.GetId() & 0xff;
    x78_resList[x88_bucketOffsets[bucket] + bucketCounts[bucket]] = info;
    ++bucketCounts[bucket];
  }
}

void CPakFile::EnsureWorldPakReady() {
  if (x28_26_worldPak && x28_27_stashedInARAM) {
    rstl::vector< SResInfo > resources(x4c_resTableCount);
    const uint size = (x4c_resTableCount * sizeof(SResInfo) + 31) & ~31;
    CARAMManager::WaitForDMACompletion(
        CARAMManager::DMAToMRAM(const_cast< void* >(x54_aramBase), resources.data(), size,
                                CARAMManager::kDMAPrio_One));
    RebuildResourceLists(resources);
    if (x28_24_buildDepList) {
      x68_depList.reserve(x4c_resTableCount);
      const SResInfo* info = resources.data();
      for (int i = 0; i < x4c_resTableCount; ++i, ++info)
        x68_depList.push_back_unsafe(info->GetId());
    }
    x28_27_stashedInARAM = false;
    UpdateFakeStaticSize();
  }
}

void CPakFile::sub_80323554() {
  if (x28_26_worldPak) {
    x28_27_stashedInARAM = true;
    x68_depList = rstl::vector< CAssetId >();
    x78_resList = rstl::vector< SResInfo >();
    x88_bucketOffsets = rstl::vector< uint >();
    UpdateFakeStaticSize();
  }
}
