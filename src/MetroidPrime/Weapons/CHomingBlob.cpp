#include "MetroidPrime/Weapons/CHomingBlob.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "WorldFormat/CCollisionCache.hpp"

// Guessed names for the target's writable rendering parameters.
static float sDistanceConstant = -5.f;
static float sAngleConstant = 5.f;
static float sAmbientRed = 1.f;
static float sAmbientGreen = 0.8f;
static float sAmbientBlue = 1.f;
static float sLightRed = 1.f;
static float sLightGreen = 1.f;
static float sLightBlue = 1.f;
static float sDistanceLinear;
static float sDistanceQuadratic;
static float sAngleLinear;
static float sAngleQuadratic;

CHomingBlob::CHomingBlob(const TToken< CGenDescription >& particle, TUniqueId uid, TAreaId areaId,
                         TUniqueId owner, bool active, const CAABox& bounds,
                         const CDamageInfo& damage, int playerIndex, const rstl::string& name,
                         const CTransform4f& xf, int modeFlags, float generatorRate,
                         float collisionRadius, float nearTargetDistance, float escapeDistance,
                         float targetSearchRadius, float homingAcceleration)
: CWeapon(uid, areaId, active, owner, kWT_Dark, name, xf,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59),
                                              CMaterialList(kMT_Character, kMT_Player)),
          CMaterialList(kMT_Projectile), damage, kPA_None, CModelData())
, mCollisionBounds(bounds)
, mParticleGen(rs_new CElementGen(particle, CElementGen::kMOT_One, CElementGen::kOSF_One))
, mCollisionCache(rs_new CCollisionCache(mCollisionBounds, 2, 2, uid.Value() & 0x3ff))
, mLightId(kInvalidUniqueId)
, mParticleAssetId(CToken(particle).GetTag().GetId())
, mTargetIds()
, mNextParticleTarget(0)
, mParticleUpdatePhase(0)
, mElapsedTime(0.f)
, x220_(6.f)
, mGeneratorRate(generatorRate)
, mCollisionRadius(collisionRadius)
, mNearTargetDistance(nearTargetDistance)
, mEscapeDistanceSquared(escapeDistance * escapeDistance)
, mTargetSearchRadius(targetSearchRadius)
, mHomingAcceleration(homingAcceleration)
, mPlayerIndex(playerIndex)
, mModeFlags(modeFlags)
, mFollowPlayerArea((modeFlags & kMF_FollowPlayerArea) != 0)
, mHasRenderBounds(false) {
  mParticleGen->SetOrientation(GetTransform().GetRotation());
  mParticleGen->SetTranslation(GetTranslation());
  mParticleGen->SetGeneratorRate(generatorRate);
}

CHomingBlob::~CHomingBlob() {}

void CHomingBlob::PreRenderAllViewports(CStateManager& mgr) {
  rstl::optional_object< CAABox > bounds = mParticleGen->GetBounds();
  if (bounds) {
    mHasRenderBounds = true;
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
  } else {
    mHasRenderBounds = false;
    const CVector3f pos = GetTranslation();
    const CAABox pointBounds(pos, pos);
    SetOtherBounds(pointBounds);
    SetRenderBounds(pointBounds);
  }
  UpdatePortalSystemState(mgr);
}

void CHomingBlob::PreRender(CStateManager& mgr) {
  SetPreRenderClipped(!mHasRenderBounds || !mgr.fn_800366e4(this));
  if (!GetPreRenderClipped() && mPlayerIndex == mgr.GetCurrentRenderPlayerIndex()) {
    SetPreRenderClipped(true);
  }
}

void CHomingBlob::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    const CAABox& bounds = GetRenderBoundsCached();
    EnsureRendered(mgr, bounds.GetCenterPoint(), bounds);
  }
}

void CHomingBlob::Render(const CStateManager& mgr) const {
  const CTransform4f& view = CGraphics::GetViewMatrix();
  const CLight light = CLight::BuildCustom(view.GetTranslation(), view.GetForward(),
                                           CColor(sLightRed, sLightGreen, sLightBlue, 1.f),
                                           sDistanceConstant, sDistanceLinear, sDistanceQuadratic,
                                           sAngleConstant, sAngleLinear, sAngleQuadratic);
  CGraphics::SetAmbientColor(CColor(sAmbientRed, sAmbientGreen, sAmbientBlue, 1.f));
  CGraphics::SetLightState(1);
  CGraphics::LoadLight(kLight0, light);
  mParticleGen->SetLeaveLightsEnabledForModelRender(true);
  mParticleGen->Render();
  CGraphics::SetLightState(0);
}

void CHomingBlob::Think(float dt, CStateManager& mgr) {
  mElapsedTime += dt;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, mCollisionBounds, GetMaterialFilter(), this);
  CGameCollision::UpdateCollisionCache(mgr, *mCollisionCache, nearList,
                                       CGameCollision::kCUP_KeepNearListIds);
  mParticleGen->Update(dt);
  UpdateParticles(mgr);

  if (mLightId != kInvalidUniqueId) {
    CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
    if (light && GetActive()) {
      light->SetLight(mParticleGen->GetLight());
    }
  }
  if (mFollowPlayerArea) {
    mgr.SetActorAreaId(*this, mgr.GetPlayer(0)->GetCurrentAreaId());
  }
  if (mParticleGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

bool CHomingBlob::FindNearestTriangle(float radius, const CVector3f& position, CVector3f& closest,
                                      const SCachedCollisionSlot*& slot, CVector3f& barycentric) {
  CSphere sphere(position, radius);
  float minDistSq = radius * radius;
  rstl::optional_object< CVector3f > nearest;
  CCollisionCache& cache = *mCollisionCache;
  CCollisionCacheIterator it(cache);
  while (!it.AtEnd()) {
    if (it.AtLeafStart()) {
      if (!CollisionUtil::AABoxSphereIntersection(*cache.GetLeafBounds(it), sphere)) {
        cache.SkipLeaf(it);
        continue;
      }
    }

    const SCachedCollisionSlot* triangle = cache.NextTriangle(it);
    const CCollisionSurface& tri = triangle->mTriangle.GetSurface();
    float baryU;
    float baryV;
    const float distSq = CollisionUtil::TriPointSqrDist_Float(
        position, tri.GetVert(0), tri.GetVert(1), tri.GetVert(2), &baryU, &baryV);
    if (distSq > minDistSq) {
      continue;
    }

    minDistSq = distSq;
    sphere = CSphere(position, CMath::SqrtF(distSq));
    barycentric = CVector3f(baryU, baryV, 1.f - (baryU + baryV));
    nearest = CMath::BaryToWorld(tri.GetVert(2), tri.GetVert(1), tri.GetVert(0), barycentric);
    slot = triangle;
  }

  if (nearest) {
    closest = *nearest;
    return true;
  }
  return false;
}

// Guessed TU-local identity; native calls pass two floats and no blob instance.
static CVector3f TangentVelocity(float normalDot, float speed, const CVector3f& velocity,
                                 const CVector3f& normal, float& resultSpeed) {
  const CVector3f tangent = velocity - normal * normalDot;
  const float magnitude = tangent.Magnitude();
  if (magnitude < 0.00011920929f) {
    resultSpeed = 0.f;
    return CVector3f::Zero();
  }
  resultSpeed = speed;
  return tangent * (speed / magnitude);
}

void CHomingBlob::UpdateParticles(CStateManager& mgr) {
  const SCachedCollisionSlot* slot = nullptr;
  CVector3f boundsPoint = CVector3f::Zero();
  CVector3f closest = CVector3f::Zero();
  CVector3f barycentric = CVector3f::Zero();

  rstl::reserved_vector< CActor*, 16 > targets;
  rstl::reserved_vector< CVector3f, 16 > targetPositions;
  rstl::reserved_vector< CAABox, 16 > targetBounds;
  rstl::reserved_vector< CVector3f, 16 > homingPositions;
  rstl::reserved_vector< float, 16 > accumulatedDamage;
  rstl::reserved_vector< float, 16 > targetDamage;

  for (int i = 0; i < mTargetIds.size(); ++i) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mTargetIds[i]));
    targets.push_back(actor);
    targetPositions.push_back(actor ? actor->GetTranslation() : CVector3f::Zero());
    const rstl::optional_object< CAABox > bounds =
        actor ? actor->GetTouchBounds() : rstl::optional_object< CAABox >();
    targetBounds.push_back(bounds ? *bounds : CAABox::MakeNullBox());
    homingPositions.push_back(actor ? actor->GetHomingPosition(mgr, 0.f) : CVector3f::Zero());
    accumulatedDamage.push_back(0.f);
    const CDamageVulnerability& vulnerability =
        actor ? *actor->GetDamageVulnerability() : CDamageVulnerability::NormalVulnerabilty();
    targetDamage.push_back(gpTweakPlayerGun->GetDarkBeamBlobDamage().GetDamage(vulnerability));
  }

  for (int i = 0; i < mParticleGen->GetParticleCount(); ++i) {
    if ((i & 3) != (mParticleUpdatePhase & 3)) {
      continue;
    }

    CElementGen::CParticle& particle = mParticleGen->mParticles[i];
    SParticleState* state = reinterpret_cast< SParticleState* >(
        const_cast< CElementGen::CAdvancedValues* >(mParticleGen->GetParticleAdditionalData(i)));
    CActor* target = nullptr;
    if (mTargetIds.size() != 0) {
      if (state->mUnassigned == -1.f) {
        state->mTargetSlot = mNextParticleTarget % mTargetIds.size();
        ++mNextParticleTarget;
        state->mFlags = 0;
      }
      target = targets[state->mTargetSlot];
    }

    bool escaped = state->HasFlag(kPF_Escaped);
    if (!escaped) {
      const CVector3f offset = mParticleGen->GetTranslation() - particle.mPos;
      if (CVector3f::Dot(offset, offset) > mEscapeDistanceSquared) {
        state->SetFlag(kPF_Escaped);
        escaped = true;
      }
    }
    const float searchRadius = escaped ? 0.05f : mCollisionRadius;

    if (!escaped && FindNearestTriangle(searchRadius, particle.mPos, closest, slot, barycentric)) {
      if (target != nullptr) {
        const float targetDist = CollisionUtil::AABoxPointDist(
            particle.mPos, targetBounds[state->mTargetSlot], &boundsPoint);
        const CVector3f toTarget = boundsPoint - particle.mPos;
        if (targetDist < mNearTargetDistance && !escaped &&
            CVector3f::Dot(toTarget, slot->mPlane.GetNormal()) > 0.f) {
          state->SetFlag(kPF_Escaped);
          --i;
          continue;
        }
        if (targetDist > 0.1f) {
          particle.mVel += toTarget * (mHomingAcceleration / targetDist);
        } else {
          const CVector3f toCenter = targetPositions[state->mTargetSlot] - particle.mPos;
          const float centerDist = toCenter.Magnitude();
          if (centerDist > 0.1f) {
            particle.mVel += toCenter * (mHomingAcceleration / centerDist);
          }
        }
      }

      const CVector3f previousPos = particle.mPos;
      particle.mPos = closest;
      const CVector3f& normal = slot->mPlane.GetNormal();
      const float normalSpeed = CVector3f::Dot(particle.mVel, normal);
      const float speed = particle.mVel.Magnitude();
      float resultSpeed = 0.f;
      const int edgeCount = (barycentric.GetX() < 0.00011920929f ? 1 : 0) +
                            (barycentric.GetY() < 0.00011920929f ? 1 : 0) +
                            (barycentric.GetZ() < 0.00011920929f ? 1 : 0);
      particle.mVel = TangentVelocity(normalSpeed, speed, particle.mVel, normal, resultSpeed);
      if (resultSpeed < 1.1920929e-7f || edgeCount > 1) {
        const float u = mgr.Random()->Float();
        const float v = (1.f - u) * mgr.Random()->Float();
        const CVector3f bary(u, v, 1.f - (u + v));
        const CVector3f point = CMath::BaryToWorld(slot->mTriangle.GetSurface().GetVert(0),
                                                   slot->mTriangle.GetSurface().GetVert(1),
                                                   slot->mTriangle.GetSurface().GetVert(2), bary);
        particle.mPos = previousPos;
        particle.mVel = (point - particle.mPos).AsNormalized() * speed;
      }
    } else if (target != nullptr) {
      const CVector3f toHoming = homingPositions[state->mTargetSlot] - particle.mPos;
      const float homingDist = toHoming.Magnitude();
      if (homingDist < 0.75f && !state->HasFlag(kPF_Damaged)) {
        state->SetFlag(kPF_Damaged);
        accumulatedDamage[state->mTargetSlot] += targetDamage[state->mTargetSlot];
      }
      if (homingDist > 0.1f) {
        particle.mVel += toHoming * (mHomingAcceleration / homingDist);
      }
    }
  }

  ++mParticleUpdatePhase;

  for (int i = 0; i < accumulatedDamage.size(); ++i) {
    const float amount = accumulatedDamage[i];
    const CDamageInfo& blobDamage = gpTweakPlayerGun->GetDarkBeamBlobDamage();
    const CDamageInfo damage(
        CWeaponMode(blobDamage.GetWeaponMode().GetType(), !mgr.IsMultiplayer()), amount, 0.f, 0.f,
        blobDamage.NoImmunity(), false);
    mgr.ApplyDamage(
        GetUniqueId(), mTargetIds[i], GetOwnerId(), damage,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
        CVector3f::Zero());
  }
}

void CHomingBlob::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();

  switch (message) {
  case kSM_Create:
    if (mParticleGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(
          mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l("HomingBlobLight"),
          GetTransform(), GetUniqueId(), mParticleGen->GetLight(), mParticleAssetId, 1, 0.f));
    }
    CGameCollision::BuildCollisionCache(mgr, *mCollisionCache, GetMaterialFilter());
    if (!(mModeFlags & kMF_SkipInitialTargets)) {
      const CVector3f center = mCollisionBounds.GetCenterPoint();
      const CAABox searchBounds(
          center - CVector3f(mTargetSearchRadius, mTargetSearchRadius, mTargetSearchRadius),
          center + CVector3f(mTargetSearchRadius, mTargetSearchRadius, mTargetSearchRadius));
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      const CActor* owner = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOwnerId()));
      mgr.BuildNearList(nearList, searchBounds,
                        CMaterialFilter::MakeIncludeExclude(
                            CMaterialList(kMT_Unknown54), CMaterialList(kMT_Character, kMT_Player)),
                        owner);

      const CUnitVector3f normal(GetTransform().GetForward());
      const CPlane plane(CVector3f::Dot(center, normal) + -0.1f, normal);

      bool swapped = true;
      while (swapped) {
        swapped = false;
        for (int i = 0; i < nearList.size() - 1; ++i) {
          const CActor* first = TCastToConstPtr< CActor >(mgr.GetObjectById(nearList[i]));
          const CActor* second = TCastToConstPtr< CActor >(mgr.GetObjectById(nearList[i + 1]));
          if (first == nullptr) {
            swapped = true;
            const TUniqueId id = nearList[i + 1];
            nearList[i + 1] = nearList[i];
            nearList[i] = id;
          } else if (second != nullptr) {
            if ((GetTranslation() - second->GetTranslation()).MagSquared() <
                (GetTranslation() - first->GetTranslation()).MagSquared()) {
              swapped = true;
              const TUniqueId id = nearList[i + 1];
              nearList[i + 1] = nearList[i];
              nearList[i] = id;
            }
          }
        }
      }

      const float searchRadiusSq = mTargetSearchRadius * mTargetSearchRadius;
      CVector3f closestPoint = CVector3f::Zero();
      for (int i = 0; i < nearList.size() && i < 16; ++i) {
        CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(nearList[i]));
        CSwarmBasics* swarm = TCastToPtr< CSwarmBasics >(actor);
        if (actor != nullptr && swarm == nullptr) {
          const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
          if (bounds) {
            const CAABox box = *bounds;
            if (CVector3f::Dot(plane.GetNormal(), box.FurthestPointAlongVector(
                                                      plane.GetNormal())) >= plane.GetConstant() &&
                CollisionUtil::AABoxPointSqrDist(center, box, &closestPoint) < searchRadiusSq) {
              mTargetIds.push_back(nearList[i]);
            }
          }
        } else if (swarm != nullptr) {
          swarm->FreezeBoids(GetTranslation(), mTargetSearchRadius);
        }
      }
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
    mgr.SendScriptMsg(mLightId, sender, message, kInvalidUniqueId);
  }
}

rstl::optional_object< CAABox > CHomingBlob::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CHomingBlob::Touch(CActor&, CStateManager&) {
  if (mElapsedTime > x220_) {
    return;
  }
}
