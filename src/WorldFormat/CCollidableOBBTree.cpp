#include "WorldFormat/CCollidableOBBTree.hpp"

#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CRayCastResult.hpp"

static CPlane TransformPlane(const CPlane& plane, const CTransform4f& xf);

uint CCollidableOBBTree::sTableIndex = -1;

CCollidableOBBTree::CCollidableOBBTree(COBBTree* tree, const CMaterialList& material)
: CCollisionPrimitive(material), mTree(tree), mTries(0), mMisses(0), mHits(0) {}

CCollidableOBBTree::~CCollidableOBBTree() {}

CAABox CCollidableOBBTree::CalculateAABox(const CTransform4f& xf) const {
  COBBox obb = COBBox::FromAABox(GetOBBTree().CalculateLocalAABox(), xf);
  return obb.CalculateAABox(CTransform4f::Identity());
}

CAABox CCollidableOBBTree::CalculateLocalAABox() const {
  return GetOBBTree().CalculateLocalAABox();
}

FourCC CCollidableOBBTree::GetPrimType() const { return 'OBBT'; }

bool CCollidableOBBTree::AABoxCollision(const COBBTree::CNode& node, const CTransform4f& xf,
                                        const CAABox& box, const COBBox& obb,
                                        const CMaterialList& material,
                                        const CMaterialFilter& filter, const CPlane* planes,
                                        CCollisionInfoList& infoList) const {
  ++mTries;
  if (!obb.OBBIntersectsBox(node.GetOBB())) {
    ++mMisses;
    return false;
  }

  node.SetHit(true);
  if (node.IsLeaf()) {
    return AABoxCollideWithLeaf(*node.GetLeafData(), xf, box, material, filter, planes, infoList);
  }

  bool hit = false;
  if (node.GetLeftNode() &&
      AABoxCollision(*node.GetLeftNode(), xf, box, obb, material, filter, planes, infoList)) {
    hit = true;
  }
  if (node.GetRightNode() &&
      AABoxCollision(*node.GetRightNode(), xf, box, obb, material, filter, planes, infoList)) {
    hit = true;
  }
  return hit;
}

bool CCollidableOBBTree::AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf,
                                              const CTransform4f& xf, const CAABox& box,
                                              const CMaterialList& material,
                                              const CMaterialFilter& filter, const CPlane* planes,
                                              CCollisionInfoList& infoList) const {
  // TODO: Filter transformed triangles, clip against the planes, and append contacts.
  return false;
}

bool CCollidableOBBTree::SphereCollision(const COBBTree::CNode& node, const CTransform4f& xf,
                                         const CSphere& sphere, const COBBox& obb,
                                         const CMaterialList& material,
                                         const CMaterialFilter& filter,
                                         CCollisionInfoList& infoList) const {
  ++mTries;
  if (!obb.OBBIntersectsBox(node.GetOBB())) {
    ++mMisses;
    return false;
  }

  node.SetHit(true);
  if (node.IsLeaf()) {
    return SphereCollideWithLeaf(*node.GetLeafData(), xf, sphere, material, filter, infoList);
  }

  bool hit = false;
  if (node.GetLeftNode() &&
      SphereCollision(*node.GetLeftNode(), xf, sphere, obb, material, filter, infoList)) {
    hit = true;
  }
  if (node.GetRightNode() &&
      SphereCollision(*node.GetRightNode(), xf, sphere, obb, material, filter, infoList)) {
    hit = true;
  }
  return hit;
}

bool CCollidableOBBTree::SphereCollideWithLeaf(const COBBTree::CLeafData& leaf,
                                               const CTransform4f& xf, const CSphere& sphere,
                                               const CMaterialList& material,
                                               const CMaterialFilter& filter,
                                               CCollisionInfoList& infoList) const {
  // TODO: Test filtered triangles and append sphere contacts with combined materials.
  return false;
}

bool CCollidableOBBTree::AABoxCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                               const CAABox& box, const COBBox& obb,
                                               const CMaterialFilter& filter) const {
  // TODO: Traverse intersecting nodes and stop at the first filtered triangle overlap.
  return false;
}

bool CCollidableOBBTree::SphereCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                                const CSphere& sphere, const COBBox& obb,
                                                const CMaterialFilter& filter) const {
  // TODO: Traverse intersecting nodes and stop at the first filtered triangle overlap.
  return false;
}

bool CCollidableOBBTree::AABoxCollisionMoving(
    const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& box, const COBBox& obb,
    const CMaterialList& material, const CMaterialFilter& filter,
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& direction,
    double& time, CCollisionInfo& info) const {
  ++mTries;
  if (!obb.OBBIntersectsBox(node.GetOBB())) {
    ++mMisses;
    return false;
  }

  node.SetHit(true);
  if (node.IsLeaf()) {
    return AABoxCollideWithLeafMoving(*node.GetLeafData(), xf, box, material, filter, components,
                                      direction, time, info);
  }

  bool hit = false;
  if (node.GetLeftNode() && AABoxCollisionMoving(*node.GetLeftNode(), xf, box, obb, material,
                                                 filter, components, direction, time, info)) {
    hit = true;
  }
  if (node.GetRightNode() && AABoxCollisionMoving(*node.GetRightNode(), xf, box, obb, material,
                                                  filter, components, direction, time, info)) {
    hit = true;
  }
  return hit;
}

bool CCollidableOBBTree::AABoxCollideWithLeafMoving(
    const COBBTree::CLeafData& leaf, const CTransform4f& xf, const CAABox& box,
    const CMaterialList& material, const CMaterialFilter& filter,
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& direction,
    double& time, CCollisionInfo& info) const {
  // TODO: Sweep the box against triangles, vertices, and edges using the shared duplicate cache.
  return false;
}

bool CCollidableOBBTree::CacheTree(CCollisionCacheWriter& writer, const COBBTree::CNode& node,
                                   const CTransform4f& xf, const CVector3f& center,
                                   const CVector3f& halfExtent, const COBBox& obb) const {
  // TODO: Append overlapping leaf triangles to the packed cache; the target returns false.
  return false;
}

void CCollidableOBBTree::CacheSphere(CCollisionCache& cache, const CTransform4f& xf, short ownerId,
                                     u64 material) {
  // TODO: Choose a prebuilt sphere by scale and cache triangles inside the query bounds.
}

void CCollidableOBBTree::CacheAABox(CCollisionCache& cache, const CTransform4f& xf, short ownerId,
                                    u64 material) {
  // TODO: Cache overlapping triangles from the transformed prebuilt unit cube.
}

bool CCollidableOBBTree::SphereCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf,
                                               const CSphere& sphere, const COBBox& obb,
                                               const CMaterialList& material,
                                               const CMaterialFilter& filter,
                                               const CVector3f& direction, double& time,
                                               CCollisionInfo& info) const {
  ++mTries;
  if (!obb.OBBIntersectsBox(node.GetOBB())) {
    ++mMisses;
    return false;
  }

  node.SetHit(true);
  if (node.IsLeaf()) {
    return SphereCollideWithLeafMoving(*node.GetLeafData(), xf, sphere, material, filter, direction,
                                       time, info);
  }

  bool hit = false;
  if (node.GetLeftNode() && SphereCollisionMoving(*node.GetLeftNode(), xf, sphere, obb, material,
                                                  filter, direction, time, info)) {
    hit = true;
  }
  if (node.GetRightNode() && SphereCollisionMoving(*node.GetRightNode(), xf, sphere, obb, material,
                                                   filter, direction, time, info)) {
    hit = true;
  }
  return hit;
}

bool CCollidableOBBTree::SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf,
                                                     const CTransform4f& xf, const CSphere& sphere,
                                                     const CMaterialList& material,
                                                     const CMaterialFilter& filter,
                                                     const CVector3f& direction, double& time,
                                                     CCollisionInfo& info) const {
  // TODO: Sweep the sphere against triangle faces, edges, and vertices, retaining the first hit.
  return false;
}

CRayCastResult CCollidableOBBTree::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  return LineIntersectsTree(rayCast.GetRay(), rayCast.GetFilter(), rayCast.GetMaxTime(),
                            rayCast.GetTransform());
}

static CPlane TransformPlane(const CPlane& plane, const CTransform4f& xf) {
  const CVector3f point = xf * (plane.GetNormal() * plane.GetConstant());
  const CVector3f normal = xf.Rotate(plane.GetNormal());
  return CPlane(CVector3f::Dot(point, normal), CUnitVector3f(normal, CUnitVector3f::kN_No));
}

CRayCastInfo::CRayCastInfo(const CMRay& ray, const CMaterialFilter& filter, float magnitude)
: mRay(ray)
, mFilter(filter)
, mMagnitude(magnitude)
, mPlane(CVector3f::Zero(), CUnitVector3f(CVector3f(0.f, 0.f, 1.f), CUnitVector3f::kN_Yes))
, mMaterial() {}

CRayCastResult CCollidableOBBTree::LineIntersectsTree(const CMRay& ray,
                                                      const CMaterialFilter& filter, float maxTime,
                                                      const CTransform4f& xf) const {
  CMRay localRay = ray.GetInvUnscaledTransformRay(xf);
  CRayCastInfo info(localRay, filter, maxTime);
  if (LineIntersectsOBBTree(GetOBBTree().GetRoot(), info)) {
    const CPlane plane = TransformPlane(info.GetPlane(), xf);
    return CRayCastResult(info.GetMagnitude(),
                          ray.GetStart() + info.GetMagnitude() * ray.GetDirection(), plane,
                          info.GetMaterial());
  }
  return CRayCastResult();
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* node,
                                               CRayCastInfo& info) const {
  if (!node) {
    return false;
  }

  ++mTries;
  float time;
  if (!node->GetOBB().LineIntersectsBox(info.GetRay(), time) || info.GetMagnitude() <= time) {
    ++mMisses;
    return false;
  }

  const bool hit = node->IsLeaf()
                       ? LineIntersectsLeaf(*node->GetLeafData(), info)
                       : LineIntersectsOBBTree(node->GetLeftNode(), node->GetRightNode(), info);
  node->SetHit(true);
  return hit;
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* left,
                                               const COBBTree::CNode* right,
                                               CRayCastInfo& info) const {
  // TODO: Visit the nearer child first and prune the farther child after updating the hit time.
  return false;
}

bool CCollidableOBBTree::LineIntersectsLeaf(const COBBTree::CLeafData& leaf,
                                            CRayCastInfo& info) const {
  // TODO: Intersect filtered triangles, retaining the nearest plane and triangle material.
  return false;
}

uint CCollidableOBBTree::GetTableIndex() const { return sTableIndex; }
