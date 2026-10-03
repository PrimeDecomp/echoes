#ifndef _CCOLLISIONCACHE
#define _CCOLLISIONCACHE

#include "Kyoto/Math/CAABox.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/locked_cache_allocator.hpp"
#include "rstl/vector.hpp"

class CTransform4f;
class CCollisionPrimitiveData;
class CCollisionCacheIterator;

// Guessed name
class CCollisionCache {
public:
  CCollisionCache(const CAABox& bounds, int x2c, int x30, ushort x34);
  ~CCollisionCache();
  void Reset(); // Guessed names
  void SetBounds(const CAABox& bounds);
  void RemoveGeometry(CCollisionCacheIterator& iterator);
  uint SkipGeometry(CCollisionCacheIterator& iterator);
  uint ReadGeometry(CCollisionCacheIterator& iterator);
  void ReadLeaf(CCollisionCacheIterator& iterator);

private:
  CAABox mBounds; // Guessed names
  rstl::vector< ushort, rstl::locked_cache_allocator > mData;
  int mMetadataWords;
  int x2c_unknown;
  int x30_unknown;
  ushort x34_unknown;
  uint mGeometryRevision;
};
CHECK_SIZEOF(CCollisionCache, 0x3c)

// Guessed name
class CCollisionCacheIterator {
public:
  CCollisionCacheIterator();
  explicit CCollisionCacheIterator(CCollisionCache& cache);
  void Reset(); // Guessed names
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
  int mOffset;
  int mTrianglesRemaining;
  int mLeavesRemaining;
  int mLeafTriangleCount;
  const ushort* mLeafStatus;
  uchar x34_unknown[4];
  int mEnd;
  int mGeometryStart;
};
CHECK_SIZEOF(CCollisionCacheIterator, 0x40)

// Guessed name. This is the triangle payload, not the packed cache's full slot.
class CCachedCollisionSurface {
public:
  CCachedCollisionSurface(const CCollisionSurface& surface, ushort triangleIndex);

private:
  CCollisionSurface mSurface;
  ushort mTriangleIndex;
};

// Guessed name
class CCollisionCacheWriter {
public:
  explicit CCollisionCacheWriter(CCollisionCache& cache);
  ~CCollisionCacheWriter();
  void AddTriangle(const CCollisionSurface& surface, ushort triangleIndex);
  void ReserveTriangles(int count);
  void FinishLeaf(); // Guessed names
  void BeginLeaf(const CAABox& bounds);
  void ReserveWords(int count);
  void BeginGeometry(const CCollisionPrimitiveData& geometry, const CTransform4f* transform,
                     short id, u64 flags);

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
