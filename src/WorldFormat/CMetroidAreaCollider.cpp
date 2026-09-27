#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/CCollisionCache.hpp"

#include <string.h>

static uint gCalledClip = 0;
static uint gRejectedByClip = 0;
static uint gTrianglesProcessed = 0;
static uint gDupTrianglesProcessed = 0;
uchar CMetroidAreaCollider::sDupPrimitiveCheckCount = 0xff;
uchar* CMetroidAreaCollider::spDupVertexList;
uchar* CMetroidAreaCollider::spDupEdgeList;
uchar* CMetroidAreaCollider::spDupTriangleList;
ushort CMetroidAreaCollider::sDupVertexCount;
ushort CMetroidAreaCollider::sDupEdgeCount;
ushort CMetroidAreaCollider::sDupTriangleCount;

static void FlagVertexIndicesForFace(uint face, bool* vertFlags);
static void FlagEdgeIndicesForFace(uint face, bool* edgeFlags);
static float PlaneIntersectionFraction(const CVector3f& start, const CVector3f& end,
                                       const CPlane& plane); // Guessed name

// Guessed name
static float PlaneIntersectionFraction(const CVector3f& start, const CVector3f& end,
                                       const CPlane& plane) {
  return -plane.GetHeight(start) / CVector3f::Dot(end - start, plane.GetNormal());
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

bool CMetroidAreaCollider::AABoxCollisionCheck_Internal(const CAreaOctTree::Node&,
                                                        CAABoxAreaCache&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter,
                                                      const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const CCollisionCache& cache,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter,
                                                      const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
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
, mHalfExtent(aabb.GetHalfExtent()) {}

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

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                               const CBooleanAABoxAreaCache&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                             const CAABox& aabb,
                                                             const CMaterialFilter& filter) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const CCollisionCache& cache,
                                                             const CAABox& aabb,
                                                             const CMaterialFilter& filter) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

CBooleanAABoxAreaCache::CBooleanAABoxAreaCache(const CAABox& aabb, const CMaterialFilter& filter)
: mAabb(aabb), mFilter(filter), mCenter(aabb.GetCenterPoint()), mHalfExtent(aabb.GetHalfExtent()) {}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter) {
  CBooleanAABoxAreaCache cache(aabb, filter);
  ResetInternalCounters();
  return AABoxCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheck_Internal(const CAreaOctTree::Node&,
                                                         CSphereAreaCache&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialList& matList,
                                                       const CMaterialFilter& filter,
                                                       CCollisionInfoList& list) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const CCollisionCache& cache,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialList& matList,
                                                       const CMaterialFilter& filter,
                                                       CCollisionInfoList& list) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                                const CSphere& sphere, const CMaterialList& matList,
                                                const CMaterialFilter& filter,
                                                CCollisionInfoList& list) {
  CSphereAreaCache cache(aabb, sphere, filter, matList, list);
  ResetInternalCounters();
  return SphereCollisionCheck_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                                const CBooleanSphereAreaCache&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                              const CAABox& aabb,
                                                              const CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const CCollisionCache& cache,
                                                              const CAABox& aabb,
                                                              const CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialFilter& filter) {
  CBooleanSphereAreaCache cache(aabb, sphere, filter);
  ResetInternalCounters();
  return SphereCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CSphere& sphere,
    const CMaterialFilter& filter, const CMaterialList& matList, CVector3f dir, float d,
    CCollisionInfo& infoOut, double& dOut) {
  dOut = d;
  ResetInternalCounters();
  // TODO: test the swept sphere against triangle faces, edges and vertices.
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
    const CCollisionCache& cache, const CAABox& aabb, const CMaterialFilter& filter,
    const CMaterialList& matList, CVector3f dir, float d, CCollisionInfo& infoOut, double& dOut) {
  dOut = d;
  ResetInternalCounters();
  // TODO: test swept box faces, vertices and edges against cached triangles.
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CMaterialFilter& filter,
    const CMaterialList& matList, CVector3f dir, float d, CCollisionInfo& infoOut, double& dOut) {
  dOut = d;
  ResetInternalCounters();
  // TODO: test swept box faces, vertices and edges against cached triangles.
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(const CVector3f&, const CAABox&,
                                                                  CVector3f, double&, CVector3f&,
                                                                  CVector3f&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
    const CCollisionSurface&, const CAABox&, const rstl::reserved_vector< uint, 8 >&, CVector3f,
    double&, CVector3f&, CVector3f&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(
    const CVector3f&, const CVector3f&, const rstl::reserved_vector< SBoxEdge, 12 >&, CVector3f,
    double&, CVector3f&, CVector3f&) {
  // TODO: reconstruct this collision query from the Echoes target.
  return false;
}

CMetroidAreaCollider::COctreeLeafCache::COctreeLeafCache(const CAreaOctTree& octTree,
                                                         TAreaId areaId)
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
  // TODO: open a packed-cache geometry group and cache nodes inside its bounds.
}

void CMetroidAreaCollider::CacheNodes(const CAreaOctTree::Node& node, CCollisionCacheWriter& writer,
                                      const CVector3f& center, const CVector3f& halfExtent) {
  // TODO: traverse overlapping nodes and append triangles through the shared writer.
}

void CCollisionCacheWriter::ReserveTriangles(int count) {
  // TODO: reserve packed slots and rebase the leaf-count, triangle-count and bounds pointers.
}

CCachedCollisionSurface::CCachedCollisionSurface(const CCollisionSurface& surface,
                                                 ushort triangleIndex)
: mSurface(surface), mTriangleIndex(triangleIndex) {}

void CCollisionCacheWriter::AddTriangle(const CCollisionSurface& surface, ushort triangleIndex) {
  // TODO: append a triangle payload in a fixed-size packed slot and expand leaf bounds.
}

void CMetroidAreaCollider::CacheAllNodes(const CAreaOctTree::Node& node,
                                         CCollisionCacheWriter& writer) {
  // TODO: append leaf triangles to the packed cache, suppressing duplicates.
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
: mStart(0., 0., 0.), mEnd(0., 0., 0.), mDelta(0., 0., 0.), mCoDir(0., 0., 0.), mDirCoDirDot(0.) {}

CMetroidAreaCollider::CMovingAABoxComponents::CMovingAABoxComponents(const CAABox& aabb,
                                                                     const CVector3f& dir)
: mAabb(aabb) {
  bool edgeFlags[12] = {};
  bool vertFlags[8] = {};
  for (int i = 0; i < 3; ++i) {
    if (dir[i] != 0.f) {
      uint face = i * 2 + (dir[i] < 0.f);
      FlagEdgeIndicesForFace(face, edgeFlags);
      FlagVertexIndicesForFace(face, vertFlags);
    }
  }
  for (uint i = 0; i < 8; ++i) {
    if (vertFlags[i])
      mVertIdxs.push_back(i);
  }
  // TODO: precompute selected edges and their dominant axes; collapse the
  // working bounds to the leading face for motion along a single axis.
}
