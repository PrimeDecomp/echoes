#include "WorldFormat/CCollisionPrimitiveData.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "WorldFormat/CCollisionEdge.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

#include <string.h>

namespace CollisionPrimitiveDataCache {
// Reconstructed registry names; occupancy and generation share one halfword.
enum ERegistryConstants { kRC_Size = 256, kRC_Occupied = 0x8000 };
static ushort sGenerations[kRC_Size];
uint gGeometryRevision = 1;
static ushort sLastId = 0xffff;
static ushort AllocateId();
} // namespace CollisionPrimitiveDataCache

namespace {
// Target-derived winding bit, distinct from Prime's older triangle flag.
const u64 kFlippedTriangle = 0x01000000;
} // namespace

CCollisionPrimitiveData::CCollisionPrimitiveData(
    int materialCount, int vertexCount, int edgeCount, int triangleCount, const u64* materials,
    const uchar* vertexMaterials, const uchar* edgeMaterials, const uchar* surfaceMaterials,
    const CCollisionEdge* edges, const ushort* surfaceIndices, const ushort* extraIndices,
    const CVector3f* vertices, bool ownsArrays)
: mMaterialCount(materialCount)
, mVertexCount(vertexCount)
, mEdgeCount(edgeCount)
, mTriangleCount(triangleCount)
, mMaterials(materials)
, mVertexMaterials(vertexMaterials)
, mEdgeMaterials(edgeMaterials)
, mSurfaceMaterials(surfaceMaterials)
, mEdges(edges)
, mSurfaceIndices(surfaceIndices)
, x28_(extraIndices)
, mVertices(vertices)
, mCacheId(CollisionPrimitiveDataCache::AllocateId())
, mOwnsArrays(ownsArrays) {}

CCollisionPrimitiveData::CCollisionPrimitiveData()
: mMaterialCount(0)
, mVertexCount(0)
, mEdgeCount(0)
, mTriangleCount(0)
, mMaterials(nullptr)
, mVertexMaterials(nullptr)
, mEdgeMaterials(nullptr)
, mSurfaceMaterials(nullptr)
, mEdges(nullptr)
, mSurfaceIndices(nullptr)
, x28_(nullptr)
, mVertices(nullptr)
, mCacheId(CollisionPrimitiveDataCache::AllocateId())
, mOwnsArrays(false) {}

CCollisionPrimitiveData::~CCollisionPrimitiveData() {
  if (mOwnsArrays) {
    CMemory::Free(mMaterials);
    CMemory::Free(mVertexMaterials);
    CMemory::Free(mEdgeMaterials);
    CMemory::Free(mSurfaceMaterials);
    CMemory::Free(mEdges);
    CMemory::Free(mSurfaceIndices);
    CMemory::Free(x28_);
  }

  CollisionPrimitiveDataCache::sGenerations[mCacheId] &=
      ~CollisionPrimitiveDataCache::kRC_Occupied;
  ++CollisionPrimitiveDataCache::gGeometryRevision;
}

CCollisionSurface CCollisionPrimitiveData::GetTriangle(uint index) const {
  const ushort triangleIndex = index;
  const u64 flags = mMaterials[mSurfaceMaterials[triangleIndex]];
  const int base = triangleIndex * 3;
  const CCollisionEdge& edge0 = GetTriangleEdge(mSurfaceIndices[base]);
  const CCollisionEdge& edge1 = GetTriangleEdge(mSurfaceIndices[base + 1]);
  if (flags & kFlippedTriangle) {
    return CCollisionSurface(mVertices[edge0.GetVertIndex2()], mVertices[edge0.GetVertIndex1()],
                             mVertices[edge1.GetVertIndex1()], flags);
  }
  return CCollisionSurface(mVertices[edge0.GetVertIndex1()], mVertices[edge0.GetVertIndex2()],
                           mVertices[edge1.GetVertIndex2()], flags);
}

CCollisionSurface CCollisionPrimitiveData::GetTriangle(ushort index, const CTransform4f* xf) const {
  const u64 flags = mMaterials[mSurfaceMaterials[index]];
  const int base = index * 3;
  const CCollisionEdge& edge0 = GetTriangleEdge(mSurfaceIndices[base]);
  const CCollisionEdge& edge1 = GetTriangleEdge(mSurfaceIndices[base + 1]);
  if (xf != nullptr) {
    if (flags & kFlippedTriangle) {
      return CCollisionSurface(*xf * mVertices[edge0.GetVertIndex2()],
                               *xf * mVertices[edge0.GetVertIndex1()],
                               *xf * mVertices[edge1.GetVertIndex1()], flags);
    }
    return CCollisionSurface(*xf * mVertices[edge0.GetVertIndex1()],
                             *xf * mVertices[edge0.GetVertIndex2()],
                             *xf * mVertices[edge1.GetVertIndex2()], flags);
  }

  if (flags & kFlippedTriangle) {
    return CCollisionSurface(mVertices[edge0.GetVertIndex2()], mVertices[edge0.GetVertIndex1()],
                             mVertices[edge1.GetVertIndex1()], flags);
  }
  return CCollisionSurface(mVertices[edge0.GetVertIndex1()], mVertices[edge0.GetVertIndex2()],
                           mVertices[edge1.GetVertIndex2()], flags);
}

CCollisionSurface CCollisionPrimitiveData::GetTriangle(ushort index, const CTransform4f* xf,
                                                       u64 additionalFlags) const {
  const u64 flags = mMaterials[mSurfaceMaterials[index]] | additionalFlags;
  const CCollisionEdge& edge0 = mEdges[mSurfaceIndices[index * 3]];
  const CCollisionEdge& edge1 = mEdges[mSurfaceIndices[index * 3 + 1]];

  if (xf != nullptr) {
    if (flags & kFlippedTriangle) {
      return CCollisionSurface(*xf * mVertices[edge0.GetVertIndex2()],
                               *xf * mVertices[edge0.GetVertIndex1()],
                               *xf * mVertices[edge1.GetVertIndex1()], flags);
    }
    return CCollisionSurface(*xf * mVertices[edge0.GetVertIndex1()],
                             *xf * mVertices[edge0.GetVertIndex2()],
                             *xf * mVertices[edge1.GetVertIndex2()], flags);
  }

  if (flags & kFlippedTriangle) {
    return CCollisionSurface(mVertices[edge0.GetVertIndex2()], mVertices[edge0.GetVertIndex1()],
                             mVertices[edge1.GetVertIndex1()], flags);
  }
  return CCollisionSurface(mVertices[edge0.GetVertIndex1()], mVertices[edge0.GetVertIndex2()],
                           mVertices[edge1.GetVertIndex2()], flags);
}

void CCollisionPrimitiveData::GetTriangleVertexIndices(ushort index, ushort indices[3]) const {
  const u64 flags = mMaterials[mSurfaceMaterials[index]];
  int start = index * 3;
  ushort edgeIndex0 = mSurfaceIndices[start];
  ushort edgeIndex1 = mSurfaceIndices[start + 1];
  const CCollisionEdge& edge0 = mEdges[edgeIndex0];
  const CCollisionEdge& edge1 = mEdges[edgeIndex1];

  if (flags & kFlippedTriangle) {
    indices[0] = edge0.GetVertIndex2();
    indices[1] = edge0.GetVertIndex1();
    indices[2] = edge1.GetVertIndex1();
  } else {
    indices[0] = edge0.GetVertIndex1();
    indices[1] = edge0.GetVertIndex2();
    indices[2] = edge1.GetVertIndex2();
  }
}

namespace CollisionPrimitiveDataCache {
ushort GetGeneration(ushort id) { return sGenerations[id]; }

static ushort AllocateId() {
  if (sLastId == 0xffff) {
    memset(sGenerations, 0, sizeof(sGenerations));
  }

  while (true) {
    ++sLastId;
    sLastId %= kRC_Size;
    if (sGenerations[sLastId] < kRC_Occupied) {
      break;
    }
  }

  ++sGenerations[sLastId];
  sGenerations[sLastId] |= kRC_Occupied;
  return sLastId;
}
} // namespace CollisionPrimitiveDataCache
