#include "MetroidPrime/Weapons/CIceImpact.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "WorldFormat/COBBTree.hpp"

#include "rstl/math.hpp"

CMarkerGrid::CMarkerGrid(const CAABox& bounds)
: mBounds(bounds)
, mGridUnits((mBounds.GetMaxPoint() - mBounds.GetMinPoint()) * 0.0625f)
, mGridState(0) {}

uint CMarkerGrid::GetValue(uint x, uint y, uint z) const {
  const uint bitOffset = (x & 3) << 1;
  const uint gridOffset = (y << 2) + (z << 6) + (x >> 2);
  return (mGridState[gridOffset] & (3U << bitOffset)) >> bitOffset;
}

bool CMarkerGrid::GetCoords(const CVector3f& point, uint& x, uint& y, uint& z) const {
  if (!mBounds.PointInside(point)) {
    return false;
  }

  const CVector3f& relative = point - mBounds.GetMinPoint();
  x = relative.GetX() / mGridUnits.GetX();
  y = relative.GetY() / mGridUnits.GetY();
  z = relative.GetZ() / mGridUnits.GetZ();
  return true;
}

void CMarkerGrid::SetValue(uint x, uint y, uint z, uint value) {
  const uint bitOffset = (x & 3) << 1;
  const uint gridOffset = (y << 2) + (z << 6) + (x >> 2);
  mGridState[gridOffset] = (mGridState[gridOffset] & ~(3 << bitOffset)) | (value << bitOffset);
}

bool CMarkerGrid::AABoxTouchesData(const CAABox& bounds, uint value) const {
  if (!mBounds.DoBoundsOverlap(bounds)) {
    return false;
  }

  CAABox clipped = bounds;
  if (!clipped.Inside(mBounds)) {
    CVector3f min = mBounds.GetMinPoint();
    min.SetX(rstl::max_val(min.GetX(), bounds.GetMinPoint().GetX()));
    min.SetY(rstl::max_val(min.GetY(), bounds.GetMinPoint().GetY()));
    min.SetZ(rstl::max_val(min.GetZ(), bounds.GetMinPoint().GetZ()));
    CVector3f max = mBounds.GetMaxPoint();
    max.SetX(rstl::min_val(max.GetX(), bounds.GetMaxPoint().GetX()));
    max.SetY(rstl::min_val(max.GetY(), bounds.GetMaxPoint().GetY()));
    max.SetZ(rstl::min_val(max.GetZ(), bounds.GetMaxPoint().GetZ()));
    clipped = CAABox(min, max);
  }

  uint minX, minY, minZ;
  uint maxX, maxY, maxZ;
  GetCoords(clipped.GetMinPoint(), minX, minY, minZ);
  GetCoords(clipped.GetMaxPoint(), maxX, maxY, maxZ);
  for (uint z = minZ; z < maxZ; ++z) {
    for (uint y = minY; y < maxY; ++y) {
      for (uint x = minX; x < maxX; ++x) {
        if (value & GetValue(x, y, z)) {
          return true;
        }
      }
    }
  }
  return false;
}

void CMarkerGrid::MarkCells(const CSphere& sphere, uint value) {
  const int width =
      static_cast< int >((sphere.GetRadius() - mGridUnits.GetX()) / mGridUnits.GetX());
  const int length =
      static_cast< int >((sphere.GetRadius() - mGridUnits.GetY()) / mGridUnits.GetY());
  const int height =
      static_cast< int >((sphere.GetRadius() - mGridUnits.GetZ()) / mGridUnits.GetZ());
  uint x, y, z;
  if (!GetCoords(sphere.GetCenter(), x, y, z)) {
    return;
  }
  for (uint k = z - width; k < z + width; ++k) {
    for (uint j = y - length; j < y + length; ++j) {
      for (uint i = x - height; i < x + height; ++i) {
        SetValue(i, j, k, value | GetValue(x, y, z));
      }
    }
  }
}

CVector3f CMarkerGrid::GetWorldPositionForCell(uint x, const uint y, uint z) const {
  return CVector3f(x * mGridUnits.GetX(), y * mGridUnits.GetY(), z * mGridUnits.GetZ()) +
         mBounds.GetMinPoint() + 0.5f * mGridUnits;
}

CIceImpact::CIceImpact(const TLockedToken< CGenDescription >& particle, TUniqueId uid, TAreaId aid,
                       TUniqueId ownerId, bool active, const rstl::string& name,
                       const CTransform4f& xf, uint flags, const CVector3f& scale,
                       const CColor& color, float boundScale)
: CEffect(uid, CEntityInfo(aid, CEntity::NullConnectionList, active), name, xf)
, mElementGen(rs_new CElementGen(TToken< CGenDescription >(particle), CElementGen::kMOT_One,
                                 CElementGen::kOSF_One))
, mLightId(kInvalidUniqueId)
, mGenAssetId(TToken< CGenDescription >(particle).GetTag().GetId())
, mOwnerId(ownerId)
, mLifeTimer(0.f)
, mLatestDamageTime(4.f)
, mSearchDirection(0)
, mHalfBounds(boundScale * scale.GetX())
, mParticleRemainder(0.f)
, mSphereGenRange(GetTranslation(), mHalfBounds - 1.6f)
, mGrid(CAABox(xf.GetTranslation() - CVector3f(mHalfBounds, mHalfBounds, mHalfBounds),
               xf.GetTranslation() + CVector3f(mHalfBounds, mHalfBounds, mHalfBounds)))
, mFollowPlayerArea(flags & 2)
, mHasRenderBounds(false) {
  mImpactSpheres.push_back(SImpactSphere(GetTranslation(), 2.4f, 1.6f, 0.f, 0.f));
  for (int i = 1; i < 3; ++i) {
    mImpactSpheres.push_back(SImpactSphere(GetTranslation(), 0.f, 1.f, 0.f, 0.f));
    mImpactSpheres[i].mPreviousRadius = mImpactSpheres[i].mRadius;
    mImpactSpheres[i].mRadius += mImpactSpheres[i].mRadiusStep;
  }
  mGrid.MarkCells(CSphere(GetTranslation(), 2.4f), 2);
}

CIceImpact::~CIceImpact() {}

void CIceImpact::PreRenderAllViewports(CStateManager& mgr) {
  const rstl::optional_object< CAABox > bounds = mElementGen->GetBounds();
  if (bounds) {
    mHasRenderBounds = true;
    SetRenderBounds(*bounds);
  } else {
    const CVector3f pos = GetTranslation();
    mHasRenderBounds = false;
    SetRenderBounds(CAABox(pos, pos));
  }
  // TODO: propagate the calculated box to the actor's other-bounds cache.
  UpdatePortalSystemState(mgr);
}

void CIceImpact::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  SetPreRenderClipped(!mHasRenderBounds || GetPreRenderClipped());
}

void CIceImpact::AddToRenderer(const CStateManager& mgr) const {
  gpRender->AddParticleGen(*mElementGen);
}

void CIceImpact::Think(float dt, CStateManager& mgr) {
  mLifeTimer += dt;
  if (mLifeTimer < 0.8f && mElementGen->GetParticleCount() < 400) {
    for (int i = 0; i < mImpactSpheres.size(); ++i) {
      SImpactSphere& sphere = mImpactSpheres[i];
      if (sphere.mRadius > sphere.mMaxRadius) {
        const rstl::optional_object< SImpactSphere > newSphere = GenerateNewSphere();
        if (newSphere) {
          sphere = *newSphere;
        }
      }
    }
    for (int i = 0; i < mImpactSpheres.size(); ++i) {
      SImpactSphere& sphere = mImpactSpheres[i];
      if (!(sphere.mRadius > sphere.mMaxRadius)) {
        sphere.mPreviousRadius = sphere.mRadius;
        sphere.mRadius += sphere.mRadiusStep;
        const CSphere outer(sphere.mPos, sphere.mRadius);
        const CSphere inner(sphere.mPos, sphere.mPreviousRadius);
        const CVector3f& pos = outer.GetCenter();
        const float radius = outer.GetRadius();
        const CAABox bounds(
            CVector3f(pos.GetX() - radius, pos.GetY() - radius, pos.GetZ() - radius),
            CVector3f(pos.GetX() + radius, pos.GetY() + radius, pos.GetZ() + radius));
        mParticleRemainder = 0.f;
        GenerateParticlesAgainstActors(mgr, bounds, outer, inner);
        CAreaCollisionCache cache(bounds);
        CGameCollision::BuildAreaCollisionCache(mgr, cache);
        for (int j = 0; j < static_cast< int >(cache.GetNumCaches()); ++j) {
          GenerateParticlesAgainstWorld(mgr, cache.GetOctreeLeafCache(j), outer, inner);
        }
      }
    }
  }

  mElementGen->SetOrientation(CTransform4f::Identity());
  mElementGen->Update(dt);

  if (mLightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
      if (GetActive()) {
        light->SetLight(mElementGen->GetLight());
      }
    }
  }

  if (mFollowPlayerArea) {
    mgr.SetActorAreaId(*this, mgr.GetPlayer(0)->GetCurrentAreaId());
  }

  if (mElementGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CIceImpact::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();
  switch (message) {
  case kSM_Create:
    if (mElementGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      const CAssetId sourceId = mGenAssetId;
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mElementGen->GetLight(),
                                      sourceId, 1, 0.f));
    }
    break;
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
  if (mLightId != kInvalidUniqueId) {
    mgr.SendScriptMsg(mLightId, sender, message);
  }
}

rstl::optional_object< CAABox > CIceImpact::GetTouchBounds() const { return mGrid.GetBounds(); }

void CIceImpact::Touch(CActor& actor, CStateManager& mgr) {
  if (mLifeTimer > mLatestDamageTime) {
    return;
  }
  if (actor.GetUniqueId() != mOwnerId) {
    const rstl::optional_object< CAABox > touchBounds = actor.GetTouchBounds();
    if (!touchBounds) {
      return;
    }

    const CDamageInfo damageInfo(CWeaponMode(kWT_Dark, true), 1.6666666f, 0.1f, 1.f);
    if (CPatterned* patterned = TCastToPtr< CPatterned >(&actor)) {
      const CAABox expanded(touchBounds->GetMinPoint() - CVector3f(0.f, 0.f, 0.5f),
                            touchBounds->GetMaxPoint() + CVector3f(0.f, 0.f, 0.5f));
      if (mGrid.AABoxTouchesData(expanded, 1)) {
        if (patterned->BodyController()->GetPercentageFrozen() == 0.f) {
          mgr.ApplyRadiusDamage(*this, expanded.GetCenterPoint(), actor, kInvalidUniqueId,
                                damageInfo);
        } else {
          patterned->BodyController()->Freeze(0.f, 8.f, 0.f);
        }
      }
    }

    if (TCastToPtr< CScriptDamageableTrigger >(&actor) != nullptr) {
      if (mGrid.AABoxTouchesData(*touchBounds, 1)) {
        mgr.ApplyDamage(
            GetUniqueId(), actor.GetUniqueId(), kInvalidUniqueId, damageInfo,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
            CVector3f::Zero());
      }
    }

    if (CPlayer* player = TCastToPtr< CPlayer >(&actor)) {
      mgr.SendScriptMsg(player->GetUniqueId(), kInvalidUniqueId, kSM_XINS);
    }
  }
}

static bool pointInSphere(const CSphere& sphere, const CVector3f& point) {
  const CVector3f& delta = sphere.GetCenter() - point;
  return delta.MagSquared() <= sphere.GetRadius() * sphere.GetRadius();
}

rstl::optional_object< CIceImpact::SImpactSphere > CIceImpact::GenerateNewSphere() {
  mSearchDirection = (mSearchDirection + 1) & 7;
  const bool forwardZ = (mSearchDirection & 1) != 0;
  const bool forwardY = (mSearchDirection & 2) != 0;
  const bool forwardX = (mSearchDirection & 4) != 0;
  for (uint z = 8; forwardZ ? z < 14 : z >= 1; forwardZ ? ++z : --z) {
    for (uint y = 8; forwardY ? y < 14 : y >= 1; forwardY ? ++y : --y) {
      for (uint x = 8; forwardX ? x < 14 : x >= 1; forwardX ? ++x : --x) {
        if (mGrid.GetValue(x, y, z) == 1) {
          const CVector3f pos = mGrid.GetWorldPositionForCell(x, y, z);
          mGrid.SetValue(x, y, z, 3);
          if (pointInSphere(mSphereGenRange, pos)) {
            mGrid.MarkCells(CSphere(pos, 1.6f), 2);
            return SImpactSphere(pos, 1.6f, 1.6f, 0.f, 0.f);
          }
        }
      }
    }
  }
  return rstl::optional_object_null();
}

bool CIceImpact::GenerateParticlesAgainstWorld(CStateManager& mgr,
                                               const CMetroidAreaCollider::COctreeLeafCache& cache,
                                               const CSphere& outer, const CSphere& inner) {
  CMetroidAreaCollider::ResetInternalCounters();
  const CMaterialFilter filter =
      CMaterialFilter::MakeExclude(CMaterialList(kMT_NoPlatformCollision));
  for (int n = 0; n < cache.GetNumLeaves(); ++n) {
    const CAreaOctTree::Node& leaf = cache.GetLeaf(n);
    const CAreaOctTree::TriListReference triangles = leaf.GetTriangleArray();
    const CAreaOctTree& owner = leaf.GetOwner();
    const int triangleCount = triangles.GetSize();
    bool done = false;
    for (int i = 0; i < triangleCount && !done; ++i) {
      int index = triangles.GetAt(i);
      if (CMetroidAreaCollider::DupTriangleListValue(index) !=
          CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
        CMetroidAreaCollider::DupTriangleListValue(index) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        const CCollisionSurface surface(owner.GetTriangle(index));
        if (filter.Passes(CMaterialList(surface.GetSurfaceFlags()))) {
          const CVector3f a = surface.GetVert(0);
          const CVector3f b = surface.GetVert(1);
          const CVector3f c = surface.GetVert(2);
          done = SubdivideAndGenerateParticles(mgr, a, b, c, outer, inner);
        }
      }
    }
  }
  return false;
}

bool CIceImpact::GenerateParticlesAgainstActors(CStateManager& mgr, const CAABox& bounds,
                                                const CSphere& outer, const CSphere& inner) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(
      nearList, bounds,
      CMaterialFilter::MakeExclude(CMaterialList(kMT_Character, kMT_Player, kMT_Projectile,
                                                 kMT_NoPlatformCollision, kMT_AIJoint)),
      this);
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it));
    CPhysicsActor* physActor = TCastToPtr< CPhysicsActor >(actor);
    if (physActor != nullptr && physActor->GetCollisionPrimitive()->GetPrimType() == 'OBTG') {
      const CCollidableOBBTreeGroup* group =
          static_cast< const CCollidableOBBTreeGroup* >(physActor->GetCollisionPrimitive());
      for (int i = 0; i < group->GetContainer()->NumTrees(); ++i) {
        GenerateParticlesAgainstOBBTree(mgr, *group->GetOBBTree(i),
                                        physActor->GetPrimitiveTransform(), outer, inner);
      }
    } else if (actor != nullptr) {
      if (actor->GetMaterialList().HasMaterial(kMT_Unknown59) ||
          TCastToPtr< CScriptWater >(actor) != nullptr) {
        const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
        if (touchBounds) {
          GenerateParticlesAgainstAABox(mgr, *touchBounds, outer, inner);
        }
      }
    }
  }
  return true;
}

bool CIceImpact::GenerateParticlesAgainstAABox(CStateManager& mgr, const CAABox& bounds,
                                               const CSphere& outer, const CSphere& inner) {
  for (int i = 0; i < 12; ++i) {
    const CTri tri = bounds.GetTri(static_cast< CAABox::EBoxFaceId >(i / 2), (i & 1) * 2);
    if (SubdivideAndGenerateParticles(mgr, tri.GetPointA(), tri.GetPointC(), tri.GetPointB(), outer,
                                      inner)) {
      break;
    }
  }
  return false;
}

bool CIceImpact::GenerateParticlesAgainstOBBTree(CStateManager& mgr, const COBBTree& tree,
                                                 const CTransform4f& xf, const CSphere& outer,
                                                 const CSphere& inner) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeExclude(CMaterialList(kMT_NoPlatformCollision));
  const int triangleCount = tree.GetTriangleCount();
  for (short i = 0; i < triangleCount; ++i) {
    const CCollisionSurface surface(tree.GetTriangle(i, &xf));
    if (filter.Passes(CMaterialList(surface.GetSurfaceFlags())) &&
        SubdivideAndGenerateParticles(mgr, surface.GetVert(0), surface.GetVert(1),
                                      surface.GetVert(2), outer, inner)) {
      break;
    }
  }
  return false;
}

bool CIceImpact::SubdivideAndGenerateParticles(CStateManager& mgr, const CVector3f& a,
                                               const CVector3f& b, const CVector3f& c,
                                               const CSphere& outer, const CSphere& inner) {
  if (!CollisionUtil::TriSphereOverlap(outer, a, b, c)) {
    return false;
  }
  if (pointInSphere(inner, a) && pointInSphere(inner, b) && pointInSphere(inner, c)) {
    return false;
  }

  const CVector3f& edgeAB = b - a;
  const CVector3f edgeAC = c - a;
  const CVector3f cross = CVector3f::Cross(edgeAB, edgeAC);
  const float area = cross.Magnitude();
  if (area > 1.f) {
    const CVector3f center =
        CMath::BaryToWorld(a, b, c, CVector3f(1.f / 3.f, 1.f / 3.f, 1.f / 3.f));
    SubdivideAndGenerateParticles(mgr, a, b, center, outer, inner);
    SubdivideAndGenerateParticles(mgr, b, c, center, outer, inner);
    SubdivideAndGenerateParticles(mgr, c, a, center, outer, inner);
    return false;
  }

  mParticleRemainder += area;
  const int count = static_cast< int >(mParticleRemainder);
  mParticleRemainder -= count;
  for (int i = 0; i < count; ++i) {
    float rx = mgr.Random()->Float();
    float ry = mgr.Random()->Float();
    float rz = mgr.Random()->Float();
    const float inv = 1.f / (rx + ry + rz);
    rx *= inv;
    ry *= inv;
    rz *= inv;
    const CVector3f point = CMath::BaryToWorld(a, b, c, CVector3f(rx, ry, rz));
    uint x, y, z;
    if (pointInSphere(inner, point) || !pointInSphere(outer, point) ||
        !mGrid.GetCoords(point, x, y, z) || (mGrid.GetValue(x, y, z) & 1)) {
      continue;
    }
    mGrid.SetValue(x, y, z, 1);

    CVector3f direction = CVector3f::Zero();
    switch (mgr.Random()->Range(0, 2)) {
    case 0:
      direction = a - point;
      if (!direction.CanBeNormalized()) {
        direction = b - point;
      }
      break;
    case 1:
      direction = b - point;
      if (!direction.CanBeNormalized()) {
        direction = c - point;
      }
      break;
    case 2:
      direction = c - point;
      if (!direction.CanBeNormalized()) {
        direction = a - point;
      }
      break;
    }
    direction = direction.AsNormalized();
    CVector3f normal = cross.AsNormalized();
    normal[kDX] += 0.4f * (mgr.Random()->Float() - 0.5f);
    normal[kDY] += 0.4f * (mgr.Random()->Float() - 0.5f);
    normal[kDZ] += 0.4f * (mgr.Random()->Float() - 0.5f);
    mElementGen->SetOrientation(
        CTransform4f::LookAt(CVector3f::Zero(), normal.AsNormalized(), direction));
    mElementGen->SetTranslation(point);
    mElementGen->ForceParticleCreation(1);
    if (mElementGen->GetParticleCount() == mElementGen->mMAXP) {
      return true;
    }
  }
  return false;
}
