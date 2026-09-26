#ifndef _CCOLLIDABLEOBBTREE
#define _CCOLLIDABLEOBBTREE

#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/COBBTree.hpp"

class CMRay;
class CSphere;

// Guessed names for the shared packed collision cache and its writer.
class CCollisionCache;
class CCollisionCacheWriter;

class CRayCastInfo {
public:
  CRayCastInfo(const CMRay& ray, const CMaterialFilter& filter, float magnitude);

  const CMRay& GetRay() const { return mRay; }
  const CMaterialFilter& GetMaterialFilter() const { return mFilter; }
  float GetMagnitude() const { return mMagnitude; }
  float& Magnitude() { return mMagnitude; }
  const CPlane& GetPlane() const { return mPlane; }
  CPlane& Plane() { return mPlane; }
  const CMaterialList& GetMaterial() const { return mMaterial; }
  CMaterialList& Material() { return mMaterial; }

private:
  const CMRay& mRay;
  const CMaterialFilter& mFilter;
  float mMagnitude;
  CPlane mPlane;
  CMaterialList mMaterial;
};
CHECK_SIZEOF(CRayCastInfo, 0x28)

class CCollidableOBBTree : public CCollisionPrimitive {
public:
  CCollidableOBBTree(COBBTree* tree, const CMaterialList& material);

  // CCollisionPrimitive
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& xf) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  ~CCollidableOBBTree() override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& rayCast) const override;

  const COBBTree& GetOBBTree() const { return *mTree; }

  bool AABoxCollision(const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& box,
                      const COBBox& obb, const CMaterialList& material,
                      const CMaterialFilter& filter, const CPlane* planes,
                      CCollisionInfoList& infoList) const;
  bool AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                            const CAABox& box, const CMaterialList& material,
                            const CMaterialFilter& filter, const CPlane* planes,
                            CCollisionInfoList& infoList) const;
  bool SphereCollision(const COBBTree::CNode& node, const CTransform4f& xf, const CSphere& sphere,
                       const COBBox& obb, const CMaterialList& material,
                       const CMaterialFilter& filter, CCollisionInfoList& infoList) const;
  bool SphereCollideWithLeaf(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                             const CSphere& sphere, const CMaterialList& material,
                             const CMaterialFilter& filter, CCollisionInfoList& infoList) const;
  bool AABoxCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& box,
                             const COBBox& obb, const CMaterialFilter& filter) const;
  bool SphereCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                              const CSphere& sphere, const COBBox& obb,
                              const CMaterialFilter& filter) const;
  bool AABoxCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& box,
                            const COBBox& obb, const CMaterialList& material,
                            const CMaterialFilter& filter,
                            const CMetroidAreaCollider::CMovingAABoxComponents& components,
                            const CVector3f& direction, double& time, CCollisionInfo& info) const;
  bool AABoxCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                                  const CAABox& box, const CMaterialList& material,
                                  const CMaterialFilter& filter,
                                  const CMetroidAreaCollider::CMovingAABoxComponents& components,
                                  const CVector3f& direction, double& time,
                                  CCollisionInfo& info) const;
  bool SphereCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf,
                             const CSphere& sphere, const COBBox& obb,
                             const CMaterialList& material, const CMaterialFilter& filter,
                             const CVector3f& direction, double& time, CCollisionInfo& info) const;
  bool SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                                   const CSphere& sphere, const CMaterialList& material,
                                   const CMaterialFilter& filter, const CVector3f& direction,
                                   double& time, CCollisionInfo& info) const;

  CRayCastResult LineIntersectsTree(const CMRay& ray, const CMaterialFilter& filter, float maxTime,
                                    const CTransform4f& xf) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode* node, CRayCastInfo& info) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode* left, const COBBTree::CNode* right,
                             CRayCastInfo& info) const;
  bool LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const;

  // Guessed names for Echoes's cache-building additions.
  bool CacheTree(CCollisionCacheWriter& writer, const COBBTree::CNode& node, const CTransform4f& xf,
                 const CVector3f& center, const CVector3f& halfExtent, const COBBox& obb) const;
  static void CacheSphere(CCollisionCache& cache, const CTransform4f& xf, short ownerId,
                          u64 material);
  static void CacheAABox(CCollisionCache& cache, const CTransform4f& xf, short ownerId,
                         u64 material);

private:
  COBBTree* mTree;
  mutable uint mTries;
  mutable uint mMisses;
  mutable uint mHits;

  static uint sTableIndex;
};
CHECK_SIZEOF(CCollidableOBBTree, 0x20)

#endif // _CCOLLIDABLEOBBTREE
