// The native double square roots use the external MSL implementation.
#define MSL_NO_INLINE_SQRT
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/CCollisionCache.hpp"

#include "Collision/CMRay.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CMath.hpp"
#include <float.h>
#include <math.h>
#include <string.h>

static uint gCalledClip = 0;
static uint gRejectedByClip = 0;
static uint gTrianglesProcessed = 0;
static uint gDupTrianglesProcessed = 0;
uchar CMetroidAreaCollider::sDupPrimitiveCheckCount = 0xff;
ushort CMetroidAreaCollider::sDupVertexCount;
ushort CMetroidAreaCollider::sDupEdgeCount;
ushort CMetroidAreaCollider::sDupTriangleCount;
uchar* CMetroidAreaCollider::spDupVertexList;
uchar* CMetroidAreaCollider::spDupEdgeList;
uchar* CMetroidAreaCollider::spDupTriangleList;

static void FlagVertexIndicesForFace(uint face, bool* vertFlags);
static void FlagEdgeIndicesForFace(uint face, bool* edgeFlags);
static float PlaneIntersectionFraction(const CVector3f& start, const CVector3f& end,
                                       const CPlane& plane); // Guessed name

// Guessed name
static float PlaneIntersectionFraction(const CVector3f& start, const CVector3f& end,
                                       const CPlane& plane) {
  return -(CVector3f::Dot(start, plane.GetNormal()) - plane.GetConstant()) /
         CVector3f::Dot(end - start, plane.GetNormal());
}

bool CMetroidAreaCollider::ConvexPolyCollision(const CPlane* planes, const CVector3f* verts,
                                               CAABox& aabb) {
  typedef rstl::reserved_vector< CVector3f, 20 > ClipVec;
  ClipVec vecs[2];
  ++gCalledClip;
  ++gRejectedByClip;
  int vecIdx = 0;
  int otherVecIdx = 1;

  for (int i = 0; i < 3; ++i) {
    vecs[0].push_back(verts[i]);
  }

  for (int i = 0; i < 6; ++i) {
    ClipVec& vec = vecs[vecIdx];
    ClipVec& otherVec = vecs[otherVecIdx];
    otherVec.clear();

    bool inFrontOf = planes[i].GetHeight(vec.front()) >= 0.f;
    for (int j = 0; j < vec.size(); ++j) {
      const CVector3f& b = vec[j == vec.size() - 1 ? 0 : j + 1];
      if (inFrontOf) {
        otherVec.push_back(vec[j]);
      }
      bool nextInFrontOf = planes[i].GetHeight(b) >= 0.f;
      if (nextInFrontOf ^ inFrontOf) {
        float f = PlaneIntersectionFraction(vec[j], b, planes[i]);
        otherVec.push_back((1.f - f) * (vec[j] - b) + b);
      }
      inFrontOf = nextInFrontOf;
    }

    if (otherVec.empty()) {
      return false;
    }

    otherVecIdx ^= 1;
    vecIdx ^= 1;
  }

  ClipVec& accumVec = vecs[otherVecIdx ^ 1];
  for (ClipVec::const_iterator it = accumVec.begin(); it != accumVec.end(); ++it) {
    aabb.AccumulateBounds(*it);
  }

  --gRejectedByClip;
  return true;
}

// Guessed name
void CMetroidAreaCollider::SetDuplicatePrimitiveBuffers(uchar* vertices, ushort vertexCount,
                                                        uchar* edges, ushort edgeCount,
                                                        uchar* triangles, ushort triangleCount) {
  spDupVertexList = vertices;
  sDupVertexCount = vertexCount;
  spDupEdgeList = edges;
  sDupEdgeCount = edgeCount;
  spDupTriangleList = triangles;
  sDupTriangleCount = triangleCount;
  sDupPrimitiveCheckCount = 0xff;
}

void CMetroidAreaCollider::ResetInternalCounters() {
  gCalledClip = 0;
  gRejectedByClip = 0;
  gTrianglesProcessed = 0;
  gDupTrianglesProcessed = 0;
  if (sDupPrimitiveCheckCount == 0xff) {
    memset(spDupVertexList, 0, sDupVertexCount);
    memset(spDupEdgeList, 0, sDupEdgeCount);
    memset(spDupTriangleList, 0, sDupTriangleCount);
    ++sDupPrimitiveCheckCount;
  }
  ++sDupPrimitiveCheckCount;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Internal(const CAreaOctTree::Node& node,
                                                        CAABoxAreaCache& cache) {
  bool ret = false;

  switch (node.GetTreeType()) {
  case CAreaOctTree::Node::kTT_Invalid:
    return false;
  case CAreaOctTree::Node::kTT_Branch: {
    for (int i = 0; i < 8; ++i) {
      CAreaOctTree::Node ch = node.GetChild(i);
      CAABox box = ch.GetBoundingBox();
      if (box.DoBoundsOverlap(cache.mAabb))
        if (AABoxCollisionCheck_Internal(ch, cache))
          ret = true;
    }
    break;
  }
  case CAreaOctTree::Node::kTT_Leaf: {
    CAreaOctTree::TriListReference list = node.GetTriangleArray();
    int size = list.GetSize();
    const CAreaOctTree& tree = node.GetOwner();
    const CAreaOctTree& owner = tree;
    const CMaterialFilter& filter = cache.mFilter;
    const CPlane* planes = cache.mPlanes;
    for (int j = 0; j < size; ++j) {
      ++gTrianglesProcessed;
      uint triIdx = list.GetAt(j);
      if (sDupPrimitiveCheckCount == spDupTriangleList[triIdx]) {
        ++gDupTrianglesProcessed;
      } else {
        spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
        const CCollisionSurface surf(owner.GetTriangle(triIdx));
        CMaterialList material(surf.GetSurfaceFlags());
        if (filter.Passes(material)) {
          if (CollisionUtil::TriBoxOverlap(cache.mCenter, cache.mHalfExtent, surf.GetVert(0),
                                           surf.GetVert(1), surf.GetVert(2)) == true) {
            CAABox aabb = CAABox::MakeMaxInvertedBox();
            if (ConvexPolyCollision(planes, &surf.GetVert(0), aabb)) {
              CPlane plane = surf.GetPlane();
              cache.mCollisionList.Add(CCollisionInfo(aabb, cache.mMaterial, material,
                                                      plane.GetNormal(), -plane.GetNormal(), -1));
              ret = true;
            }
          }
        }
      }
    }
    break;
  }
  default:
    break;
  }

  return ret;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter,
                                                      const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  static const CUnitVector3f right(1.f, 0.f, 0.f);
  static const CUnitVector3f forward(0.f, 1.f, 0.f);
  static const CUnitVector3f up(0.f, 0.f, 1.f);
  const CVector3f min = aabb.GetMinPoint();
  const CVector3f max = aabb.GetMaxPoint();
  const CPlane planes[6] = {
      CPlane(min, right),    CPlane(max, -right), CPlane(min, forward),
      CPlane(max, -forward), CPlane(min, up),     CPlane(max, -up),
  };

  ResetInternalCounters();
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;
  bool ret = false;

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference listRef = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int size = listRef.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        ushort triIdx = listRef.GetAt(j);
        if (sDupPrimitiveCheckCount == spDupTriangleList[triIdx]) {
          ++gDupTrianglesProcessed;
        } else {
          spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          const CCollisionSurface& surf = owner.GetTriangle(triIdx);
          CMaterialList material(surf.GetSurfaceFlags());
          if (filter.Passes(material)) {
            if (CollisionUtil::TriBoxOverlap(center, halfExtent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              CAABox aabb2 = CAABox::MakeMaxInvertedBox();
              if (ConvexPolyCollision(planes, &surf.GetVert(0), aabb2)) {
                CPlane plane = surf.GetPlane();
                list.Add(CCollisionInfo(aabb2, matList, material, plane.GetNormal(),
                                        -plane.GetNormal(), -1));
                ret = true;
              }
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(CCollisionCache& cache, const CAABox& aabb,
                                                      const CMaterialFilter& filter,
                                                      const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  static const CUnitVector3f right(1.f, 0.f, 0.f);
  static const CUnitVector3f forward(0.f, 1.f, 0.f);
  static const CUnitVector3f up(0.f, 0.f, 1.f);
  const CPlane planes[6] = {
      CPlane(aabb.GetMinPoint(), right),   CPlane(aabb.GetMaxPoint(), -right),
      CPlane(aabb.GetMinPoint(), forward), CPlane(aabb.GetMaxPoint(), -forward),
      CPlane(aabb.GetMinPoint(), up),      CPlane(aabb.GetMaxPoint(), -up),
  };
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;
  bool ret = false;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !aabb.DoBoundsOverlap(*cache.GetLeafBounds(iterator))) {
      cache.SkipLeaf(iterator);
      continue;
    }
    SCachedCollisionSlot* slot = cache.NextTriangle(iterator);
    const CCollisionSurface& surface = slot->mTriangle.GetSurface();
    ++gTrianglesProcessed;
    if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2)) == true) {
      CMaterialList material(surface.GetSurfaceFlags());
      if (filter.Passes(material)) {
        CAABox clipped = CAABox::MakeMaxInvertedBox();
        if (ConvexPolyCollision(planes, &surface.GetVert(0), clipped)) {
          list.Add(CCollisionInfo(
              clipped, matList, CMaterialList(material.GetValue() | iterator.GetMaterialFlags()),
              slot->mPlane.GetNormal(), -slot->mPlane.GetNormal(), iterator.GetObjectId()));
          ret = true;
        }
      }
    }
  }
  return ret;
}

CAABoxAreaCache::CAABoxAreaCache(const CAABox& aabb, const CPlane* pl,
                                 const CMaterialFilter& filter, const CMaterialList& material,
                                 CCollisionInfoList& collisionList)
: mAabb(aabb)
, mPlanes(pl)
, mFilter(filter)
, mMaterial(material)
, mCollisionList(collisionList)
, mCenter(aabb.GetCenterPoint())
, mHalfExtent((aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f) {}

bool CMetroidAreaCollider::AABoxCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                               const CMaterialFilter& filter,
                                               const CMaterialList& matList,
                                               CCollisionInfoList& list) {
  const CVector3f& min = aabb.GetMinPoint();
  const CVector3f& max = aabb.GetMaxPoint();
  const CUnitVector3f xAxis(1.f, 0.f, 0.f);
  const CUnitVector3f yAxis(0.f, 1.f, 0.f);
  const CUnitVector3f zAxis(0.f, 0.f, 1.f);
  CPlane planes[6] = {
      CPlane(min, xAxis),  CPlane(max, -xAxis), CPlane(min, yAxis),
      CPlane(max, -yAxis), CPlane(min, zAxis),  CPlane(max, -zAxis),
  };
  CAABoxAreaCache cache(aabb, planes, filter, matList, list);
  ResetInternalCounters();
  return AABoxCollisionCheck_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Internal(
    const CAreaOctTree::Node& node, const CBooleanAABoxAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& tree = ch.GetOwner();
          const CAreaOctTree& owner = tree;
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            const CCollisionSurface surf(owner.GetTriangle(list.GetAt(j)));
            if (cache.mFilter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriBoxOverlap(cache.mCenter, cache.mHalfExtent, surf.GetVert(0),
                                               surf.GetVert(1), surf.GetVert(2)) == true)
                return true;
            }
          }
        } else {
          if (AABoxCollisionCheckBoolean_Internal(ch, cache) == true)
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                             const CAABox& aabb,
                                                             const CMaterialFilter& filter) {
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& tree = node.GetOwner();
      const CAreaOctTree& owner = tree;
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        const CCollisionSurface surf(owner.GetTriangle(list.GetAt(j)));
        if (filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriBoxOverlap(center, halfExtent, surf.GetVert(0), surf.GetVert(1),
                                           surf.GetVert(2)) == true)
            return true;
        }
      }
    }
  }

  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(CCollisionCache& cache,
                                                             const CAABox& aabb,
                                                             const CMaterialFilter& filter) {
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;
  gTrianglesProcessed += cache.GetNumTriangles();
  const CCollisionCache& readCache = cache;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !aabb.DoBoundsOverlap(*cache.GetLeafBounds(iterator))) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const CCachedCollisionSurface* tri = readCache.NextTriangle(iterator);
    const CCollisionSurface& surface = tri->GetSurface();
    if (filter.Passes(CMaterialList(surface.GetSurfaceFlags())) &&
        CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2)) == true) {
      return true;
    }
  }
  return false;
}

CBooleanAABoxAreaCache::CBooleanAABoxAreaCache(const CAABox& aabb, const CMaterialFilter& filter)
: mAabb(aabb)
, mFilter(filter)
, mCenter(aabb.GetCenterPoint())
, mHalfExtent((aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f) {}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter) {
  CBooleanAABoxAreaCache cache(aabb, filter);
  return AABoxCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheck_Internal(const CAreaOctTree::Node& node,
                                                         CSphereAreaCache& cache) {
  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType chTp = node.GetChildType(i);
    if (chTp != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (chTp == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& tree = ch.GetOwner();
          const CAreaOctTree& owner = tree;
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            uint triIdx = list.GetAt(j);
            if (sDupPrimitiveCheckCount == spDupTriangleList[triIdx]) {
              ++gDupTrianglesProcessed;
            } else {
              spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
              const CCollisionSurface surf(owner.GetTriangle(triIdx));
              CMaterialList material(surf.GetSurfaceFlags());
              if (cache.mFilter.Passes(material)) {
                if (CollisionUtil::TriSphereIntersection(cache.mSphere, surf.GetVert(0),
                                                         surf.GetVert(1), surf.GetVert(2), point,
                                                         normal)) {
                  cache.mCollisionList.Add(
                      CCollisionInfo(point, cache.mMaterial, material, normal, -1));
                  ret = true;
                }
              }
            }
          }
        } else {
          if (SphereCollisionCheck_Internal(ch, cache) == true)
            ret = true;
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialList& matList,
                                                       const CMaterialFilter& filter,
                                                       CCollisionInfoList& clist) {
  ResetInternalCounters();

  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& tree = node.GetOwner();
      const CAreaOctTree& owner = tree;
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        uint triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount == spDupTriangleList[triIdx]) {
          ++gDupTrianglesProcessed;
        } else {
          spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          const CCollisionSurface surf(owner.GetTriangle(triIdx));
          CMaterialList material(surf.GetSurfaceFlags());
          if (filter.Passes(material)) {
            if (CollisionUtil::TriSphereIntersection(sphere, surf.GetVert(0), surf.GetVert(1),
                                                     surf.GetVert(2), point, normal)) {
              clist.Add(CCollisionInfo(point, matList, material, normal, -1));
              ret = true;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(CCollisionCache& cache, const CAABox& aabb,
                                                       const CSphere& sphere,
                                                       const CMaterialList& matList,
                                                       const CMaterialFilter& filter,
                                                       CCollisionInfoList& list) {
  CVector3f point(CVector3f::Zero());
  CVector3f normal(CVector3f::Zero());
  bool ret = false;
  const CCollisionCache& readCache = cache;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !aabb.DoBoundsOverlap(*cache.GetLeafBounds(iterator))) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const CCollisionSurface& surface = readCache.NextTriangle(iterator)->GetSurface();
    ++gTrianglesProcessed;
    CMaterialList material(surface.GetSurfaceFlags());
    if (filter.Passes(material) &&
        CollisionUtil::TriSphereIntersection(sphere, surface.GetVert(0), surface.GetVert(1),
                                             surface.GetVert(2), point, normal)) {
      list.Add(CCollisionInfo(point, matList,
                              CMaterialList(material.GetValue() | iterator.GetMaterialFlags()),
                              normal, iterator.GetObjectId()));
      ret = true;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                                const CSphere& sphere, const CMaterialList& matList,
                                                const CMaterialFilter& filter,
                                                CCollisionInfoList& list) {
  CSphereAreaCache cache(aabb, sphere, filter, matList, list);
  ResetInternalCounters();
  return SphereCollisionCheck_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Internal(
    const CAreaOctTree::Node& node, const CBooleanSphereAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& tree = ch.GetOwner();
          const CAreaOctTree& owner = tree;
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            const CCollisionSurface surf(owner.GetTriangle(list.GetAt(j)));
            if (cache.mFilter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriSphereOverlap(cache.mSphere, surf.GetVert(0), surf.GetVert(1),
                                                  surf.GetVert(2)) == true)
                return true;
            }
          }
        } else {
          if (SphereCollisionCheckBoolean_Internal(ch, cache) == true)
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                              const CAABox& aabb,
                                                              const CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& tree = node.GetOwner();
      const CAreaOctTree& owner = tree;
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        const CCollisionSurface surf(owner.GetTriangle(list.GetAt(j)));
        if (filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriSphereOverlap(sphere, surf.GetVert(0), surf.GetVert(1),
                                              surf.GetVert(2)) == true)
            return true;
        }
      }
    }
  }

  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(CCollisionCache& cache,
                                                              const CAABox& aabb,
                                                              const CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  const CCollisionCache& readCache = cache;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !aabb.DoBoundsOverlap(*cache.GetLeafBounds(iterator))) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const CCollisionSurface& surface = readCache.NextTriangle(iterator)->GetSurface();
    ++gTrianglesProcessed;
    if (filter.Passes(CMaterialList(surface.GetSurfaceFlags())) &&
        CollisionUtil::TriSphereOverlap(sphere, surface.GetVert(0), surface.GetVert(1),
                                        surface.GetVert(2)) == true) {
      return true;
    }
  }
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialFilter& filter) {
  CBooleanSphereAreaCache cache(aabb, sphere, filter);
  return SphereCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

static inline CVector3f TriangleEdgeNormal(const CVector3f& normal, const CVector3f& edge) {
  return CVector3f::Cross(normal, edge);
}

bool CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CSphere& sphere,
    const CMaterialFilter& filter, const CMaterialList& matList, CVector3f dir, float mag,
    CCollisionInfo& infoOut, double& dOut) {
  dOut = mag;
  ResetInternalCounters();

  CVector3f moveVec = mag * dir;
  CAABox movedAABB = aabb;
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  CVector3f extent = movedAABB.GetHalfExtent();
  bool ret = false;

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (movedAABB.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int listSize = list.GetSize();
      for (int j = 0; j < listSize; ++j) {
        uint triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount != spDupTriangleList[triIdx]) {
          spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          ++gTrianglesProcessed;
          u64 matValue = owner.GetTriangleMaterial(triIdx);
          CMaterialList triMat(matValue);
          if (filter.Passes(triMat)) {
            ushort vertIndices[3];
            owner.GetTriangleVertexIndices(triIdx, vertIndices);
            CCollisionSurface surf(owner.GetVert(vertIndices[0]), owner.GetVert(vertIndices[1]),
                                   owner.GetVert(vertIndices[2]), matValue);

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              CVector3f surfNormal = surf.GetNormal();
              CVector3f endDelta = sphere.GetCenter() + moveVec - surf.GetVert(0);
              float endHeight = CVector3f::Dot(endDelta, surfNormal);
              if (!(endHeight > sphere.GetRadius())) {
                bool triRet = false;

                double triMagD = sphere.GetRadius() -
                                 CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0), surfNormal);
                triMagD /= CVector3f::Dot(dir, surfNormal);
                CVector3f intersectPoint = sphere.GetCenter() + static_cast< float >(triMagD) * dir;

                bool outsideEdges[3];
                outsideEdges[0] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(0),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(1) - surf.GetVert(0))) < 0.f;
                outsideEdges[1] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(1),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(2) - surf.GetVert(1))) < 0.f;
                outsideEdges[2] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(2),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(0) - surf.GetVert(2))) < 0.f;

                if (triMagD >= 0.0 && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] &&
                    triMagD < dOut) {
                  triRet = true;
                  infoOut = CCollisionInfo(intersectPoint - sphere.GetRadius() * surfNormal,
                                           matList, triMat, surfNormal, -1);
                  dOut = triMagD;
                  ret = true;
                }

                bool intersects = CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0),
                                                 surfNormal) <= sphere.GetRadius();
                bool testVert[3] = {true, true, true};
                const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
                for (int k = 0; k < 3; ++k) {
                  if (intersects || outsideEdges[k]) {
                    int edgeIdx = edgeIndices[k];
                    if (sDupPrimitiveCheckCount != spDupEdgeList[edgeIdx]) {
                      spDupEdgeList[edgeIdx] = sDupPrimitiveCheckCount;
                      u64 edgeMatVal = owner.GetEdgeMaterial(edgeIdx);
                      if (!(edgeMatVal & (u64(1) << kMT_NoEdgeCollision))) {
                        static int mod3[4] = {0, 1, 2, 0};
                        CVector3f edgeVec = surf.GetVert(mod3[k + 1]) - surf.GetVert(k);
                        float edgeVecMag = edgeVec.Magnitude();
                        edgeVec *= 1.f / edgeVecMag;

                        CVector3f vertToSphere = sphere.GetCenter() - surf.GetVert(k);
                        float vtsDotEdge = CVector3f::Dot(vertToSphere, edgeVec);
                        CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                        float dirDotEdge = CVector3f::Dot(dir, edgeVec);
                        CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                        float edgeRejMagSq = edgeRej.MagSquared();

                        if (edgeRejMagSq > 0.f) {
                          float tmp = 2.f * CVector3f::Dot(vtsRej, edgeRej);
                          float tmp2 = tmp * tmp - 4.f * edgeRejMagSq *
                                                       (vtsRej.MagSquared() -
                                                        sphere.GetRadius() * sphere.GetRadius());
                          if (tmp2 >= 0.f) {
                            double invDenom = 0.5 / edgeRejMagSq;
                            double eMag = invDenom * (-tmp - sqrt(tmp2));
                            if (eMag >= 0.0) {
                              double t = eMag * dirDotEdge + vtsDotEdge;
                              if (t >= 0.0 && t <= edgeVecMag && eMag < dOut) {
                                triRet = true;
                                CVector3f ePoint =
                                    surf.GetVert(k) + static_cast< float >(t) * edgeVec;
                                CVector3f eNormal =
                                    (sphere.GetCenter() + static_cast< float >(eMag) * dir - ePoint)
                                        .AsNormalized();
                                infoOut = CCollisionInfo(ePoint, matList, CMaterialList(edgeMatVal),
                                                         eNormal, -1);
                                dOut = eMag;
                                ret = true;
                                testVert[k] = false;
                                testVert[mod3[k + 1]] = false;
                              } else if (t < -sphere.GetRadius() && dirDotEdge <= 0.f) {
                                testVert[k] = false;
                              } else if (t > edgeVecMag + sphere.GetRadius() && dirDotEdge >= 0.f) {
                                testVert[mod3[k + 1]] = false;
                              }
                            }
                          } else {
                            testVert[k] = false;
                            testVert[mod3[k + 1]] = false;
                          }
                        }
                      }
                    }
                  }
                }

                for (int k = 0; k < 3; ++k) {
                  int vertIdx = vertIndices[k];
                  if (testVert[k]) {
                    if (!(owner.GetVertMaterial(vertIdx) & (u64(1) << kMT_NoEdgeCollision)) &&
                        sDupPrimitiveCheckCount != spDupVertexList[vertIdx]) {
                      spDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                      double d = dOut;
                      if (CollisionUtil::RaySphereIntersection_Double(
                              CSphere(surf.GetVert(k), sphere.GetRadius()), sphere.GetCenter(), dir,
                              d) &&
                          d >= 0.0) {
                        triRet = true;
                        CVector3f vNormal =
                            (sphere.GetCenter() + dir * static_cast< float >(d) - surf.GetVert(k))
                                .AsNormalized();
                        infoOut = CCollisionInfo(surf.GetVert(k), matList,
                                                 CMaterialList(owner.GetVertMaterial(vertIdx)),
                                                 vNormal, -1);
                        dOut = d;
                        ret = true;
                      }
                    }
                  } else {
                    spDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                  }
                }

                if (triRet) {
                  moveVec = static_cast< float >(dOut) * dir;
                  movedAABB = aabb;
                  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
                  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
                  center = movedAABB.GetCenterPoint();
                  extent = movedAABB.GetHalfExtent();
                }
              }
            } else {
              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              spDupEdgeList[edgeIndices[0]] = sDupPrimitiveCheckCount;
              spDupEdgeList[edgeIndices[1]] = sDupPrimitiveCheckCount;
              spDupEdgeList[edgeIndices[2]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[0]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[1]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[2]] = sDupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
    CCollisionCache& cache, const CAABox& aabb, const CMaterialFilter& filter,
    const CMaterialList& matList, CVector3f dir, float mag, CCollisionInfo& infoOut, double& dOut) {

  dOut = mag;
  ResetInternalCounters();

  CVector3f moveVec = mag * dir;
  CMovingAABoxComponents components(aabb, dir);

  CAABox movedAABB = components.mAabb;
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  CVector3f extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;
  bool ret = false;

  CVector3f normal(CVector3f::Zero());
  CVector3f point(CVector3f::Zero());

  const CCollisionCache& readCache = cache;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !movedAABB.DoBoundsOverlap(*cache.GetLeafBounds(iterator))) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const CCachedCollisionSurface& triangle = *readCache.NextTriangle(iterator);
    const CCollisionSurface& surf = triangle.GetSurface();
    ushort triIdx = triangle.GetTriangleIndex();
    ++gTrianglesProcessed;
    CMaterialList triMat(surf.GetSurfaceFlags());
    if (filter.Passes(triMat) &&
        CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                     surf.GetVert(2)) == true) {
      const CCollisionPrimitiveData& owner = iterator.GetGeometry();
      ushort vertIndices[3];
      owner.GetTriangleVertexIndices(triIdx, vertIndices);
      bool triRet = false;
      double d = dOut;
      if (MovingAABoxCollisionCheck_BoxVertexTri(surf, aabb, components.mVertIdxs, dir, d, normal,
                                                 point) &&
          d < dOut) {
        triRet = true;
        infoOut = CCollisionInfo(point, matList,
                                 CMaterialList(triMat.GetValue() | iterator.GetMaterialFlags()),
                                 normal, iterator.GetObjectId());
        ret = true;
        dOut = d;
      }

      for (int k = 0; k < 3; ++k) {
        int vertIdx = vertIndices[k];
        if (!(owner.GetVertMaterial(vertIdx) & (u64(1) << kMT_NoEdgeCollision)) &&
            sDupPrimitiveCheckCount != spDupVertexList[vertIdx]) {
          spDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
          const CVector3f& vtx = surf.GetVert(k);
          if (movedAABB.PointInside(vtx)) {
            d = dOut;
            if (MovingAABoxCollisionCheck_TriVertexBox(vtx, aabb, dir, d, normal, point) &&
                d < dOut) {
              CMaterialList vertMat(owner.GetVertMaterial(vertIdx));
              triRet = true;
              infoOut = CCollisionInfo(
                  point, matList, CMaterialList(vertMat.GetValue() | iterator.GetMaterialFlags()),
                  normal, iterator.GetObjectId());
              ret = true;
              dOut = d;
            }
          }
        }
      }

      const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
      for (int k = 0; k < 3; ++k) {
        int edgeIdx = edgeIndices[k];
        if (sDupPrimitiveCheckCount != spDupEdgeList[edgeIdx]) {
          u64 edgeMat = owner.GetEdgeMaterial(edgeIdx);
          if (!(edgeMat & (u64(1) << kMT_NoEdgeCollision))) {
            spDupEdgeList[edgeIdx] = sDupPrimitiveCheckCount;
            d = dOut;
            static const int nextVertex[3] = {1, 2, 0};
            if (MovingAABoxCollisionCheck_Edge(surf.GetVert(k), surf.GetVert(nextVertex[k]),
                                               components.mEdges, dir, d, normal, point) &&
                d < dOut) {
              triRet = true;
              infoOut = CCollisionInfo(point, matList,
                                       CMaterialList(edgeMat | iterator.GetMaterialFlags()), normal,
                                       iterator.GetObjectId());
              ret = true;
              dOut = d;
            }
          }
        }
      }

      if (triRet) {
        moveVec = static_cast< float >(dOut) * dir;
        movedAABB = components.mAabb;
        movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
        movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
        center = movedAABB.GetCenterPoint();
        extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;
      }
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CMaterialFilter& filter,
    const CMaterialList& matList, CVector3f dir, float mag, CCollisionInfo& infoOut, double& dOut) {
  dOut = mag;
  ResetInternalCounters();

  CVector3f moveVec = mag * dir;
  CMovingAABoxComponents components(aabb, dir);

  CAABox movedAABB = components.mAabb;
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  CVector3f extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;
  bool ret = false;

  CVector3f normal(CVector3f::Zero());
  CVector3f point(CVector3f::Zero());

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (movedAABB.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int listSize = list.GetSize();
      for (int j = 0; j < listSize; ++j) {
        uint triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount != spDupTriangleList[triIdx]) {
          spDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          ++gTrianglesProcessed;
          u64 matValue = owner.GetTriangleMaterial(triIdx);
          CMaterialList triMat(matValue);
          if (filter.Passes(triMat)) {
            ushort vertIndices[3];
            owner.GetTriangleVertexIndices(triIdx, vertIndices);
            CCollisionSurface surf(owner.GetVert(vertIndices[0]), owner.GetVert(vertIndices[1]),
                                   owner.GetVert(vertIndices[2]), matValue);

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              bool triRet = false;
              double d = dOut;
              if (MovingAABoxCollisionCheck_BoxVertexTri(surf, aabb, components.mVertIdxs, dir, d,
                                                         normal, point) &&
                  d < dOut) {
                triRet = true;
                infoOut = CCollisionInfo(point, matList, triMat, normal, -1);
                ret = true;
                dOut = d;
              }

              for (int k = 0; k < 3; ++k) {
                int vertIdx = vertIndices[k];
                if (!(owner.GetVertMaterial(vertIdx) & (u64(1) << kMT_NoEdgeCollision)) &&
                    sDupPrimitiveCheckCount != spDupVertexList[vertIdx]) {
                  spDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                  const CVector3f& vtx = owner.GetVert(vertIdx);
                  if (movedAABB.PointInside(vtx)) {
                    d = dOut;
                    if (MovingAABoxCollisionCheck_TriVertexBox(vtx, aabb, dir, d, normal, point) &&
                        d < dOut) {
                      CMaterialList vertMat(owner.GetVertMaterial(vertIdx));
                      triRet = true;
                      infoOut = CCollisionInfo(point, matList, vertMat, normal, -1);
                      ret = true;
                      dOut = d;
                    }
                  }
                }
              }

              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              for (int k = 0; k < 3; ++k) {
                int edgeIdx = edgeIndices[k];
                if (sDupPrimitiveCheckCount != spDupEdgeList[edgeIdx]) {
                  spDupEdgeList[edgeIdx] = sDupPrimitiveCheckCount;
                  u64 edgeMat = owner.GetEdgeMaterial(edgeIdx);
                  if (!(edgeMat & (u64(1) << kMT_NoEdgeCollision))) {
                    d = dOut;
                    const CCollisionEdge& edge = owner.GetEdge(edgeIdx);
                    if (MovingAABoxCollisionCheck_Edge(owner.GetVert(edge.GetVertIndex1()),
                                                       owner.GetVert(edge.GetVertIndex2()),
                                                       components.mEdges, dir, d, normal, point) &&
                        d < dOut) {
                      triRet = true;
                      infoOut = CCollisionInfo(point, matList, CMaterialList(edgeMat), normal, -1);
                      ret = true;
                      dOut = d;
                    }
                  }
                }
              }

              if (triRet) {
                moveVec = static_cast< float >(dOut) * dir;
                movedAABB = components.mAabb;
                movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
                movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
                center = movedAABB.GetCenterPoint();
                extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;
              }
            } else {
              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              spDupEdgeList[edgeIndices[0]] = sDupPrimitiveCheckCount;
              spDupEdgeList[edgeIndices[1]] = sDupPrimitiveCheckCount;
              spDupEdgeList[edgeIndices[2]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[0]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[1]] = sDupPrimitiveCheckCount;
              spDupVertexList[vertIndices[2]] = sDupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(const CVector3f& vert,
                                                                  const CAABox& aabb, CVector3f dir,
                                                                  double& dOut, CVector3f& normal,
                                                                  CVector3f& point) {
  bool ret = false;
  float rayLen = static_cast< float >(dOut);
  CMRay ray(vert, -dir, rayLen);
  CVector3f norm(CVector3f::Zero());
  double d;
  if (CollisionUtil::RayAABoxIntersection_Double(ray, aabb, norm, d) == 2) {
    double nd = d * dOut;
    if (nd < dOut) {
      ret = true;
      normal = -norm;
      dOut = nd;
      point = vert;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
    const CCollisionSurface& surf, const CAABox& aabb,
    const rstl::reserved_vector< uint, 8 >& vertIndices, CVector3f dir, double& d,
    CVector3f& normalOut, CVector3f& pointOut) {
  bool ret = false;
  for (int i = 0; i < vertIndices.size(); ++i) {
    CVector3f point = aabb.GetPoint(vertIndices[i]);
    if (CollisionUtil::RayTriangleIntersection_Double(point, dir, &surf.GetVert(0), d)) {
      pointOut = point + dir * static_cast< float >(d);
      normalOut = surf.GetNormal();
      ret = true;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(
    const CVector3f& ev0, const CVector3f& ev1, const rstl::reserved_vector< SBoxEdge, 12 >& edges,
    CVector3f dir, double& d, CVector3f& normal, CVector3f& point) {
  bool ret = false;

  CVector3d ev0d = ev0;
  CVector3d ev1d = ev1;
  CVector3d originalDelta = ev0d - ev1d;
  for (int i = 0; i < edges.size(); ++i) {
    const SBoxEdge& edge = edges[i];
    const CVector3d* vertex = &ev0d;
    if ((CVector3d::Dot(edge.mCoDir, *vertex) >= edge.mDirCoDirDot) ==
        (CVector3d::Dot(edge.mCoDir, ev1d) >= edge.mDirCoDirDot))
      continue;

    CVector3d cross0 = CVector3d::Cross(edge.mDelta, originalDelta);
    if (cross0.MagSquared() < FLT_EPSILON)
      continue;

    CVector3d delta = originalDelta;
    CVector3d cross0Norm = cross0.AsNormalized();
    if (CVector3d::Dot(cross0Norm, dir) >= 0.0) {
      vertex = &ev1d;
      delta = -delta;
      cross0 = -cross0;
      cross0Norm = -cross0Norm;
    }

    CVector3d clipped = *vertex + (-(CVector3d::Dot(*vertex, edge.mCoDir) - edge.mDirCoDirDot) /
                                   CVector3d::Dot(delta, edge.mCoDir)) *
                                      delta;
    static const int kOtherAxis0[3] = {1, 0, 0};
    static const int kOtherAxis1[3] = {2, 2, 1};
    const int ci0 = kOtherAxis0[edge.mDominantAxis];
    const int ci1 = kOtherAxis1[edge.mDominantAxis];

    const float& dir0 = dir[ci0];
    const float& dir1 = dir[ci1];
    const double& edgeDelta0 = edge.mDelta[ci0];
    const double& edgeDelta1 = edge.mDelta[ci1];
    const double denominator = edgeDelta0 * dir1 - edgeDelta1 * dir0;
    double eMag = (edge.mDelta[ci0] * (clipped[ci1] - edge.mStart[ci1]) -
                   edge.mDelta[ci1] * (clipped[ci0] - edge.mStart[ci0])) /
                  denominator;

    if (!(eMag < 0.0) && !(eMag >= d)) {
      CVector3d clippedMag = clipped - eMag * CVector3d(dir);
      double dotCheck =
          (edge.mStart.GetX() - clippedMag.GetX()) * (edge.mEnd.GetX() - clippedMag.GetX()) +
          (edge.mStart.GetY() - clippedMag.GetY()) * (edge.mEnd.GetY() - clippedMag.GetY()) +
          (edge.mStart.GetZ() - clippedMag.GetZ()) * (edge.mEnd.GetZ() - clippedMag.GetZ());
      if (dotCheck < 0.0 && eMag < d) {
        normal = cross0Norm.AsCVector3f();
        d = eMag;
        point = clipped.AsCVector3f();
        ret = true;
      }
    }
  }

  return ret;
}

CMetroidAreaCollider::COctreeLeafCache::COctreeLeafCache(const CAreaOctTree& octTree,
                                                         int areaId)
: mAreaId(areaId), mOctTree(octTree), mOverflow(false) {}

void CMetroidAreaCollider::COctreeLeafCache::AddLeaf(const CAreaOctTree::Node& node) {
  if (mNodeCache.size() == mNodeCache.capacity()) {
    mOverflow = true;
    return;
  }
  mNodeCache.push_back(node);
}

CAreaCollisionCache::CAreaCollisionCache(const CAABox& aabb)
: mAabb(aabb), mLeafOverflow(false), mCacheOverflow(false) {}

void CAreaCollisionCache::AddOctreeLeafCache(
    const CMetroidAreaCollider::COctreeLeafCache& leafCache) {
  if (!leafCache.GetNumLeaves())
    return;
  if (leafCache.HasCacheOverflowed())
    mLeafOverflow = true;
  if (mLeafCaches.size() < mLeafCaches.capacity())
    mLeafCaches.push_back(leafCache);
  else {
    mLeafOverflow = true;
    mCacheOverflow = true;
  }
}

void CAreaCollisionCache::SetCacheBounds(const CAABox& aabb) { mAabb = aabb; }

void CAreaCollisionCache::ClearCache() {
  mLeafCaches.clear();
  mLeafOverflow = false;
  mCacheOverflow = false;
}

void CMetroidAreaCollider::BuildOctreeLeafCache(const CAreaOctTree::Node& node, const CAABox& aabb,
                                                COctreeLeafCache& leafCache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type == CAreaOctTree::Node::kTT_Invalid)
      continue;
    CAreaOctTree::Node child = node.GetChild(i);
    if (!aabb.DoBoundsOverlap(child.GetBoundingBox()))
      continue;
    if (type == CAreaOctTree::Node::kTT_Leaf)
      leafCache.AddLeaf(child);
    else
      BuildOctreeLeafCache(child, aabb, leafCache);
  }
}

void CMetroidAreaCollider::BuildCollisionCache(const CAreaOctTree::Node& node,
                                               CCollisionCache& cache) {
  ResetInternalCounters();
  CVector3f center = cache.GetBounds().GetCenterPoint();
  CVector3f halfExtent = (cache.GetBounds().GetMaxPoint() - cache.GetBounds().GetMinPoint()) * 0.5f;
  CCollisionCacheWriter writer(cache);
  writer.BeginGeometry(node.GetOwner(), 0, -1, 0);
  CacheNodes(node, writer, center, halfExtent);
}

void CMetroidAreaCollider::CacheNodes(const CAreaOctTree::Node& node, CCollisionCacheWriter& writer,
                                      const CVector3f& center, const CVector3f& halfExtent) {
  const CAABox& bounds = writer.GetBounds();
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type == CAreaOctTree::Node::kTT_Invalid)
      continue;
    CAreaOctTree::Node child = node.GetChild(i);
    if (!bounds.DoBoundsOverlap(child.GetBoundingBox()))
      continue;
    if (child.GetBoundingBox().Inside(bounds)) {
      CacheAllNodes(child, writer);
    } else if (type == CAreaOctTree::Node::kTT_Leaf) {
      writer.BeginLeaf(CAABox::MakeMaxInvertedBox());
      CAreaOctTree::TriListReference list = child.GetTriangleArray();
      const CAreaOctTree& owner = child.GetOwner();
      int size = list.GetSize();
      writer.ReserveTriangles(size);
      for (int j = 0; j < size; ++j) {
        uint index = list.GetAt(j);
        if (spDupTriangleList[index] != sDupPrimitiveCheckCount) {
          spDupTriangleList[index] = sDupPrimitiveCheckCount;
          CCollisionSurface surface = owner.GetTriangle(index);
          if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0),
                                           surface.GetVert(1), surface.GetVert(2))) {
            writer.AddTriangle(surface, index);
          }
        }
      }
    } else {
      CacheNodes(child, writer, center, halfExtent);
    }
  }
}

void CCollisionCacheWriter::ReserveTriangles(int count) {
  int required = mCache.mData.size();
  required += count * (sizeof(SCachedCollisionSlot) / sizeof(ushort));
  if (mCache.mData.capacity() < required) {
    ushort* oldData = mCache.mData.data();
    mCache.mData.reserve(mCache.mData.capacity() * 2 > required ? mCache.mData.capacity() * 2 : required * 2);
    const int delta = mCache.mData.data() - oldData;
    mLeafCount += delta;
    mTriangleCount += delta;
    mLeafBounds = reinterpret_cast< CAABox* >(reinterpret_cast< ushort* >(mLeafBounds) + delta);
  }
}

CCachedCollisionSurface::CCachedCollisionSurface(const CCollisionSurface& surface,
                                                 ushort triangleIndex)
: mSurface(surface), mTriangleIndex(triangleIndex) {}

void CCollisionCacheWriter::AddTriangle(const CCollisionSurface& surface, ushort triangleIndex) {
  ++*mTriangleCount;
  int size = mCache.mData.size();
  new (mCache.mData.data() + size) CCachedCollisionSurface(surface, triangleIndex);
  size += sizeof(SCachedCollisionSlot) / sizeof(ushort);
  if (size > mCache.mData.size()) {
    mCache.mData.reserve(size);
  }
  // The payload is already constructed; resize would overwrite it with zeros.
  mCache.mData.mCount = size;
  mLeafBounds->AccumulateBounds(surface.GetVert(0));
  mLeafBounds->AccumulateBounds(surface.GetVert(1));
  mLeafBounds->AccumulateBounds(surface.GetVert(2));
}

void CMetroidAreaCollider::CacheAllNodes(const CAreaOctTree::Node& node,
                                         CCollisionCacheWriter& writer) {
  if (node.GetTreeType() == CAreaOctTree::Node::kTT_Leaf) {
    writer.BeginLeaf(CAABox::MakeMaxInvertedBox());
    CAreaOctTree::TriListReference list = node.GetTriangleArray();
    const CAreaOctTree& tree = node.GetOwner();
    const CAreaOctTree& owner = tree;
    int size = list.GetSize();
    writer.ReserveTriangles(size);
    for (int i = 0; i < size; ++i) {
      uint index = list.GetAt(i);
      if (spDupTriangleList[index] != sDupPrimitiveCheckCount) {
        spDupTriangleList[index] = sDupPrimitiveCheckCount;
        CCollisionSurface surface = owner.GetTriangle(index);
        writer.AddTriangle(surface, index);
      }
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      if (node.GetChildType(i) != CAreaOctTree::Node::kTT_Invalid) {
        CacheAllNodes(node.GetChild(i), writer);
      }
    }
  }
}

static void FlagEdgeIndicesForFace(uint face, bool* edgeFlags) {
  switch (face) {
  case 0:
    edgeFlags[10] = true;
    edgeFlags[11] = true;
    edgeFlags[2] = true;
    edgeFlags[4] = true;
    return;
  case 1:
    edgeFlags[8] = true;
    edgeFlags[9] = true;
    edgeFlags[0] = true;
    edgeFlags[6] = true;
    return;
  case 2:
    edgeFlags[4] = true;
    edgeFlags[5] = true;
    edgeFlags[6] = true;
    edgeFlags[7] = true;
    return;
  case 3:
    edgeFlags[0] = true;
    edgeFlags[1] = true;
    edgeFlags[2] = true;
    edgeFlags[3] = true;
    return;
  case 4:
    edgeFlags[7] = true;
    edgeFlags[8] = true;
    edgeFlags[3] = true;
    edgeFlags[11] = true;
    return;
  case 5:
    edgeFlags[1] = true;
    edgeFlags[5] = true;
    edgeFlags[9] = true;
    edgeFlags[10] = true;
    return;
  default:
    break;
  }
}

static void FlagVertexIndicesForFace(uint face, bool* vertFlags) {
  switch (face) {
  case 0:
    vertFlags[1] = true;
    vertFlags[3] = true;
    vertFlags[5] = true;
    vertFlags[7] = true;
    return;
  case 1:
    vertFlags[0] = true;
    vertFlags[2] = true;
    vertFlags[4] = true;
    vertFlags[6] = true;
    return;
  case 2:
    vertFlags[2] = true;
    vertFlags[3] = true;
    vertFlags[6] = true;
    vertFlags[7] = true;
    return;
  case 3:
    vertFlags[0] = true;
    vertFlags[1] = true;
    vertFlags[4] = true;
    vertFlags[5] = true;
    return;
  case 4:
    vertFlags[4] = true;
    vertFlags[5] = true;
    vertFlags[6] = true;
    vertFlags[7] = true;
    return;
  case 5:
    vertFlags[0] = true;
    vertFlags[1] = true;
    vertFlags[2] = true;
    vertFlags[3] = true;
    return;
  default:
    break;
  }
}

CMetroidAreaCollider::SBoxEdge::SBoxEdge()
: mStart(CVector3d::Zero())
, mEnd(CVector3d::Zero())
, mDelta(CVector3d::Zero())
, mCoDir(CVector3d::Zero())
, mDirCoDirDot(0.) {}

CMetroidAreaCollider::CMovingAABoxComponents::CMovingAABoxComponents(const CAABox& aabb,
                                                                     const CVector3f& dir)
: mAabb(aabb) {
  bool edgeFlags[12] = {};
  bool vertFlags[8] = {};
  static rstl::reserved_vector< rstl::pair< int, int >, 12 > edgeIndices;
  static rstl::reserved_vector< SBoxEdge, 12 > edges;
  static int edgeAxes[12];
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    for (int i = 0; i < 12; ++i) {
      edgeIndices.push_back(CAABox::GetEdge(CAABox::EBoxEdgeId(i)));
      rstl::pair< int, int > indices = CAABox::GetEdge(CAABox::EBoxEdgeId(i));
      edgeAxes[i] = indices.first - indices.second;
      edges.push_back(SBoxEdge());
    }
  }

  uint faceCount = 0;
  for (int i = 0; i < 3; ++i) {
    if (dir[i] != 0.f) {
      uint face = i * 2 + (dir[i] < 0.f);
      FlagEdgeIndicesForFace(face, edgeFlags);
      FlagVertexIndicesForFace(face, vertFlags);
      ++faceCount;
    }
  }
  double invX = 1.;
  double invY = 1.;
  double invZ = 1.;
  if (faceCount != 1) {
    double x = dir.GetX();
    double y = dir.GetY();
    double z = dir.GetZ();
    invX = 1. / sqrt(y * y + z * z);
    invY = 1. / sqrt(x * x + z * z);
    invZ = 1. / sqrt(x * x + y * y);
  }
  for (int i = 0; i < 12; ++i) {
    if (!edgeFlags[i])
      continue;
    SBoxEdge edge = edges[i];
    switch (edgeAxes[i]) {
    case -1:
      edge.mCoDir[1] = -dir.GetZ() * invX;
      edge.mCoDir[2] = dir.GetY() * invX;
      break;
    case 1:
      edge.mCoDir[1] = dir.GetZ() * invX;
      edge.mCoDir[2] = -dir.GetY() * invX;
      break;
    case -2:
      edge.mCoDir[0] = dir.GetZ() * invY;
      edge.mCoDir[2] = -dir.GetX() * invY;
      break;
    case 2:
      edge.mCoDir[0] = dir.GetZ() * invY;
      edge.mCoDir[2] = -dir.GetX() * invY;
      break;
    case -4:
      edge.mCoDir[0] = -dir.GetY() * invZ;
      edge.mCoDir[1] = dir.GetX() * invZ;
      break;
    case 4:
      edge.mCoDir[0] = dir.GetY() * invZ;
      edge.mCoDir[1] = -dir.GetX() * invZ;
      break;
    }
    edge.mStart = CVector3d(aabb.GetPoint(edgeIndices[i].first));
    edge.mEnd = CVector3d(aabb.GetPoint(edgeIndices[i].second));
    edge.mDelta = edge.mEnd - edge.mStart;
    edge.mDirCoDirDot = CVector3d::Dot(edge.mStart, edge.mCoDir);
    edge.mDominantAxis = fabs(edge.mCoDir.GetX()) > fabs(edge.mCoDir.GetY()) ? 0 : 1;
    if (fabs(edge.mCoDir[edge.mDominantAxis]) < fabs(edge.mCoDir.GetZ()))
      edge.mDominantAxis = 2;
    mEdges.push_back(edge);
  }
  for (uint i = 0; i < 8; ++i) {
    if (vertFlags[i])
      mVertIdxs.push_back(i);
  }
  if (faceCount == 1) {
    mAabb = CAABox::MakeMaxInvertedBox();
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[0]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[1]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[2]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[3]));
  }
}
