#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include <math.h>

static CElementGen* CreateElementGen(CAssetId id) {
  if (id != kInvalidAssetId) {
    TLockedToken< CGenDescription > desc = gpSimplePool->GetObj(SObjectTag('PART', id));
    return rs_new CElementGen(desc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  }
  return nullptr;
}

CBouncyGrenade::CBouncyGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CTransform4f& xf, const CModelData& model,
                               const CActorParameters& actParams, TUniqueId parentId,
                               float velocity, const CBouncyGrenadeData& data,
                               float explodePlayerDistance, const CAABox& bounds,
                               TUniqueId targetId, float maxHomingAngle, uint flags,
                               const CMaterialList* extraMaterials, const CHealthInfo* healthInfo)
: CPhysicsActor(
      uid, name, info, 0, xf, model,
      CMaterialList(kMT_Projectile).Union(extraMaterials ? *extraMaterials : CMaterialList()),
      !model.IsNull()
          ? model.GetBounds()
          : (bounds.Invalid() ? CAABox(CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f))
                              : bounds),
      SMoverData(data.GetVelocityInfo().GetMass()), actParams, skDefaultStepData)
, mData(data)
, mNumBounces(data.GetNumBounces())
, mParentId(parentId)
, mTargetId(targetId)
, mMaxHomingAngle(maxHomingAngle)
, mLastPosition(xf.GetTranslation())
, mStationaryTime(0.f)
, mElapsedTime(0.f)
, mElementGenExplodeCombat(CreateElementGen(data.GetElementGenId1()))
, mElementGenExplodeXRay(CreateElementGen(data.GetElementGenId2()))
, mElementGenTrailCombat(CreateElementGen(data.GetElementGenId3()))
, mElementGenTrailXRay(CreateElementGen(data.GetElementGenId4()))
, mExplodePlayerDistance(explodePlayerDistance)
, mFlags(flags)
, mExploded(false)
, mHasRenderBounds(false) {
  if (healthInfo) {
    mHealthInfo = *healthInfo;
  }
  if (extraMaterials) {
    mMaterialsToRemove = *extraMaterials;
  }
  const float mass = GetMass();
  SetMomentumWR(CVector3f(0.f, 0.f, -kDefaultGravityAccel * mass));
  SetVelocityWR(velocity * xf.GetForward());
  mElementGenExplodeCombat->SetParticleEmission(false);
  mElementGenExplodeXRay->SetParticleEmission(false);
  if (!mElementGenTrailCombat.null()) {
    mElementGenTrailCombat->SetParticleEmission(true);
  }
  if (!mElementGenTrailXRay.null()) {
    mElementGenTrailXRay->SetParticleEmission(true);
  }
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Character));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
}

void CBouncyGrenade::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                  CStateManager& mgr) {
  static const CMaterialList skSolidTypes(kMT_Unknown59, kMT_Ceiling, kMT_Wall, kMT_Floor,
                                          kMT_Character);
  bool shouldExplode = false;
  if (id != mParentId) {
    if (const CEntity* entity = mgr.GetObjectById(id)) {
      if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(entity)) {
        shouldExplode = actor->GetOwnerId() != mParentId;
      } else {
        shouldExplode = true;
      }
    }
  }
  if (shouldExplode) {
    if (const CBouncyGrenade* other = TCastToConstPtr< CBouncyGrenade >(mgr.GetObjectById(id))) {
      Bounce(mgr, (GetTranslation() - other->GetTranslation()).AsNormalized(), false, false);
    } else {
      Explode(mgr, id);
    }
  } else if (IsArmed()) {
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().SharesMaterials(skSolidTypes)) {
        if ((mFlags & 1) != 0 && !info.GetMaterialLeft().HasMaterial(kMT_SeekerTarget)) {
          Explode(mgr);
        } else if (mNumBounces != 0) {
          const CVector3f normal = CVector3f::Dot(GetVelocityWR(), info.GetNormalLeft()) > 0.f
                                       ? info.GetNormalRight()
                                       : info.GetNormalLeft();
          Bounce(mgr, normal, true, true);
        } else {
          Explode(mgr);
        }
        break;
      }
    }
  }
  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CBouncyGrenade::Touch(CActor& act, CStateManager& mgr) { CActor::Touch(act, mgr); }

void CBouncyGrenade::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage kind = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (kind) {
  case kSM_Damage:
    if (const CHealthInfo* health = GetHealthInfo()) {
      if (health->GetHP() <= 0.f) {
        Explode(mgr);
      }
    }
    break;
  case kSM_XENZ:
    if (!mExploded && (mFlags & 4) != 0) {
      Explode(mgr);
    }
    break;
  default:
    break;
  }
}

void CBouncyGrenade::Think(float dt, CStateManager& mgr) {
  UpdateGrenadeFX(dt, mgr);
  if (mElementGenExplodeCombat->IsSystemDeletable() &&
      mElementGenExplodeXRay->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
  if (mElapsedTime > 0.4f && !GetMaterialList().HasMaterial(kMT_Unknown59)) {
    AddMaterial(kMT_Unknown59, mgr);
  }
}

rstl::optional_object< CAABox > CBouncyGrenade::GetTouchBounds() const {
  return GetCollisionPrimitive()->CalculateAABox(GetTransform());
}

void CBouncyGrenade::Render(const CStateManager& mgr) const {
  if (!mExploded) {
    if (HasModelData()) {
      GetModelData()->Render(mgr, GetTransform(), nullptr,
                             CModelFlags(CModelFlags::kT_Opaque, 1.f));
    }
    bool darkVisor = false;
    if ((mFlags & 2) != 0 && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
      darkVisor = true;
    }
    if (darkVisor) {
      CElementGen::sEnableAlphaModulation = darkVisor;
      gpRender->SetDestinationAlpha(255);
    }
    if (!mElementGenTrailCombat.null()) {
      mElementGenTrailCombat->Render();
    }
    if (!mElementGenTrailXRay.null()) {
      mElementGenTrailXRay->Render();
    }
    if (darkVisor) {
      gpRender->DisableDestinationAlpha();
      CElementGen::sEnableAlphaModulation = false;
    }
  }
}

void CBouncyGrenade::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
  if (mExploded && mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Echo) {
    gpRender->AddParticleGen(*mElementGenExplodeCombat);
  }
  if (mHasRenderBounds && mgr.IsActorVisible(*this)) {
    EnsureRendered(mgr);
  }
}

void CBouncyGrenade::PreRenderAllViewports(CStateManager& mgr) {
  rstl::optional_object< CAABox > bounds;
  if (!mElementGenTrailCombat.null()) {
    bounds = mElementGenTrailCombat->GetBounds();
  }
  if (!mElementGenTrailXRay.null()) {
    rstl::optional_object< CAABox > xrayBounds = mElementGenTrailXRay->GetBounds();
    if (xrayBounds.valid()) {
      if (bounds.valid()) {
        const CAABox& xrayBox = *xrayBounds;
        CAABox& box = *bounds;
        box.AccumulateBounds(xrayBox.GetMinPoint());
        box.AccumulateBounds(xrayBox.GetMaxPoint());
      } else {
        bounds = xrayBounds;
      }
    }
  }
  if (bounds.valid()) {
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
    mHasRenderBounds = true;
  } else {
    const CVector3f pos = GetTranslation();
    const CAABox pointBounds(pos, pos);
    SetOtherBounds(pointBounds);
    SetRenderBounds(pointBounds);
    mHasRenderBounds = false;
  }
  UpdatePortalSystemState(mgr);
}

void CBouncyGrenade::UpdateGrenadeFX(float dt, CStateManager& mgr) {
  if (GetActive()) {
    const CTransform4f orientation = GetTransform().GetRotation();
    const CVector3f translation = GetTranslation();
    const CVector3f scale = HasModelData() ? GetModelData()->GetScale() : CVector3f(1.f, 1.f, 1.f);
    if (mExploded) {
      Stop();
      mElementGenExplodeCombat->SetOrientation(orientation);
      mElementGenExplodeCombat->SetGlobalTranslation(translation);
      mElementGenExplodeCombat->SetGlobalScale(scale);
      mElementGenExplodeCombat->Update(dt);
      mElementGenExplodeXRay->SetOrientation(orientation);
      mElementGenExplodeXRay->SetGlobalTranslation(translation);
      mElementGenExplodeXRay->SetGlobalScale(scale);
      mElementGenExplodeXRay->Update(dt);
    } else {
      if (!mElementGenTrailCombat.null()) {
        mElementGenTrailCombat->SetOrientation(orientation);
        mElementGenTrailCombat->SetTranslation(translation);
        mElementGenTrailCombat->SetGlobalScale(scale);
        mElementGenTrailCombat->Update(dt);
      }
      if (!mElementGenTrailXRay.null()) {
        mElementGenTrailXRay->SetOrientation(orientation);
        mElementGenTrailXRay->SetGlobalTranslation(translation);
        mElementGenTrailXRay->SetGlobalScale(scale);
        mElementGenTrailXRay->Update(dt);
      }
    }
    UpdateExplodeChecks(dt, mgr);
  }
}

void CBouncyGrenade::Explode(CStateManager& mgr, TUniqueId uid) {
  if (mExploded) {
    return;
  }
  mExploded = true;
  CAudioSys::C3DEmitterParmData parms(mData.GetExplodeSfxMaxDist(), mData.GetExplodeSfxFalloff(), 1,
                                      0x7f, 0x14);
  parms.mPos = GetTranslation();
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = mData.GetExplodeSfx();
  CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), true, false,
                          CSfxManager::kMedPriority);
  mElementGenExplodeCombat->SetParticleEmission(true);
  mElementGenExplodeXRay->SetParticleEmission(true);
  if (!mElementGenTrailCombat.null()) {
    mElementGenTrailCombat->SetParticleEmission(false);
  }
  if (!mElementGenTrailXRay.null()) {
    mElementGenTrailXRay->SetParticleEmission(false);
  }
  bool isParent = uid == mParentId;
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    isParent = actor->GetOwnerId() == mParentId;
  }
  const CDamageInfo& dInfo = mData.GetDamageInfo();
  if (uid != kInvalidUniqueId && !isParent) {
    mgr.ApplyDamage(
        GetUniqueId(), uid, GetUniqueId(), dInfo,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
        CVector3f::Zero());
  }
  if (dInfo.GetRadius() > 1.f) {
    const CVector3f pos = GetTranslation();
    const CVector3f extent(dInfo.GetRadius(), dInfo.GetRadius(), dInfo.GetRadius());
    const CAABox bounds(pos - extent, pos + extent);
    const CMaterialFilter filter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Character, kMT_Player));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      bool isNearParent = *it == mParentId;
      if (const CCollisionActor* actor =
              TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(*it))) {
        isNearParent = actor->GetOwnerId() == mParentId;
      }
      if (isNearParent || *it == uid) {
        continue;
      }
      if (const CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it))) {
        const float magnitude = (actor->GetTranslation() - GetTranslation()).Magnitude();
        if (magnitude < dInfo.GetRadius()) {
          if (mData.GetX4c()) {
            const float scale = (dInfo.GetRadius() - magnitude) / dInfo.GetRadius();
            const CDamageInfo info(dInfo.GetWeaponMode(), scale * dInfo.GetDamage(),
                                   dInfo.GetRadius(), scale * dInfo.GetKnockBackPower());
            mgr.ApplyDamage(
                GetUniqueId(), *it, GetUniqueId(), info,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
                CVector3f::Zero());
          } else {
            mgr.ApplyDamage(
                GetUniqueId(), *it, GetUniqueId(), dInfo,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
                CVector3f::Zero());
          }
        }
      }
    }
  }
  if (mMaterialsToRemove.valid()) {
    CMaterialList materials = GetMaterialList();
    materials.Remove(*mMaterialsToRemove);
    SetMaterialList(materials, mgr);
  }
}

bool CBouncyGrenade::IsArmed() const { return mElapsedTime > 0.2f; }

void CBouncyGrenade::UpdateExplodeChecks(float dt, CStateManager& mgr) {
  if (mExploded) {
    return;
  }
  mElapsedTime += dt;
  if (mElapsedTime >= 15.f) {
    Explode(mgr);
    return;
  }
  if (IsArmed()) {
    const CVector3f translation = GetTranslation();
    const CVector3f moved = translation - mLastPosition;
    if (!moved.IsNonZero()) {
      mStationaryTime += dt;
      if (mStationaryTime >= 0.5f) {
        Explode(mgr);
        return;
      }
    }
    mLastPosition += moved;
    for (int i = 0; i < uint(mgr.GetNumPlayers()); ++i) {
      const CPlayer* player = mgr.GetPlayer(i);
      const CVector3f playerPos =
          player->GetTranslation() + CVector3f(0.f, 0.f, 0.5f * player->GetEyeHeight());
      const CVector3f& delta = CVector3f(playerPos - translation);
      if (delta.MagSquared() < mExplodePlayerDistance * mExplodePlayerDistance) {
        Explode(mgr);
        return;
      }
    }
  }
}

void CBouncyGrenade::Bounce(CStateManager& mgr, const CVector3f& normal, bool consumeBounce,
                            bool home) {
  const float speed = mData.GetVelocityInfo().GetSpeed();
  const CVector3f impulse = (speed * GetConstantForceWR().Magnitude()) * normal;
  const CAxisAngle angularImpulse = -speed * GetAngularMomentumWR();
  ApplyImpulseWR(impulse, angularImpulse);
  if (home && mTargetId != kInvalidUniqueId) {
    if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
      CVector3f velocity = GetVelocityWR();
      velocity.SetZ(0.f);
      CVector3f toTarget = target->GetAimPosition(mgr, 0.f) - GetTranslation();
      toTarget.SetZ(0.f);
      CQuaternion rotation = CQuaternion::ShortestRotationArc(velocity, toTarget);
      if (2.f * rotation.GetScalar() * rotation.GetScalar() - 1.f <= 0.99f) {
        float maxAngle = 0.017453292f * mMaxHomingAngle;
        const float angle = acos(2.f * rotation.GetScalar() * rotation.GetScalar() - 1.f);
        if (maxAngle < angle) {
          const float sinHalfAngle = sin(0.5f * angle);
          maxAngle = 0.5f * maxAngle;
          const float scale = sin(maxAngle) / sinHalfAngle;
          rotation = CQuaternion(cos(maxAngle), scale * rotation.GetVector());
        }
        CTransform4f newXf = GetTransform().MultiplyIgnoreTranslation(rotation.BuildTransform4f());
        newXf.Orthonormalize();
        SetTransform(newXf);
        SetVelocityWR(rotation.BuildTransform4f() * GetVelocityWR());
      }
    }
  }
  CAudioSys::C3DEmitterParmData parms(mData.GetExplodeSfxMaxDist(), mData.GetExplodeSfxFalloff(), 1,
                                      0x7f, 0x14);
  parms.mPos = GetTranslation();
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = mData.GetBounceSfx();
  CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), true, false,
                          CSfxManager::kMedPriority);
  if (consumeBounce) {
    --mNumBounces;
  }
}

CBouncyGrenade::~CBouncyGrenade() {}
