#ifndef _CCOLLISIONCACHE
#define _CCOLLISIONCACHE

#include "WorldFormat/CCollisionSurface.hpp"

class CAABox;
class CCollisionCache; // Guessed name; Echoes's packed geometry cache.

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
