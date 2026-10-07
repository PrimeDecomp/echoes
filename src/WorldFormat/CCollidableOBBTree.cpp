#include "WorldFormat/CCollidableOBBTree.hpp"

#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "WorldFormat/CCollisionCache.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include <math.h>

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
                                              const CTransform4f& xf, const CAABox& aabb,
                                              const CMaterialList& material,
                                              const CMaterialFilter& filter, const CPlane* planes,
                                              CCollisionInfoList& infoList) const {
  CVector3f center = aabb.GetCenterPoint();
  CVector3f extent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;

  int surfCount = leaf.GetSurfaceVector().size();
  bool ret = false;
  for (int i = 0; i < surfCount; ++i) {
    CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
    if (filter.Passes(triMat) && CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0),
                                                              surf.GetVert(1), surf.GetVert(2))) {
      mHits += 1;
      CAABox newAABB = CAABox::MakeMaxInvertedBox();
      if (CMetroidAreaCollider::ConvexPolyCollision(planes, &surf.GetVert(0), newAABB)) {
        CPlane plane = surf.GetPlane();
        CCollisionInfo info(newAABB, material,
                            CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                            plane.GetNormal(), -plane.GetNormal(), -1);
        infoList.Add(info);
        ret = true;
      }
    }
  }

  return ret;
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
  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
    CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
    if (filter.Passes(triMat)) {
      mHits += 1;
      if (CollisionUtil::TriSphereIntersection(sphere, surf.GetVert(0), surf.GetVert(1),
                                               surf.GetVert(2), point, normal)) {
        infoList.Add(CCollisionInfo(point, material,
                                    CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                                    normal, -1));
        ret = true;
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::AABoxCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                               const CAABox& aabb, const COBBox& obb,
                                               const CMaterialFilter& filter) const {
  CVector3f center = aabb.GetCenterPoint();
  CVector3f extent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;

  mTries += 1;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
      int surfCount = leaf.GetSurfaceVector().size();
      for (int i = 0; i < surfCount; ++i) {
        CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
        CMaterialList triMat(surf.GetSurfaceFlags());
        if (filter.Passes(triMat) &&
            CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                         surf.GetVert(2))) {
          return true;
        }
      }
    } else {
      if (node.GetLeftNode() && AABoxCollisionBoolean(*node.GetLeftNode(), xf, aabb, obb, filter))
        return true;
      if (node.GetRightNode() && AABoxCollisionBoolean(*node.GetRightNode(), xf, aabb, obb, filter))
        return true;
    }
  } else {
    mMisses += 1;
  }

  return false;
}

bool CCollidableOBBTree::SphereCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                                const CSphere& sphere, const COBBox& obb,
                                                const CMaterialFilter& filter) const {
  mTries += 1;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
      int surfCount = leaf.GetSurfaceVector().size();
      for (int i = 0; i < surfCount; ++i) {
        CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
        CMaterialList triMat(surf.GetSurfaceFlags());
        if (filter.Passes(triMat) &&
            CollisionUtil::TriSphereOverlap(sphere, surf.GetVert(0), surf.GetVert(1),
                                            surf.GetVert(2))) {
          return true;
        }
      }
    } else {
      if (node.GetLeftNode() &&
          SphereCollisionBoolean(*node.GetLeftNode(), xf, sphere, obb, filter))
        return true;
      if (node.GetRightNode() &&
          SphereCollisionBoolean(*node.GetRightNode(), xf, sphere, obb, filter))
        return true;
    }
  } else {
    mMisses += 1;
  }

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
    const COBBTree::CLeafData& leaf, const CTransform4f& xf, const CAABox& aabb,
    const CMaterialList& material, const CMaterialFilter& filter,
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& dir,
    double& dOut, CCollisionInfo& info) const {
  CVector3f normal(CVector3f::Zero());
  CVector3f point(CVector3f::Zero());

  CAABox movedAABB = components.mAabb;
  CVector3f moveVec = static_cast< float >(dOut) * dir;
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  bool ret = false;
  CVector3f extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
    int triIdx = leaf.GetSurfaceVector()[i];
    CCollisionSurface surf = GetOBBTree().GetTriangle(triIdx, &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
    if (filter.Passes(triMat)) {
      if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                       surf.GetVert(2))) {
        mHits += 1;

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);

        double d = dOut;
        if (CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
                surf, aabb, components.mVertIdxs, dir, d, normal, point) &&
            d < dOut) {
          info = CCollisionInfo(point, material,
                                CMaterialList(triMat.GetValue() | GetMaterial().GetValue()), normal,
                                -1);
          ret = true;
          dOut = d;
        }

        for (int k = 0; k < 3; ++k) {
          uint vertIdx = vertIndices[k];
          u64 vertMatVal = GetOBBTree().GetVertMaterial(vertIdx);
          if (!(vertMatVal & (u64(1) << kMT_NoEdgeCollision)) &&
              CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
            CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                CMetroidAreaCollider::GetDupPrimitiveCheckCount();
            if (movedAABB.PointInside(surf.GetVert(k))) {
              d = dOut;
              if (CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(
                      surf.GetVert(k), aabb, dir, d, normal, point) &&
                  d < dOut) {
                info = CCollisionInfo(point, material,
                                      CMaterialList(vertMatVal | GetMaterial().GetValue()), normal,
                                      -1);
                ret = true;
                dOut = d;
              }
            }
          }
        }

        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        for (int k = 0; k < 3; ++k) {
          uint edgeIdx = edgeIndices[k];
          if (CMetroidAreaCollider::DupEdgeListValue(edgeIdx) !=
              CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
            CMetroidAreaCollider::DupEdgeListValue(edgeIdx) =
                CMetroidAreaCollider::GetDupPrimitiveCheckCount();
            u64 edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
            if (!(edgeMatVal & (u64(1) << kMT_NoEdgeCollision))) {
              int nextVert = k == 2 ? 0 : k + 1;
              d = dOut;
              if (CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(
                      surf.GetVert(k), surf.GetVert(nextVert), components.mEdges, dir, d, normal,
                      point) &&
                  d < dOut) {
                info = CCollisionInfo(point, material,
                                      CMaterialList(edgeMatVal | GetMaterial().GetValue()), normal,
                                      -1);
                ret = true;
                dOut = d;
              }
            }
          }
        }
      } else {
        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
        CMetroidAreaCollider::DupVertexListValue(vertIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::CacheTree(CCollisionCacheWriter& writer, const COBBTree::CNode& node,
                                   const CTransform4f& xf, const CVector3f& center,
                                   const CVector3f& halfExtent, const COBBox& obb) const {
  ++mTries;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
      writer.BeginLeaf(CAABox::MakeMaxInvertedBox());
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
      int count = leaf.GetSurfaceVector().size();
      writer.ReserveTriangles(count);
      for (int i = 0; i < count; ++i) {
        int index = leaf.GetSurfaceVector()[i];
        CCollisionSurface surface = GetOBBTree().GetTriangle(index, &xf);
        if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                         surface.GetVert(2))) {
          writer.AddTriangle(surface, index);
        }
      }
    } else {
      if (node.GetLeftNode()) {
        CacheTree(writer, *node.GetLeftNode(), xf, center, halfExtent, obb);
      }
      if (node.GetRightNode()) {
        CacheTree(writer, *node.GetRightNode(), xf, center, halfExtent, obb);
      }
    }
  } else {
    ++mMisses;
  }
  return false;
}

void CCollidableOBBTree::CacheSphere(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                                     u64 material) {
  float scale = xf.GetRight().Magnitude();
  COBBTree* tree;
  if (scale < 2.f) {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereLow);
  } else if (scale < 5.f) {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereMedium);
  } else {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereHigh);
  }

  CCollidableOBBTree primitive(tree, CMaterialList(material));
  float invScale = 1.f / scale;
  CVector3f center = cache.GetBounds().GetCenterPoint();
  COBBox obb(CTransform4f::Translate((center - xf.GetTranslation()) * invScale),
             (cache.GetBounds().GetMaxPoint() - center) * invScale);
  CVector3f worldCenter = cache.GetBounds().GetCenterPoint();
  CVector3f halfExtent = (cache.GetBounds().GetMaxPoint() - cache.GetBounds().GetMinPoint()) * 0.5f;
  CCollisionCacheWriter writer(cache);
  writer.BeginGeometry(*tree, &xf, ownerId, material);
  primitive.CacheTree(writer, *tree->GetRoot(), xf, worldCenter, halfExtent, obb);
}

void CCollidableOBBTree::CacheAABox(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                                    u64 material) {
  COBBTree* tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitCube);
  CCollisionCacheWriter writer(cache);
  writer.BeginGeometry(*tree, &xf, ownerId, material);
  writer.ReserveTriangles(tree->GetTriangleCount());
  CVector3f center = cache.GetBounds().GetCenterPoint();
  CVector3f halfExtent = (cache.GetBounds().GetMaxPoint() - cache.GetBounds().GetMinPoint()) * 0.5f;
  for (ushort i = 0; i < tree->GetTriangleCount(); ++i) {
    CCollisionSurface surface = tree->GetTriangle(i, &xf);
    if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2))) {
      writer.AddTriangle(surface, i);
    }
  }
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

static inline CVector3f TriangleEdgeNormal(const CVector3f& normal, const CVector3f& edge) {
  return CVector3f::Cross(normal, edge);
}

bool CCollidableOBBTree::SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf,
                                                     const CTransform4f& xf, const CSphere& sphere,
                                                     const CMaterialList& material,
                                                     const CMaterialFilter& filter,
                                                     const CVector3f& dir, double& dOut,
                                                     CCollisionInfo& info) const {
  static int mod3[4] = {0, 1, 2, 0};

  float radius = sphere.GetRadius();
  CVector3f radiusVec(radius, radius, radius);
  CAABox aabb(sphere.GetCenter() - radiusVec, sphere.GetCenter() + radiusVec);

  CVector3f moveVec = static_cast< float >(dOut) * dir;
  CAABox moveAABB = aabb;
  moveAABB.AccumulateBounds(moveAABB.GetMaxPoint() + moveVec);
  moveAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);

  CVector3f boxCenter = moveAABB.GetCenterPoint();
  bool ret = false;
  CVector3f extent = (moveAABB.GetMaxPoint() - moveAABB.GetMinPoint()) * 0.5f;

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
    ushort triIdx = leaf.GetSurfaceVector()[i];
    CCollisionSurface surf = GetOBBTree().GetTriangle(triIdx, &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
    if (filter.Passes(triMat)) {
      if (CollisionUtil::TriBoxOverlap(boxCenter, extent, surf.GetVert(0), surf.GetVert(1),
                                       surf.GetVert(2))) {
        mHits += 1;

        CVector3f surfNormal = surf.GetNormal();
        CVector3f toMovedSphere = sphere.GetCenter() + moveVec - surf.GetVert(0);
        if (!(CVector3f::Dot(toMovedSphere, surfNormal) > sphere.GetRadius())) {
          double mag =
              sphere.GetRadius() - CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0), surfNormal);
          mag /= CVector3f::Dot(dir, surfNormal);

          CVector3f intersectPoint = sphere.GetCenter() + static_cast< float >(mag) * dir;
          bool outsideEdges[3];
          outsideEdges[0] =
              CVector3f::Dot(intersectPoint - surf.GetVert(0),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(1) - surf.GetVert(0))) <
              0.f;
          outsideEdges[1] =
              CVector3f::Dot(intersectPoint - surf.GetVert(1),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(2) - surf.GetVert(1))) <
              0.f;
          outsideEdges[2] =
              CVector3f::Dot(intersectPoint - surf.GetVert(2),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(0) - surf.GetVert(2))) <
              0.f;

          if (mag >= 0.0 && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] &&
              mag < dOut) {
            info = CCollisionInfo(intersectPoint - sphere.GetRadius() * surfNormal, material,
                                  CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                                  surfNormal, -1);
            ret = true;
            dOut = mag;
          }

          const CVector3f& vts2 = sphere.GetCenter() - surf.GetVert(0);
          bool intersects = CVector3f::Dot(vts2, surfNormal) <= sphere.GetRadius();
          bool testVert[3] = {true, true, true};
          const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
          for (int k = 0; k < 3; ++k) {
            if (intersects || outsideEdges[k]) {
              uint edgeIdx = edgeIndices[k];
              if (CMetroidAreaCollider::DupEdgeListValue(edgeIdx) !=
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
                CMetroidAreaCollider::DupEdgeListValue(edgeIdx) =
                    CMetroidAreaCollider::GetDupPrimitiveCheckCount();
                u64 edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
                if (!(edgeMatVal & (u64(1) << kMT_NoEdgeCollision))) {
                  CVector3f edgeVec = surf.GetVert(mod3[k + 1]) - surf.GetVert(k);
                  float edgeVecMag = edgeVec.Magnitude();
                  edgeVec *= 1.f / edgeVecMag;

                  CVector3f vertToSphere = sphere.GetCenter() - surf.GetVert(k);
                  float vtsDotEdge = CVector3f::Dot(vertToSphere, edgeVec);
                  float dirDotEdge = CVector3f::Dot(dir, edgeVec);
                  CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                  CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                  float edgeRejMagSq = edgeRej.MagSquared();

                  if (edgeRejMagSq > 0.f) {
                    float b = 2.f * CVector3f::Dot(vtsRej, edgeRej);
                    float discriminant =
                        b * b - 4.f * edgeRejMagSq *
                                    (vtsRej.MagSquared() - sphere.GetRadius() * sphere.GetRadius());
                    if (discriminant >= 0.f) {
                      double inverse = 0.5 / edgeRejMagSq;
                      double mag2 = inverse * (-static_cast< double >(b) - sqrt(discriminant));
                      if (mag2 >= 0.0) {
                        double t = mag2 * dirDotEdge + vtsDotEdge;
                        if (t >= 0.0 && t <= edgeVecMag && mag2 < dOut) {
                          CVector3f point = surf.GetVert(k) + static_cast< float >(t) * edgeVec;
                          CVector3f normal =
                              (sphere.GetCenter() + static_cast< float >(mag2) * dir - point)
                                  .AsNormalized();
                          info = CCollisionInfo(
                              point, material, CMaterialList(edgeMatVal | GetMaterial().GetValue()),
                              normal, -1);
                          dOut = mag2;
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

          ushort vertIndices[3];
          GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
          for (int k = 0; k < 3; ++k) {
            uint vertIdx = vertIndices[k];
            if (testVert[k]) {
              u64 vertMatVal = GetOBBTree().GetVertMaterial(vertIdx);
              if (!(vertMatVal & (u64(1) << kMT_NoEdgeCollision)) &&
                  CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
                      CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
                CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                    CMetroidAreaCollider::GetDupPrimitiveCheckCount();
                double d = dOut;
                if (CollisionUtil::RaySphereIntersection_Double(
                        CSphere(surf.GetVert(k), sphere.GetRadius()), sphere.GetCenter(), dir, d) &&
                    d >= 0.0) {
                  float dF = static_cast< float >(d);
                  CVector3f normal =
                      (sphere.GetCenter() + dF * dir - surf.GetVert(k)).AsNormalized();
                  info = CCollisionInfo(surf.GetVert(k), material,
                                        CMaterialList(vertMatVal | GetMaterial().GetValue()),
                                        normal, -1);
                  dOut = d;
                  ret = true;
                }
              }
            } else {
              CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount();
            }
          }
        }
      } else {
        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
        CMetroidAreaCollider::DupVertexListValue(vertIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
      }
    }
  }

  return ret;
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
  return CRayCastResult::MakeInvalid();
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* node,
                                               CRayCastInfo& info) const {
  if (!node) {
    return false;
  }

  ++mTries;
  float time;
  if (!node->GetOBB().LineIntersectsBox(info.GetRay(), time) || !(time < info.GetMagnitude())) {
    ++mMisses;
    return false;
  }

  const bool hit = node->IsLeaf()
                       ? LineIntersectsLeaf(*node->GetLeafData(), info)
                       : LineIntersectsOBBTree(node->GetLeftNode(), node->GetRightNode(), info);
  node->SetHit(true);
  return hit;
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* n0, const COBBTree::CNode* n1,
                                               CRayCastInfo& info) const {
  bool ret = false;
  float t0, t1;
  bool intersects0 = false;

  mTries += 2;

  if (n0 && n0->GetOBB().LineIntersectsBox(info.GetRay(), t0) == true && t0 < info.GetMagnitude())
    intersects0 = true;

  bool intersects1 = false;
  if (n1 && n1->GetOBB().LineIntersectsBox(info.GetRay(), t1) == true && t1 < info.GetMagnitude())
    intersects1 = true;

  if (intersects0 && intersects1) {
    if (t0 < t1) {
      if ((n0->IsLeaf() == true
               ? LineIntersectsLeaf(*n0->GetLeafData(), info)
               : LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info)) == true) {
        if (info.GetMagnitude() < t1)
          return true;
        ret = true;
      }
      if (n1->IsLeaf()) {
        if (LineIntersectsLeaf(*n1->GetLeafData(), info))
          ret = true;
      } else {
        if (LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info) == true)
          ret = true;
      }
    } else {
      if ((n1->IsLeaf() == true
               ? LineIntersectsLeaf(*n1->GetLeafData(), info)
               : LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info)) == true) {
        if (info.GetMagnitude() < t0)
          return true;
        ret = true;
      }
      if (n0->IsLeaf()) {
        if (LineIntersectsLeaf(*n0->GetLeafData(), info))
          ret = true;
      } else {
        if (LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info) == true)
          ret = true;
      }
    }
  } else {
    if (intersects0) {
      if (n0->IsLeaf() == true) {
        if (LineIntersectsLeaf(*n0->GetLeafData(), info))
          return true;
      } else {
        if (LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info) == true)
          return true;
      }
    }
    if (intersects1) {
      if (n1->IsLeaf() == true) {
        if (LineIntersectsLeaf(*n1->GetLeafData(), info))
          return true;
      } else {
        if (LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info) == true)
          return true;
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::LineIntersectsLeaf(const COBBTree::CLeafData& leaf,
                                            CRayCastInfo& info) const {
  ushort intersectIdx = 0;
  bool ret = false;
  int surfCount = leaf.GetSurfaceVector().size();
  const CMaterialFilter& filter = info.GetMaterialFilter();
  for (ushort i = 0; i < surfCount; ++i) {
    const CCollisionSurface& surface = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i]);
    CMaterialList matList(surface.GetSurfaceFlags());
    if (filter.Passes(matList)) {
      if (CollisionUtil::RayTriangleIntersection(info.GetRay().GetStart(),
                                                 info.GetRay().GetDirection(), &surface.GetVert(0),
                                                 info.Magnitude())) {
        intersectIdx = i;
        ret = true;
      }
    }
  }

  if (ret) {
    const CCollisionSurface& surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[intersectIdx]);
    info.Plane() = surf.GetPlane();
    info.Material() = CMaterialList(surf.GetSurfaceFlags());
  }

  return ret;
}

uint CCollidableOBBTree::GetTableIndex() const { return sTableIndex; }
