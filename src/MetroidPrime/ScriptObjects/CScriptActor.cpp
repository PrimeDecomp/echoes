#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include <float.h>

CScriptActor::CScriptActor(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& model, const CAABox& bounds,
                           const CMaterialList& materials, float mass, float zMomentum,
                           const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                           const CActorParameters& parameters,
                           const SEchoParameters& echoParameters, bool looping, int shaderIdx,
                           bool castsShadow, bool scaleAdvancementDelta, bool unused,
                           float animationTimeVariation, CAssetId projectile,
                           const CDamageInfo& projectileDamage, CAssetId collisionTree)
: CPhysicsActor(uid, name, info, 0, xf, model, materials, bounds, SMoverData(mass), parameters,
                StepData(0.3f, 0.3f, 0))
, mInitialHealth(health)
, mCurrentHealth(health)
, mDamageVulnerability(vulnerability)
, mFadeInTime(parameters.GetFadeInTime())
, mFadeOutTime(parameters.GetFadeOutTime())
, mAnimationTimeVariation(animationTimeVariation)
, mShaderIdx(shaderIdx)
, mTriggerId(kInvalidUniqueId)
, mDead(false)
, mAnimating(true)
, mProcessModelFlags(shaderIdx != 0)
, mScaleAdvancementDelta(scaleAdvancementDelta)
, mIsPlayerActor(false)
, mSkipRendering(false)
, mRenderImmediately(false) {
  if (projectile != kInvalidAssetId) {
    mProjectileInfo = CProjectileInfo(projectile, projectileDamage);
    mProjectileInfo->Token().Lock();
  }
  if (HasModelData()) {
    if (castsShadow) {
      SetDrawShadow(true);
    }
    if (HasAnimation()) {
      ModelData()->EnableLooping(looping);
    }
  }
  if (collisionTree != kInvalidAssetId) {
    mTreeGroupContainer =
        TLockedToken< COBBTreeGroup >(gpSimplePool->GetObj(SObjectTag('DCLN', collisionTree)));
    mCollisionPrimitive = rs_new CCollidableOBBTreeGroup(
        static_cast< const COBBTreeGroup* >(**mTreeGroupContainer), GetMaterialList());
  }
  SetMomentumWR(CVector3f(0.f, 0.f, -zMomentum));
  AllocateEchoEmitter(true, GetBoundingBox(), echoParameters);
}

CScriptActor::~CScriptActor() {}

CHealthInfo* CScriptActor::HealthInfo() { return &mCurrentHealth; }

const CDamageVulnerability* CScriptActor::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CScriptActor::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CScriptActor::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Unknown59)) {
    CAABox bounds = GetBoundingBox();
    if (!mCollisionPrimitive.null()) {
      bounds.Include(mCollisionPrimitive->CalculateAABox(GetTransform()));
    }
    return bounds;
  }
  return rstl::optional_object_null();
}

void CScriptActor::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (EchoEmitter() != nullptr) {
    EchoEmitter()->SetBounds(GetBoundingBox());
  }
  if (HasAnimation()) {
    const bool timeRemaining =
        GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, rstl::string_l("Whole Body"));
    const bool loop = GetModelData()->GetIsLoop();
    const float variation = mAnimationTimeVariation * mgr.Random()->Range(-1.f, 1.f);
    mAnimationTimeVariation = 0.f;
    const CAdvancementDeltas deltas = CActor::UpdateAnimation(dt + variation, mgr, true);

    if (timeRemaining || loop) {
      mAnimating = true;
      CVector3f translation = deltas.GetOffsetDelta();
      if (mScaleAdvancementDelta) {
        translation = GetTransform().Rotate(CVector3f::ByElementMultiply(
            GetModelData()->GetScale(), GetTransform().TransposeRotate(translation)));
      }
      MoveToOR(translation, dt);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    }
    if (!timeRemaining && mAnimating && !loop) {
      SendScriptMsgs(kSS_MaxReached, mgr);
      mAnimating = false;
      Stop();
    }
  }

  if (!mDead && HealthInfo()->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
  }
  CActor::Think(dt, mgr);
}

void CScriptActor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->state == kSS_InheritBounds && conn->msg == kSM_Activate) {
        const CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->objId);
        for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
          if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(id->second))) {
            mTriggerId = id->second;
          }
        }
      }
      if (conn->state == kSS_ScanSource && conn->msg == kSM_Attach) {
        AddMaterial(kMT_Scannable, mgr);
      }
    }
    break;
  case kSM_Reset:
    mDead = false;
    mCurrentHealth = mInitialHealth;
    break;
  case kSM_Increment:
    if (!GetActive()) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Activate);
      CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), mFadeInTime);
    }
    break;
  case kSM_Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), mFadeOutTime);
    break;
  case kSM_Kill:
    if (!mDead) {
      HealthInfo()->SetHP(0.f);
      mDead = true;
      SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
    }
    break;
  case kSM_InternalMessage00:
    if (EchoEmitter() != nullptr) {
      EchoEmitter()->TriggerDamageEcho();
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptActor::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (GetPreRenderClipped() &&
      TCastToConstPtr< CCinematicCamera >(
          *mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true))) {
    SetPreRenderClipped(false);
  }

  if (!GetPreRenderClipped() && mProcessModelFlags && mShaderIdx != 0) {
    SetModelFlags(GetModelFlags().UseShaderSet(mShaderIdx));
  }
  if (mgr.GetObjectById(mTriggerId) == nullptr) {
    mTriggerId = kInvalidUniqueId;
  }
  if (!mPortalPlane.null()) {
    SetModelFlags(
        CModelFlags(GetModelFlags(), GetModelFlags().GetOtherFlags() | CModelFlags::kF_Unknown80));
  }
}

EWeaponCollisionResponseTypes CScriptActor::GetCollisionResponseType(const CVector3f& point,
                                                                     const CVector3f& normal,
                                                                     const CWeaponMode& mode,
                                                                     int attribs) const {
  const CWeaponTypeVulnerability vulnerability = GetDamageVulnerability()->GetVulnerability(mode);
  const bool hurts = !close_enough(vulnerability.mDamageMultiplier, 0.f) &&
                     vulnerability.mEffect != CWeaponTypeVulnerability::kE_Immune;
  if (!hurts && vulnerability.mEffect == CWeaponTypeVulnerability::kE_Reflect) {
    return kWCR_Unknown15;
  }
  return CActor::GetCollisionResponseType(point, normal, mode, attribs);
}

CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const {
  if (mTriggerId != kInvalidUniqueId) {
    const CScriptTrigger* trigger =
        static_cast< const CScriptTrigger* >(mgr.GetObjectById(mTriggerId));
    if (trigger != nullptr) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return CActor::GetSortingBounds(mgr);
}

void CScriptActor::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  if (type == kUE_Projectile) {
    if (mProjectileInfo.valid()) {
      FireProjectile(mgr, node.GetLocatorName());
    }
    return;
  }
  CActor::DoUserAnimEvent(mgr, node, type, dt);
}

void CScriptActor::FireProjectile(CStateManager& mgr, const rstl::string& locator) {
  const CTransform4f xf = GetTransform() * GetScaledLocatorTransform(locator);
  if (!mProjectileInfo->Token().TryCache()) {
    return;
  }

  if (!mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 1)) {
    return;
  }

  CEnergyProjectile* projectile = rs_new CEnergyProjectile(
      true, mProjectileInfo->Token(), kWT_AI, xf, kMT_Character, mProjectileInfo->GetDamage(),
      mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), kInvalidUniqueId, kPA_None, false,
      CVector3f(1.f, 1.f, 1.f), CImpactVisorEffect(), false, true, false, 1.f, 4.f, 4.f);
  if (projectile != nullptr) {
    mgr.AddObject(projectile);
  }
}

void CScriptActor::SetPortalPlane(const CPlane& plane) {
  if (mPortalPlane.null()) {
    mPortalPlane = rs_new CPlane(plane);
  } else {
    *mPortalPlane = plane;
  }
}

void CScriptActor::AddToRenderer(const CStateManager& mgr) const {
  if (mSkipRendering) {
    return;
  }

  if (mRenderImmediately) {
    if (!GetPreRenderClipped()) {
      Render(mgr);
    }
  } else {
    CActor::AddToRenderer(mgr);
  }
}

void CScriptActor::Render(const CStateManager& mgr) const {
  if (!mPortalPlane.null()) {
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), *mPortalPlane);
  }
  CPhysicsActor::Render(mgr);
}

const CCollisionPrimitive* CScriptActor::GetCollisionPrimitive() const {
  return mCollisionPrimitive.null() ? CPhysicsActor::GetCollisionPrimitive()
                                    : mCollisionPrimitive.get();
}

CTransform4f CScriptActor::GetPrimitiveTransform() const {
  if (!mCollisionPrimitive.null()) {
    CTransform4f xf = GetTransform();
    xf.AddTranslation(GetPrimitiveOffset());
    return xf;
  }
  return CTransform4f::Translate(GetTranslation() + GetPrimitiveOffset());
}

CEntity* LoadActor(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrActor sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrActor.inc"

  CMaterialList materials;
  if (sldrThis.immovable) {
    materials.Add(kMT_Immovable);
  }
  if (sldrThis.isSolid) {
    materials.Add(kMT_Unknown59);
  }
  if (sldrThis.isCameraThrough) {
    materials.Add(kMT_CameraPassthrough);
  }
  if (sldrThis.isScanThrough) {
    materials.Add(kMT_ScanPassthrough);
  }

  const rstl::optional_object< CModelData > model =
      LdrToModelData(sldrThis.editorProperties.transform.scale, sldrThis.model,
                     sldrThis.animationInformation, sldrThis.isLoop);
  if (!model.valid()) {
    return nullptr;
  }

  const CAABox bounds =
      sldrThis.collisionBox == CVector3f::Zero()
          ? model->GetBounds(LdrToTransform4f(sldrThis.editorProperties).GetRotation())
          : LoadCAABox(mgr, info.GetAreaId(), sldrThis.editorProperties.transform.scale,
                       LdrToTransform4f(sldrThis.editorProperties), sldrThis.collisionBox,
                       sldrThis.collisionOffset);

  return rs_new CScriptActor(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *model, bounds, materials, sldrThis.mass, sldrThis.gravity, LdrToHealthInfo(sldrThis.health),
      LdrToDamageVulnerability(sldrThis.vulnerability),
      LdrToActorParameters(sldrThis.actorInformation),
      LdrToEchoParameters(sldrThis.echoInformation), sldrThis.isLoop, sldrThis.renderTextureSet,
      sldrThis.drawsShadow, sldrThis.scaleAnimation, sldrThis.aiShootThrough,
      sldrThis.randomAnimationOffset, sldrThis.projectile,
      LdrToDamageInfo(sldrThis.projectileDamage), sldrThis.collisionModel);
}

bool CScriptActor::CheckActorRenderOnly() const {
  const bool unknown59 = GetMaterialList().HasMaterial(kMT_Unknown59);
  const bool passthrough = GetMaterialList().HasMaterial(kMT_CameraPassthrough);
  if (!GetMaterialList().HasMaterial(kMT_Immovable) || !passthrough || unknown59) {
    return false;
  }
  return true;
}
