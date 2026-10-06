#include "WorldFormat/CCollisionCache.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "WorldFormat/CCollisionPrimitiveData.hpp"
#include <string.h>

namespace {
// Reconstructed command names for the native halfword stream.
enum ECacheCommand {
  kCC_Padding = 0xfff0,
  kCC_NoObject = 0xfff1,
  kCC_Object = 0xfff2,
  kCC_NoTransform = 0xfff3,
  kCC_Transform = 0xfff4,
  kCC_Geometry = 0xfff5,
  kCC_Material = 0xfff6,
  kCC_NoMaterial = 0xfff7
};
} // namespace

void CCollisionCacheWriter::FinishLeaf() {
  if (mTriangleCount != 0 && *mTriangleCount == 0) {
    mTriangleCount = 0;
    mLeafBounds = 0;
    --*mLeafCount;
    mCache.mMetadataWords -= mCache.mData.size() - mLeafStart;
    if (mLeafStart > mCache.mData.size()) {
      mCache.mData.reserve(mLeafStart);
    }
    // Packed halfwords have no destructors; do not zero-fill the retained payload.
    mCache.mData.mCount = mLeafStart;
    if (*mLeafCount == 0) {
      mLeafCount = 0;
      mCache.mMetadataWords -= mCache.mData.size() - mGeometryStart;
      if (mGeometryStart > mCache.mData.size()) {
        mCache.mData.reserve(mGeometryStart);
      }
      mCache.mData.mCount = mGeometryStart;
    }
  }
}

void CCollisionCacheWriter::BeginLeaf(const CAABox& bounds) {
  if (mTriangleCount != 0 && *mTriangleCount == 0) {
    *mLeafBounds = bounds;
    return;
  }
  mLeafStart = mCache.mData.size();
  ++*mLeafCount;
  ReserveWords(14);
  mCache.mData.push_back_unsafe(0);
  mTriangleCount = &mCache.mData.back();
  ++mCache.mMetadataWords;
  mCache.mData.push_back_unsafe(0);
  ++mCache.mMetadataWords;
  mLeafBounds = reinterpret_cast< CAABox* >(mCache.mData.data() + mCache.mData.size());
  const uint* words = reinterpret_cast< const uint* >(&bounds);
  for (int i = 0; i < 6; ++i) {
    mCache.mData.push_back_unsafe(0);
    mCache.mData.push_back_unsafe(0);
    *reinterpret_cast< uint* >(mCache.mData.data() + mCache.mData.size() - 2) = words[i];
  }
  mCache.mMetadataWords += 12;
}

void CCollisionCacheWriter::ReserveWords(int count) {
  int required = mCache.mData.size() + count;
  if (mCache.mData.capacity() < required) {
    uint oldAddress = reinterpret_cast< uint >(mCache.mData.data());
    int capacity = mCache.mData.capacity() * 2;
    mCache.mData.reserve(capacity > required ? capacity : required * 2);
    // Translate packed cursors as addresses, including the initial null cursors.
    // Integer subtraction avoids subtracting pointers to different allocations.
    int delta =
        int(reinterpret_cast< uint >(mCache.mData.data()) - oldAddress) / int(sizeof(ushort));
    mLeafCount =
        reinterpret_cast< ushort* >(reinterpret_cast< uint >(mLeafCount) + delta * sizeof(ushort));
    mTriangleCount = reinterpret_cast< ushort* >(reinterpret_cast< uint >(mTriangleCount) +
                                                 delta * sizeof(ushort));
    mLeafBounds =
        reinterpret_cast< CAABox* >(reinterpret_cast< uint >(mLeafBounds) + delta * sizeof(ushort));
  }
}

void CCollisionCacheWriter::BeginGeometry(const CCollisionPrimitiveData& geometry,
                                          const CTransform4f* transform, short id, u64 flags) {
  FinishLeaf();
  mGeometryStart = mCache.mData.size();
  ReserveWords(40);
  mCache.mData.push_back_unsafe(kCC_Geometry);
  mCache.mData.push_back_unsafe(0);
  mCache.mData.push_back_unsafe(0);
  *reinterpret_cast< const CCollisionPrimitiveData** >(mCache.mData.data() + mCache.mData.size() -
                                                       2) = &geometry;
  mCache.mData.push_back_unsafe(geometry.GetCacheId());
  mCache.mData.push_back_unsafe(CollisionPrimitiveDataCache::GetGeneration(geometry.GetCacheId()));
  mCache.mMetadataWords += 5;
  if (transform != 0) {
    if ((mCache.mData.size() & 1) == 0) {
      mCache.mData.push_back_unsafe(kCC_Padding);
      ++mCache.mMetadataWords;
    }
    mCache.mData.push_back_unsafe(kCC_Transform);
    const uint* words = reinterpret_cast< const uint* >(transform);
    for (int i = 0; i < 12; ++i) {
      mCache.mData.push_back_unsafe(0);
      mCache.mData.push_back_unsafe(0);
      *reinterpret_cast< uint* >(mCache.mData.data() + mCache.mData.size() - 2) = words[i];
    }
    mCache.mMetadataWords += 25;
  } else {
    mCache.mData.push_back_unsafe(kCC_NoTransform);
    ++mCache.mMetadataWords;
  }
  if (id != -1) {
    mCache.mData.push_back_unsafe(kCC_Object);
    mCache.mData.push_back_unsafe(id);
    mCache.mMetadataWords += 2;
  } else {
    mCache.mData.push_back_unsafe(kCC_NoObject);
    ++mCache.mMetadataWords;
  }
  if (flags != 0) {
    mCache.mData.push_back_unsafe(kCC_Material);
    for (int i = 0; i < 4; ++i) {
      mCache.mData.push_back_unsafe(0);
    }
    *reinterpret_cast< u64* >(mCache.mData.data() + mCache.mData.size() - 4) = flags;
    mCache.mMetadataWords += 5;
  } else {
    mCache.mData.push_back_unsafe(kCC_NoMaterial);
    ++mCache.mMetadataWords;
  }
  mCache.mData.push_back_unsafe(0);
  mLeafCount = &mCache.mData.back();
  ++mCache.mMetadataWords;
  if ((mCache.mData.size() & 1) != 0) {
    mCache.mData.push_back_unsafe(kCC_Padding);
    ++mCache.mMetadataWords;
  }
  mTriangleCount = 0;
  mLeafBounds = 0;
  BeginLeaf(CAABox::MakeMaxInvertedBox());
}

CCollisionCacheWriter::~CCollisionCacheWriter() { FinishLeaf(); }
CCollisionCacheWriter::CCollisionCacheWriter(CCollisionCache& cache)
: mCache(cache)
, mGeometryStart(0)
, mLeafStart(0)
, mLeafCount(0)
, mTriangleCount(0)
, mLeafBounds(0) {}

void CCollisionCache::RemoveGeometry(CCollisionCacheIterator& iterator) {
  uint removed = iterator.mOffset - iterator.mEnd;
  uint remaining = (mData.size() - iterator.mOffset) * sizeof(ushort);
  if (remaining != 0) {
    memcpy(mData.data() + iterator.mEnd, mData.data() + iterator.mOffset, remaining);
  }
  int size = mData.size() - removed;
  if (size > mData.size()) {
    mData.reserve(size);
  }
  mData.mCount = size;
  mMetadataWords -=
      removed - iterator.mGeometryTriangleCount * (sizeof(SCachedCollisionSlot) / sizeof(ushort));
  iterator.mOffset = iterator.mEnd;
  iterator.mObjectId = -1;
  iterator.mTransform = 0;
  iterator.mGeometry = 0;
  iterator.mMaterialFlags = 0;
  iterator.mLeafBounds = 0;
  iterator.mTrianglesRemaining = 0;
  iterator.mLeavesRemaining = 0;
  iterator.mLeafExhausted = true;
}

uint CCollisionCache::SkipGeometry(CCollisionCacheIterator& iterator) {
  if (iterator.mOffset == mData.size()) {
    return uint(-1);
  }
  iterator.mGeometryTriangleCount = 0;
  iterator.mEnd = iterator.mOffset;
  uint id = ReadGeometry(iterator);
  while (iterator.mLeavesRemaining != 0) {
    ReadLeaf(iterator);
    iterator.mGeometryTriangleCount += iterator.mTrianglesRemaining;
    iterator.mOffset +=
        iterator.mTrianglesRemaining * (sizeof(SCachedCollisionSlot) / sizeof(ushort));
    iterator.mTrianglesRemaining = 0;
    --iterator.mLeavesRemaining;
  }
  return id;
}

uint CCollisionCache::ReadGeometry(CCollisionCacheIterator& iterator) {
  ushort* words = mData.data() + iterator.mOffset;
  uint id = 0;
  uint generation = 0;
  int commands = 4;
  do {
    ushort command = *words++;
    --commands;
    ++iterator.mOffset;
    switch (command) {
    case kCC_NoObject:
      iterator.mObjectId = -1;
      break;
    case kCC_Object:
      iterator.mObjectId = *words++;
      ++iterator.mOffset;
      break;
    case kCC_NoTransform:
      iterator.mTransform = 0;
      break;
    case kCC_Transform:
      iterator.mTransform = reinterpret_cast< const CTransform4f* >(words);
      words += 24;
      iterator.mOffset += 24;
      break;
    case kCC_Geometry:
      iterator.mGeometry = *reinterpret_cast< const CCollisionPrimitiveData** >(words);
      id = words[2];
      generation = words[3];
      words += 4;
      iterator.mOffset += 4;
      break;
    case kCC_Material:
      iterator.mMaterialFlags = *reinterpret_cast< u64* >(words);
      words += 4;
      iterator.mOffset += 4;
      break;
    case kCC_NoMaterial:
      iterator.mMaterialFlags = 0;
      break;
    }
  } while (commands != 0);
  iterator.mLeavesRemaining = mData[iterator.mOffset++];
  if ((iterator.mOffset & 1) != 0) {
    ++iterator.mOffset;
  }
  return id | (generation << 16);
}

void CCollisionCache::ReadLeaf(CCollisionCacheIterator& iterator) {
  iterator.mTrianglesRemaining = mData[iterator.mOffset];
  iterator.mLeafTriangleCount = iterator.mTrianglesRemaining;
  ++iterator.mOffset;
  iterator.mLeafStatus = mData.data() + iterator.mOffset;
  iterator.mLeafStatusZero = *iterator.mLeafStatus == 0;
  ++iterator.mOffset;
  iterator.mLeafBounds = reinterpret_cast< const CAABox* >(mData.data() + iterator.mOffset);
  iterator.mOffset += 12;
  iterator.mLeafExhausted = true;
}

void CCollisionCache::Reset() {
  mData.clear();
  mData = rstl::vector< ushort, rstl::locked_cache_allocator >(
      rstl::locked_cache_allocator(x30_unknown == 2 ? 2 : 0));
  mMetadataWords = 0;
  mData.reserve(x30_unknown == 2 ? 0x400 : x30_unknown == 0 ? 0x1000 : 0x800);
  mGeometryRevision = CollisionPrimitiveDataCache::gGeometryRevision;
}

void CCollisionCache::SetBounds(const CAABox& bounds) { mBounds = bounds; }

CCollisionCache::CCollisionCache(const CAABox& bounds, int x2c, int x30, ushort x34)
: mBounds(bounds)
, mData(rstl::vector< ushort, rstl::locked_cache_allocator >(
      rstl::locked_cache_allocator(x30 == 2 ? 2 : 0)))
, mMetadataWords(0)
, mDynamicGeometryMode(x2c)
, x30_unknown(x30)
, mOwnerId(x34)
, mGeometryRevision(CollisionPrimitiveDataCache::gGeometryRevision) {
  mData.reserve(x30_unknown == 2 ? 0x400 : x30_unknown == 0 ? 0x1000 : 0x800);
}

CCollisionCacheIterator::CCollisionCacheIterator() {
  Reset();
  mEnd = 0;
  mGeometryTriangleCount = 0;
}

CCollisionCacheIterator::CCollisionCacheIterator(CCollisionCache& cache) {
  Reset();
  mEnd = cache.mData.size();
  if (cache.mGeometryRevision != CollisionPrimitiveDataCache::gGeometryRevision) {
    CCollisionCacheIterator iterator;
    uint id;
    while ((id = cache.SkipGeometry(iterator)) != uint(-1)) {
      if (ushort(id >> 16) != CollisionPrimitiveDataCache::GetGeneration(ushort(id))) {
        cache.RemoveGeometry(iterator);
        mEnd = cache.mData.size();
      }
    }
    cache.mGeometryRevision = CollisionPrimitiveDataCache::gGeometryRevision;
  }
  if (cache.GetNumTriangles() != 0) {
    cache.ReadGeometry(*this);
    cache.ReadLeaf(*this);
    --mLeavesRemaining;
  }
}

bool CCollisionCacheIterator::MatchesGeometry(short id, const CCollisionPrimitiveData* geometry,
                                              const CTransform4f& transform, u64 flags) const {
  if (ushort(mObjectId) != ushort(id))
    return false;
  if (mGeometry != geometry)
    return false;
  if (mMaterialFlags != flags)
    return false;
  ConstMtxPtr current = transform.GetCStyleMatrix();
  for (int i = 0; i < 4; ++i) {
    ConstMtxPtr cached = mTransform->GetCStyleMatrix();
    if (!close_enough(CVector3f(current[0][i], current[1][i], current[2][i]),
                      CVector3f(cached[0][i], cached[1][i], cached[2][i]), 0.0001f)) {
      return false;
    }
  }
  return true;
}

void CCollisionCacheIterator::Reset() {
  mObjectId = -1;
  mTransform = 0;
  mGeometry = 0;
  mMaterialFlags = 0;
  mLeafExhausted = true;
  mLeafStatusZero = false;
  mLeafBounds = 0;
  mOffset = 0;
  mTrianglesRemaining = 0;
  mLeavesRemaining = 0;
  mLeafTriangleCount = 0;
  mLeafStatus = 0;
}

inline void rstl::locked_cache_allocator::Allocate(void*& out, uint size) {
  if (size == 0) {
    out = 0;
    return;
  }
  ++mAllocationCount;
  if (mPreferHeap) {
    if (mAllocationCount == 2)
      mPreviousHeapAllocation = true;
    else
      mHeapAllocation = true;
    out = CMemory::Alloc(size, IAllocator::kHI_RoundUpLen);
  } else {
    out = AllocateLockedCache32(size);
    if (mHeapAfterCacheAttempt)
      mPreferHeap = true;
    if (out == 0) {
      --mAllocationCount;
      mPreferHeap = true;
      Allocate(out, size);
    }
  }
}
