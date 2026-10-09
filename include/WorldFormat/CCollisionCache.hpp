#ifndef _CCOLLISIONCACHE
#define _CCOLLISIONCACHE

#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/locked_cache_allocator.hpp"
#include "rstl/vector.hpp"

class CTransform4f;
class CCollisionPrimitiveData;
class CCollisionCacheIterator;
class CCachedCollisionSurface;

// Guessed name. This is the triangle payload, not the packed cache's full slot.
class CCachedCollisionSurface {
public:
  CCachedCollisionSurface(const CCollisionSurface& surface, ushort triangleIndex);

  const CCollisionSurface& GetSurface() const { return mSurface; }
  ushort GetTriangleIndex() const { return mTriangleIndex; }

private:
  CCollisionSurface mSurface;
  ushort mTriangleIndex;
};

// Guessed name. One packed cache slot: the triangle payload followed by a plane that is
// filled in the first time the leaf is visited.
struct SCachedCollisionSlot {
  CCachedCollisionSurface mTriangle;
  CPlane mPlane;
};

// Guessed name
class CCollisionCacheIterator {
  friend class CCollisionCache;

public:
  CCollisionCacheIterator();
  explicit CCollisionCacheIterator(CCollisionCache& cache);
  void Reset(); // Guessed names
  short GetObjectId() const { return mObjectId; }
  u64 GetMaterialFlags() const { return mMaterialFlags; }
  const CCollisionPrimitiveData& GetGeometry() const { return *mGeometry; }
  bool AtEnd() const { return mOffset >= mEnd; }
  bool AtLeafStart() const { return mLeafExhausted; }
  const CTransform4f* GetTransform() const { return mTransform; } // Guessed name
  bool MatchesGeometry(short id, const CCollisionPrimitiveData* geometry,
                       const CTransform4f& transform, u64 flags) const;

private:
  short mObjectId; // Guessed names
  const CTransform4f* mTransform;
  const CCollisionPrimitiveData* mGeometry;
  u64 mMaterialFlags;
  bool mLeafExhausted : 1;
  bool mLeafStatusZero : 1;
  const CAABox* mLeafBounds;
  uint mOffset;
  uint mTrianglesRemaining;
  uint mLeavesRemaining;
  uint mLeafTriangleCount;
  ushort* mLeafStatus;
  uchar x34_unknown[4];
  uint mEnd;
  int mGeometryTriangleCount;
};
CHECK_SIZEOF(CCollisionCacheIterator, 0x40)

// Guessed name
class CCollisionCache {
  friend class CCollisionCacheWriter;
  friend class CCollisionCacheIterator;

public:
  CCollisionCache(const CAABox& bounds, int x2c, int x30, ushort x34);
  void Reset(); // Guessed names
  void SetBounds(const CAABox& bounds);
  const CAABox& GetBounds() const { return mBounds; }
  uint GetNumTriangles() const {
    return uint(mData.size() - mMetadataWords) / (sizeof(SCachedCollisionSlot) / sizeof(ushort));
  }
  int GetDynamicGeometryMode() const { return mDynamicGeometryMode; }
  TUniqueId GetOwnerId() const { return TUniqueId(mOwnerId); }
  void RemoveGeometry(CCollisionCacheIterator& iterator);
  uint SkipGeometry(CCollisionCacheIterator& iterator);
  uint ReadGeometry(CCollisionCacheIterator& iterator);
  void ReadLeaf(CCollisionCacheIterator& iterator);
  // Guessed names. Weak copies are emitted in CSurfaceAlignmentHelper.
  const CCachedCollisionSurface* NextTriangle(CCollisionCacheIterator& iterator) const {
    if (iterator.mTrianglesRemaining != 0) {
      const CCachedCollisionSurface* surface =
          reinterpret_cast< const CCachedCollisionSurface* >(&mData[iterator.mOffset]);
      --iterator.mTrianglesRemaining;
      iterator.mLeafExhausted = iterator.mTrianglesRemaining == 0;
      iterator.mOffset += GetTriangleStride();
      return surface;
    }
    // The existing reader declarations are nonconst, but only advance the iterator.
    CCollisionCache* cache = const_cast< CCollisionCache* >(this);
    if (iterator.mLeavesRemaining == 0) {
      cache->ReadGeometry(iterator);
    }
    --iterator.mLeavesRemaining;
    cache->ReadLeaf(iterator);
    return NextTriangle(iterator);
  }
  // Weak copies are emitted in CMorphBall.
  SCachedCollisionSlot* NextTriangle(CCollisionCacheIterator& iterator) {
    if (iterator.mTrianglesRemaining != 0) {
      SCachedCollisionSlot* slot =
          reinterpret_cast< SCachedCollisionSlot* >(mData.data() + iterator.mOffset);
      --iterator.mTrianglesRemaining;
      iterator.mLeafExhausted = iterator.mTrianglesRemaining == 0;
      iterator.mOffset += sizeof(SCachedCollisionSlot) / sizeof(ushort);
      if (iterator.mLeafStatusZero) {
        const CCollisionSurface& surface = slot->mTriangle.GetSurface();
        slot->mPlane = CPlane(surface.GetVert(0), surface.GetVert(1), surface.GetVert(2));
        --iterator.mLeafTriangleCount;
        if (iterator.mLeafTriangleCount == 0) {
          *iterator.mLeafStatus = 1;
        }
      }
      return slot;
    }
    if (iterator.mLeavesRemaining == 0) {
      ReadGeometry(iterator);
    }
    --iterator.mLeavesRemaining;
    ReadLeaf(iterator);
    return NextTriangle(iterator);
  }
  const CAABox* GetLeafBounds(CCollisionCacheIterator& iterator) {
    if (iterator.mTrianglesRemaining != 0) {
      return iterator.mLeafBounds;
    }
    if (iterator.mLeavesRemaining == 0) {
      ReadGeometry(iterator);
    }
    --iterator.mLeavesRemaining;
    ReadLeaf(iterator);
    return GetLeafBounds(iterator);
  }
  void SkipLeaf(CCollisionCacheIterator& iterator) {
    iterator.mOffset +=
        iterator.mTrianglesRemaining * (sizeof(SCachedCollisionSlot) / sizeof(ushort));
    iterator.mTrianglesRemaining = 0;
    iterator.mLeafExhausted = true;
  }
  int GetTriangleStride() const { return sizeof(SCachedCollisionSlot) / sizeof(ushort); }

private:
  CAABox mBounds; // Guessed names
  rstl::vector< ushort, rstl::locked_cache_allocator > mData;
  int mMetadataWords;
  int mDynamicGeometryMode; // Guessed role; the full mode domain remains unresolved.
  int x30_unknown;
  ushort mOwnerId; // Packed TUniqueId storage; constructor API remains ushort.
  uint mGeometryRevision;
};
CHECK_SIZEOF(CCollisionCache, 0x3c)

// Guessed name
class CCollisionCacheWriter {
public:
  explicit CCollisionCacheWriter(CCollisionCache& cache);
  const CAABox& GetBounds() const { return mCache.GetBounds(); }
  ~CCollisionCacheWriter();
  void AddTriangle(const CCollisionSurface& surface, ushort triangleIndex);
  void ReserveTriangles(int count);
  void FinishLeaf(); // Guessed names
  void BeginLeaf(const CAABox& bounds);
  void ReserveWords(int count);
  void BeginGeometry(const CCollisionPrimitiveData& geometry, const CTransform4f* transform,
                     ushort id, u64 flags);

private:
  CCollisionCache& mCache;
  int mGeometryStart;
  int mLeafStart;
  ushort* mLeafCount;
  ushort* mTriangleCount;
  CAABox* mLeafBounds;
};
CHECK_SIZEOF(CCollisionCacheWriter, 0x18)

#endif // _CCOLLISIONCACHE
