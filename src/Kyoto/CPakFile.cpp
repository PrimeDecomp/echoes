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
: mId(id) {
  const uint typeIdx = CFactoryMgr::FourCCToTypeIdx(fourCC);
  mData[0] = static_cast< uchar >(typeIdx | (flags != 0 ? 0x80 : 0));
  mData[1] = static_cast< uchar >(offset >> 5);
  mData[2] = static_cast< uchar >(offset >> 13);
  mData[3] = static_cast< uchar >((offset >> 21) | ((size << 2) & 0x80));
  mData[4] = static_cast< uchar >(size >> 6);
  mData[5] = static_cast< uchar >(size >> 14);
  mData[6] = static_cast< uchar >(groupedSize >> 5);
}

uint CPakFile::SResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(mData[0] & 0x7f); }

uint CPakFile::SResInfo::GetOffset() const {
  return ((mData[1] | (mData[2] << 8) | (mData[3] << 16)) & 0x7fffff) << 5;
}

uint CPakFile::SResInfo::GetSize() const {
  return ((mData[3] >> 7) | (mData[4] << 1) | (mData[5] << 9)) << 5;
}

bool CPakFile::SResInfo::IsCompressed() const { return (mData[0] & ~0x7f) != 0; }

void CPakFile::SResInfo::SetGroupedSize(uint size) { mData[6] = static_cast< uchar >(size >> 5); }

uint CPakFile::SResInfo::GetGroupedSize() const { return mData[6] << 5; }

CPakFile::CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak)
: mFile(filename.data())
, mBuildDepList(buildDepList)
, mAramFile(mFile.IsARAMFile())
, mWorldPak(worldPak)
, mStashedInARAM(false)
, mAsyncLoadPhase(kAP_Warmup)
, mResTableOffset(0)
, mResTableCount(0)
, mFakeStaticSize(0)
, mAramBase(CARAMManager::GetInvalidAlloc())
, mCurrentSeek(-1) {}

CPakFile::~CPakFile() {
  while (mAsyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }
  CMemory::OffsetFakeStatics(-mFakeStaticSize);
  CARAMManager::Free(mAramBase);
}

uint CPakFile::GetFakeStaticSize() const {
  return mResTableCount * sizeof(SResInfo) + mDepList.size() * sizeof(CAssetId) +
         mResList.size() * sizeof(SResInfo) + mBucketOffsets.size() * sizeof(uint);
}

void CPakFile::UpdateFakeStaticSize() {
  const int newSize = GetFakeStaticSize();
  CMemory::OffsetFakeStatics(newSize - mFakeStaticSize);
  mFakeStaticSize = newSize;
}

void CPakFile::AsyncIdle() {
  if (mAsyncLoadPhase != kAP_Loaded && mFile.IsARAMFileLoaded() &&
      (mDvdReq.null() || mDvdReq->IsComplete())) {
    switch (mAsyncLoadPhase) {
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
  const int length = rstl::min_val< int >(mFile.Length(), 8192);
  mHeaderData.resize(length);
  mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.SyncRead(mHeaderData.data(), length));
  mAsyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream in(mHeaderData.data(), mHeaderData.size());
  mDvdReq = rstl::auto_ptr< CDvdRequest >();

  const int version = in.ReadInt32();
  if (version != 0x30005) {
    char buf[248];
    sprintf(buf, "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            mFile.GetFilename().data(), 0x30005, version);
    return;
  }

  in.ReadInt32();
  const int nameCount = in.ReadInt32();
  mNameList.reserve(nameCount);
  for (int i = 0; i < nameCount; ++i) {
    const FourCC type = in.ReadInt32();
    const CAssetId id = in.ReadInt32();
    const rstl::string name = CStringExtras::ReadString(in);
    mNameList.push_back_unsafe(
        rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
  }

  mResTableCount = in.ReadInt32();
  mResTableOffset = in.GetReadPosition();
  mAsyncLoadPhase = kAP_DataLoad;

  const int oldSize = mHeaderData.size();
  const uint resourceBytes = mResTableCount * 20;
  const int newSize = (resourceBytes + mResTableOffset + 31) & ~31;
  if (newSize > oldSize) {
    mHeaderData.resize(newSize);
    mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.AsyncSeekRead(
        mHeaderData.data() + oldSize, mHeaderData.size() - oldSize, kSO_Set, oldSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  mDvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&mHeaderData[mResTableOffset],
                     mHeaderData.size() - mResTableOffset);
  LoadResourceTable(in);
  mAsyncLoadPhase = kAP_Loaded;

  if (mWorldPak) {
    const uint size = (mResTableCount * sizeof(SResInfo) + 31) & ~31;
    mAramBase = CARAMManager::Alloc(size);
    const uint handle = CARAMManager::DMAToARAM(mResList.data(),
                                                 const_cast< void* >(mAramBase), size,
                                                 CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(handle);
  }

  mHeaderData = rstl::vector< uchar >();
  UpdateFakeStaticSize();
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
  rstl::vector< SResInfo > sortedResources;
  sortedResources.reserve(mResTableCount);
  if (mBuildDepList)
    mDepList.reserve(mResTableCount);

  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    const uint flags = in.ReadInt32();
    const uint type = in.ReadInt32();
    const uint id = in.ReadInt32();
    const uint size = in.ReadInt32();
    const uint offset = in.ReadInt32();
    sortedResources.push_back_unsafe(SResInfo(id, type, offset, size, flags, 0));
    if (mBuildDepList)
      mDepList.push_back_unsafe(id);
  }

  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    SResInfo& info = sortedResources[i];
    if (info.GetSize() <= 8192) {
      uint groupedSize = 0;
      for (int j = i + 1; j < static_cast< int >(mResTableCount); ++j) {
        const uint nextSize = sortedResources[j].GetSize();
        if (groupedSize + nextSize >= 8192)
          break;
        groupedSize += nextSize;
      }
      info.SetGroupedSize(groupedSize);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(sortedResources.begin(), sortedResources.end(), compare);
  RebuildResourceLists(sortedResources);
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const {
  if (!mStashedInARAM) {
    for (rstl::vector< rstl::pair< rstl::string, SObjectTag > >::const_iterator it =
             mNameList.begin();
         it != mNameList.end(); ++it) {
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
  if (mStashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::const_iterator first =
      mResList.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo >::const_iterator last =
      mResList.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;
  return &*it;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id) {
  if (mStashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::const_iterator first =
      mResList.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo >::const_iterator last =
      mResList.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;

  const SResInfo* best = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
  ++it;
  while (it != last) {
    if (it->GetId() != id)
      break;
    const int delta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
    if (delta < bestDelta) {
      best = &*it;
      bestDelta = delta;
    }
    ++it;
  }
  mCurrentSeek = best->GetOffset() + best->GetSize();
  return best;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id) {
  if (mStashedInARAM)
    return nullptr;
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo >::const_iterator first =
      mResList.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo >::const_iterator last =
      mResList.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo(id, 'TXTR', 0, 0, 0, 0), compare);
  if (it == last || it->GetId() != id)
    return nullptr;

  const SResInfo* best = &*it;
  int bestDelta = mCurrentSeek - static_cast< int >(it->GetOffset());
  ++it;
  while (it != last) {
    if (it->GetId() != id)
      break;
    const int delta = mCurrentSeek - static_cast< int >(it->GetOffset());
    if ((bestDelta < 0 && (delta > 0 || delta > bestDelta)) ||
        (bestDelta >= 0 && delta > 0 && delta < bestDelta)) {
      best = &*it;
      bestDelta = delta;
    }
    ++it;
  }
  mCurrentSeek = best->GetOffset() + best->GetSize();
  return best;
}

void CPakFile::RebuildResourceLists(const rstl::vector< SResInfo >& sortedResources) {
  rstl::reserved_vector< uint, 256 > bucketCounts(0);

  const SResInfo emptyInfo(0, 'TXTR', 0, 0, 0, 0);
  mResList.clear();
  mResList.resize(mResTableCount, emptyInfo);
  mBucketOffsets.clear();
  mBucketOffsets.reserve(257);
  for (rstl::vector< SResInfo >::const_iterator it = sortedResources.begin();
       it != sortedResources.end(); ++it)
    ++bucketCounts[it->GetId() & 0xff];
  mBucketOffsets.push_back_unsafe(0);
  uint offset = 0;
  for (uint i = 0; i < 256; ++i) {
    offset += bucketCounts[i];
    mBucketOffsets.push_back_unsafe(offset);
    bucketCounts[i] = 0;
  }

  for (int i = 0; i < sortedResources.size(); ++i) {
    const SResInfo& info = sortedResources[i];
    const uint bucket = info.GetId() & 0xff;
    mResList[mBucketOffsets[bucket] + bucketCounts[bucket]] = info;
    ++bucketCounts[bucket];
  }
}

void CPakFile::EnsureWorldPakReady() {
  if (mWorldPak && mStashedInARAM) {
    rstl::vector< SResInfo > resources(mResTableCount);
    const uint size = (mResTableCount * sizeof(SResInfo) + 31) & ~31;
    CARAMManager::WaitForDMACompletion(
        CARAMManager::DMAToMRAM(const_cast< void* >(mAramBase), resources.data(), size,
                                CARAMManager::kDMAPrio_One));
    RebuildResourceLists(resources);
    if (mBuildDepList) {
      mDepList.reserve(mResTableCount);
      const SResInfo* info = resources.data();
      for (int i = 0; i < mResTableCount; ++i, ++info)
        mDepList.push_back_unsafe(info->GetId());
    }
    mStashedInARAM = false;
    UpdateFakeStaticSize();
  }
}

void CPakFile::sub_80323554() {
  if (mWorldPak) {
    mStashedInARAM = true;
    mDepList = rstl::vector< CAssetId >();
    mResList = rstl::vector< SResInfo >();
    mBucketOffsets = rstl::vector< uint >();
    UpdateFakeStaticSize();
  }
}
