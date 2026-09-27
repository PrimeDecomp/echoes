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
  CCollisionPrimitiveData(int materialCount, int vertexCount, int edgeCount, int triangleCount,
                          const u64* materials, const uchar* vertexMaterials,
                          const uchar* edgeMaterials, const uchar* surfaceMaterials,
                          const CCollisionEdge* edges, const ushort* surfaceIndices,
                          const ushort* extraIndices, const CVector3f* vertices, bool ownsArrays);
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
