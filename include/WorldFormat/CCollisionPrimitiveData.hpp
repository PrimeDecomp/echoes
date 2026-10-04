#ifndef _CCOLLISIONPRIMITIVEDATA
#define _CCOLLISIONPRIMITIVEDATA

#include "types.h"

class CCollisionEdge;
class CCollisionSurface;
class CTransform4f;
class CVector3f;

// Collision-array view with optional ownership of metadata/index buffers, not vertices.
// COBBTree owns its arrays through SIndexData instead.
class CCollisionPrimitiveData {
public:
  CCollisionPrimitiveData();
  CCollisionPrimitiveData(int materialCount, int vertexCount, int edgeCount, int triangleCount,
                          const u64* materials, const uchar* vertexMaterials,
                          const uchar* edgeMaterials, const uchar* surfaceMaterials,
                          const CCollisionEdge* edges, const ushort* surfaceIndices,
                          const ushort* extraIndices, const CVector3f* vertices, bool ownsArrays);
  ~CCollisionPrimitiveData();

  int GetTriangleCount() const { return mTriangleCount; }
  // Guessed name/qualifiers, correlated with the native triangle consumers.
  void GetTriangleVertexIndices(ushort index, ushort indices[3]) const;
  // GC uint formal is a compatible reconstruction; the callee uses a ushort triangle domain.
  // Wii exports use ushort instead. Neither version's signature proves the other.
  CCollisionSurface GetTriangle(uint index) const;
  CCollisionSurface GetTriangle(ushort index, const CTransform4f* xf) const;
  // Additional-flags overload is target-derived; this spelling is reconstructed.
  CCollisionSurface GetTriangle(ushort index, const CTransform4f* xf, u64 additionalFlags) const;
  const ushort* GetTriangleEdgeIndices(ushort index) const { return mSurfaceIndices + index * 3; }
  u64 GetVertMaterial(uint index) const { return mMaterials[mVertexMaterials[index]]; }
  u64 GetEdgeMaterial(uint index) const { return mMaterials[mEdgeMaterials[index]]; }

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

// Guessed scope/names for the geometry registration pool; IDs are not TUniqueId.
namespace CollisionPrimitiveDataCache {
extern uint gGeometryRevision;
ushort GetGeneration(ushort id);
} // namespace CollisionPrimitiveDataCache

#endif // _CCOLLISIONPRIMITIVEDATA
