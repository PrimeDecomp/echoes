#ifndef _CCOLLISIONPRIMITIVEDATA
#define _CCOLLISIONPRIMITIVEDATA

#include "types.h"

class CCollisionEdge;
class CCollisionSurface;
class CTransform4f;
class CVector3f;

// Shared collision-array view. COBBTree owns its arrays through SIndexData instead.
class CCollisionPrimitiveData {
public:
  CCollisionPrimitiveData();
  ~CCollisionPrimitiveData();

  CCollisionSurface GetTriangle(ushort index) const;
  CCollisionSurface GetTriangle(ushort index, const CTransform4f* xf) const;

protected:
  int mMaterialCount;
  int mVertexCount;
  int mEdgeCount;
  int mTriangleCount;
  const u64* mMaterials;
  const uchar* mVertexMaterials;
  const uchar* mEdgeMaterials;
  const uchar* mSurfaceMaterials;
  const CCollisionEdge* mEdges;
  const ushort* mSurfaceIndices;
  const ushort* x28_; // Additional serialized index array; role unresolved.
  const CVector3f* mVertices;
  ushort mCacheId;
  bool mOwnsArrays : 1;
};
CHECK_SIZEOF(CCollisionPrimitiveData, 0x34)

#endif // _CCOLLISIONPRIMITIVEDATA
