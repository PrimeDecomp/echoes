#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZone.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "math.h"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

CEntity* LoadSafeZone(CStateManager& mgr, CInputStream& input, CEntityInfo& info);
CEntity* LoadSafeZoneCrystal(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

CScriptSafeZone::CScriptSafeZone(
    TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CVector3f& scale,
    const CTransform4f& xf, const CDamageInfo& damage, const CVector3f& forceField,
    float activationTime, float deactivationTime, float lifetime, float randomLifetimeOffset,
    float insideFadeStart, float insideFadeTime, float insideFadeMinAlpha, float flashTime,
    uint flags, bool deactivateOnEnter, bool deactivateOnExit, CAssetId impactEffect,
    const CDarkWorldInfo& normalInfo, const CDarkWorldInfo& hurtfulInfo,
    const CDarkWorldInfo& echoInfo, const CDamageInfo& normalDamage,
    const CDamageInfo& hurtfulDamage, bool filterSoundEffects, int lowPassFrequency,
    bool ignoreCinematicCamera, bool mobile, bool generateMobileLight,
    const CVector3f& mobileLightOffset, EShapeType shape, const CSafeZoneFog& insideFog,
    const CSafeZoneFog& outsideFog, const SEchoParameters& echoParameters, float flashBrightness,
    ushort flashSound, CColor insideFilterColor, float insideFilterTime)
: CScriptTriggerEllipsoid(uid, name, info, scale, xf, damage, forceField, flags | 0x1028f806,
                          deactivateOnEnter, deactivateOnExit, shape)
, mActivationTime(activationTime)
, mDeactivationTime(deactivationTime)
, mActivation(info.GetActive() ? 0.9999999f : 0.f)
, mLifetime(lifetime)
, mRandomLifetimeOffset(randomLifetimeOffset)
, mLifeTimer(0.f)
, mShellPulse(0.f)
, mLoopSoundDelay(0.f)
, mInsideTime(0.f)
, mInsideAlpha(1.f)
, mInsideFadeStart(insideFadeStart)
, mInsideFadeTime(insideFadeTime)
, mInsideFadeMinAlpha(insideFadeMinAlpha)
, mFlashTimer(0.f)
, mFlashTime(flashTime)
, mFlashBrightness(flashBrightness)
, mFlashSound(flashSound)
, mCameraInside(false)
, mPrevCameraInside(false)
, mIgnoreCinematicCamera(ignoreCinematicCamera)
, mFogDirty(false)
, mMobile(mobile)
, mGenerateMobileLight(generateMobileLight)
, mFilterSoundEffects(filterSoundEffects)
, mLowPassFrequency(lowPassFrequency)
, mLowPassFilterId(0)
, mMobileLightOffset(mobileLightOffset)
, mLightId(kInvalidUniqueId)
, x262_(kInvalidUniqueId)
, mNormalInfo(normalInfo)
, mHurtfulInfo(hurtfulInfo)
, mEchoInfo(echoInfo)
, mNormalDamage(normalDamage)
, mHurtfulDamage(hurtfulDamage)
, mCurrentInfo(&mNormalInfo)
, mZoneType(kZT_Normal)
, mObstructionType(-1)
, mObstructionPos(CVector3f::Zero())
, mObstructionRadius(0.f)
, mInsideFog(insideFog)
, mOutsideFog(outsideFog)
, mInsideFilterColor(insideFilterColor)
, mInsideFilterTime(insideFilterTime)
, mImpactEffect(gpSimplePool->GetObj(SObjectTag('PART', impactEffect))) {
  mNormalDamage.SetNoImmunity(true);
  mHurtfulDamage.SetNoImmunity(true);
  AllocateEchoEmitter(false, GetTriggerBoundsWR(), echoParameters);
}

// The REL carries its own weak copy of the base destructor and inlines it here.
inline CScriptTriggerEllipsoid::~CScriptTriggerEllipsoid() {}

CScriptSafeZone::~CScriptSafeZone() {}

void CScriptSafeZone::SetActive(const bool active) { CActor::SetActive(active); }

CLight CScriptSafeZone::BuildLight() const {
  const float radius = mActivation * GetScale().GetX();
  if (mGenerateMobileLight) {
    const CColor color = CColor::Lerp(CColor::Black(), CColor::White(), 1.f);
    CLight light(CLight::BuildPoint(CVector3f::Zero(), color));
    light.SetAttenuation(0.0001f, 1.f / (5000.f * radius), 1.f / (0.5f * radius));
    return light;
  }
  CLight light(CLight::BuildHard(CVector3f::Zero(), CColor::White(), radius));
  return light;
}

void CScriptSafeZone::SetZoneType(EZoneType type) {
  if (mZoneType == type) {
    return;
  }
  mZoneType = type;
  switch (mZoneType) {
  case kZT_Normal:
    mCurrentInfo = &mNormalInfo;
    break;
  case kZT_Hurtful:
    mCurrentInfo = &mHurtfulInfo;
    break;
  case kZT_Echo:
    mCurrentInfo = &mEchoInfo;
    break;
  }
}

void CScriptSafeZone::PlaySound(CStateManager& mgr, ushort sfx, uint flags) {
  const int areaId = mgr.mNextAreaId.Value();
  ProcessSoundEvent(flags | sfx, 1.f, 0, 1.f, 75.f, CSegId(0), 0x2000, 0x2000, 0.f, 0x14, 0x7f,
                    GetDistanceToCamera(mgr), GetTranslation(), areaId, mgr, true);
}

void CScriptSafeZone::PlayDeactivateSound(CStateManager& mgr) {
  StopLoopedSounds();
  PlaySound(mgr, mCurrentInfo->x4_, 0);
}

void CScriptSafeZone::PlayActivateSound(CStateManager& mgr) {
  mLoopSoundDelay = 0.f;
  StopLoopedSounds();
  PlaySound(mgr, mCurrentInfo->x0_, 0);
}

void CScriptSafeZone::SetLowPassFilter(bool enabled) {
  if (mFilterSoundEffects) {
    if (enabled) {
      if (mLowPassFilterId == 0) {
        mLowPassFilterId = CSfxManager::AddLowPassAreaFilter(mLowPassFrequency, 0.f);
      }
    } else {
      CSfxManager::RemoveLowPassAreaFilter(mLowPassFilterId);
      mLowPassFilterId = 0;
    }
  }
}

void CScriptSafeZone::Think(float dt, CStateManager& mgr) {
  bool moved = false;
  if (mMobile && GetTransformDirtySpare()) {
    moved = true;
  }
  CScriptTriggerEllipsoid::Think(dt, mgr);

  rstl::list< TUniqueId >::iterator projIt = mProjectiles.begin();
  while (projIt != mProjectiles.end()) {
    if (mgr.GetObjectById(*projIt) == nullptr) {
      projIt = mProjectiles.erase(projIt);
    } else {
      ++projIt;
    }
  }

  rstl::list< rstl::auto_ptr< CElementGen > >::iterator genIt = mImpactGens.begin();
  while (genIt != mImpactGens.end()) {
    CElementGen* gen = genIt->get();
    gen->Update(dt);
    if (gen->IsSystemDeletable()) {
      genIt = mImpactGens.erase(genIt);
    } else {
      ++genIt;
    }
  }

  bool changed = false;
  if (GetActive()) {
    if (mgr.GetIsDarkWorld()) {
      if (mActivation < 1.f) {
        mActivation = rstl::min_val(1.f, (1.f / mActivationTime) * dt + mActivation);
        changed = true;
      } else if (mShellPulse > 0.f) {
        mShellPulse = rstl::max_val(mShellPulse - (1.f / mActivationTime) * dt, 0.f);
      }
    } else if (mActivation != 0.f) {
      mActivation = 0.f;
      changed = true;
    }
    if (mLifetime > 0.f) {
      mLifeTimer += dt;
    }
    UpdateLoopSound(dt, mgr);
    UpdateInsideAlpha(dt, mgr);
    UpdateFlash(dt, mgr);
    if (moved) {
      SetTransformDirtySpare(false);
      if (mObstructionType != -1) {
        UpdateObstruction(mgr, false);
        UpdateObstruction(mgr, true);
      }
      if (mGenerateMobileLight) {
        if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
          light->SetTransform(CTransform4f::Translate(mMobileLightOffset) * GetTransform());
        }
      }
    }
    if (mZoneType == kZT_Echo) {
      mgr.InformListeners(GetTranslation(), static_cast< EListenNoiseType >(6));
    }
  } else if (mActivation > 0.f) {
    mActivation = mActivation - (1.f / mDeactivationTime) * dt;
    if (mActivation <= 0.f) {
      mActivation = 0.f;
      SetZoneType(kZT_Normal);
      SetLowPassFilter(false);
    }
    changed = true;
  }

  if (changed) {
    UpdateSafeZoneManager(mgr);
  }

  if (CEchoEmitter* emitter = EchoEmitter()) {
    if (mActivation == 0.f) {
      emitter->SetActive(false);
    } else {
      UpdateEchoEmitter(dt, mgr);
    }
  }

  if (mLifetime > 0.f && mLifeTimer > mLifetime) {
    if (GetActive()) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Deactivate, kInvalidUniqueId);
    }
    if (mActivation == 0.f) {
      mgr.mSafeZoneManager->RemoveSafeZone(GetUniqueId());
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }

  if (mPrevCameraInside != mCameraInside) {
    if (!mIgnoreCinematicCamera || !mgr.CameraManager(0)->IsInCinematicCamera()) {
      SendScriptMsgs(mCameraInside ? kSS_MaxReached : kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
      ApplyFog(mgr, mCameraInside ? mInsideFog : mOutsideFog);
      ApplyInsideFilter(mgr);
      SetLowPassFilter(mCameraInside);
      mFogDirty = false;
    }
    mPrevCameraInside = mCameraInside;
  }

  if (mFogDirty) {
    ApplyFog(mgr, mCameraInside ? mInsideFog : mOutsideFog);
    SetLowPassFilter(mCameraInside);
    mFogDirty = false;
  }
}

void CScriptSafeZone::UpdateEchoEmitter(float dt, CStateManager& mgr) {
  CEchoEmitter* emitter = EchoEmitter();
  const float half = 0.75f * (mActivation * (0.70710677f * GetScale().GetX()));
  const CVector3f center = GetTouchBounds()->GetCenterPoint();
  emitter->SetActive(true);
  emitter->SetBounds(
      CAABox(center - CVector3f(half, half, half), center + CVector3f(half, half, half)));
  emitter->Think(dt, mgr);
}

void CScriptSafeZone::UpdateSafeZoneManager(CStateManager& mgr) {
  CSafeZoneManager* manager = mgr.mSafeZoneManager;
  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  bool lightActive = false;
  if (light && light->GetActive()) {
    lightActive = true;
  }
  if (mActivation > 0.f) {
    const CVector3f radii = mActivation * GetScale();
    manager->AddOrUpdateSafeZone(mgr, GetUniqueId(), GetTouchBounds()->GetCenterPoint(), radii,
                                 mActivation);
    if (light) {
      if (!lightActive) {
        mgr.SendScriptMsg(light, GetUniqueId(), kSM_Activate);
      }
      light->SetLight(BuildLight());
    }
  } else {
    manager->RemoveSafeZone(GetUniqueId());
    if (lightActive) {
      mgr.SendScriptMsg(light, GetUniqueId(), kSM_Deactivate);
    }
    SetLowPassFilter(false);
  }
}

void CScriptSafeZone::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Create: {
    if (mLifetime > 0.f) {
      mLifetime += mRandomLifetimeOffset * mgr.Random()->Float();
    }
    if (!mMobile || mGenerateMobileLight) {
      CTransform4f xf = GetTransform();
      if (mGenerateMobileLight) {
        xf.AddTranslation(mMobileLightOffset);
      }
      CGameLight* light = rs_new CGameLight(mgr.AllocateUniqueId(), GetCurrentAreaId(), GetActive(),
                                            rstl::string_l("Safezone Game Light"), xf,
                                            kInvalidUniqueId, BuildLight(), 0x5afe, 10000, 0.f);
      mgr.AddObject(*light);
      mLightId = light->GetUniqueId();
    }
    if (CEchoEmitter* emitter = EchoEmitter()) {
      emitter->CreateEmitter(mgr);
    }
    break;
  }
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
    }
    if (x262_ != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x262_);
    }
    SetLowPassFilter(false);
    if (CEchoEmitter* emitter = EchoEmitter()) {
      emitter->DestroyEmitter(mgr);
    }
    break;
  case kSM_Activate:
    if (!GetActive() && mActivation == 0.f) {
      mShellPulse = 2.f;
      PlayActivateSound(mgr);
      UpdateObstruction(mgr, true);
      SetLowPassFilter(mCameraInside);
    }
    break;
  case kSM_Deactivate:
    if (GetActive()) {
      PlayDeactivateSound(mgr);
      UpdateObstruction(mgr, false);
      SetLowPassFilter(false);
    }
    break;
  case kSM_InternalMessage3:
    if (!GetActive()) {
      mActivation = 1.f;
      UpdateObstruction(mgr, true);
      SetLowPassFilter(mCameraInside);
      UpdateSafeZoneManager(mgr);
    }
    CScriptTrigger::AcceptScriptMsg(mgr, CScriptMsg(msg.GetSenderId(), msg.GetId(), kSM_Activate,
                                                    msg.GetOriginator(), msg.GetState()));
    return;
  case kSM_InternalMessage2:
    if (GetActive()) {
      mActivation = 0.f;
      UpdateObstruction(mgr, false);
      SetLowPassFilter(false);
      UpdateSafeZoneManager(mgr);
    }
    CScriptTrigger::AcceptScriptMsg(mgr, CScriptMsg(msg.GetSenderId(), msg.GetId(), kSM_Deactivate,
                                                    msg.GetOriginator(), msg.GetState()));
    return;
  case kSM_AreaLoaded:
    if (GetActive()) {
      CSfxManager::SetChannel(CSfxManager::kSC_Game);
      PlaySound(mgr, mCurrentInfo->x2_, 0x80000000);
      UpdateObstruction(mgr, true);
    }
    break;
  case kSM_Increment:
    if (mZoneType != kZT_Hurtful) {
      SetZoneType(kZT_Hurtful);
      mShellPulse = 2.f;
      mInsideTime = 0.f;
      if (GetActive()) {
        UpdateObstruction(mgr, true);
        PlayActivateSound(mgr);
        SetLowPassFilter(mCameraInside);
      }
    }
    break;
  case kSM_InternalMessage0:
    if (mZoneType != kZT_Echo) {
      SetZoneType(kZT_Echo);
      mShellPulse = 2.f;
      mInsideTime = 0.f;
      if (GetActive()) {
        UpdateObstruction(mgr, true);
        PlayActivateSound(mgr);
        SetLowPassFilter(mCameraInside);
      }
    }
    break;
  case kSM_Decrement:
    if (mZoneType != kZT_Normal) {
      SetZoneType(kZT_Normal);
      mShellPulse = 2.f;
      mInsideTime = 0.f;
      if (GetActive()) {
        UpdateObstruction(mgr, true);
        PlayActivateSound(mgr);
        SetLowPassFilter(mCameraInside);
      }
    }
    break;
  case kSM_InternalMessage1:
    if (GetActive()) {
      PlaySound(mgr, mFlashSound, 0);
      mFlashTimer = 1.f;
    }
    break;
  default:
    break;
  }
  CScriptTrigger::AcceptScriptMsg(mgr, msg);
}

void CScriptSafeZone::UpdateLoopSound(float dt, CStateManager& mgr) {
  if (mLoopSoundDelay >= mCurrentInfo->xc_) {
    if (!FindLoopedSound(mCurrentInfo->x2_)) {
      PlaySound(mgr, mCurrentInfo->x2_, 0x80000000);
    }
  } else {
    mLoopSoundDelay = rstl::min_val(mLoopSoundDelay + dt, mCurrentInfo->xc_);
  }
}

void CScriptSafeZone::UpdateFlash(float dt, CStateManager& mgr) {
  if (mFlashTimer > 0.f) {
    mFlashTimer -= dt / mFlashBrightness;
    if (mFlashTimer < 0.f) {
      mFlashTimer = 0.f;
    }
  }
}

void CScriptSafeZone::UpdateInsideAlpha(float dt, CStateManager& mgr) {
  if (mCameraInside) {
    mInsideTime += dt;
    if (mInsideTime < mInsideFadeStart) {
      mInsideAlpha = 1.f;
    } else if (mInsideTime < mInsideFadeTime) {
      const float t = 1.f - (mInsideTime - mInsideFadeStart) / (mInsideFadeTime - mInsideFadeStart);
      mInsideAlpha = t + (1.f - t) * mInsideFadeMinAlpha;
    } else {
      mInsideAlpha = mInsideFadeMinAlpha;
    }
  } else {
    mInsideTime = 0.f;
    mInsideAlpha = 1.f;
  }
}

void CScriptSafeZone::ApplyInsideFilter(CStateManager& mgr) {
  CCameraFilterPass& pass = mgr.CameraFilterPass(0, 10);
  if (pass.GetCurrentType() == CCameraFilterPass::kFT_Passthru) {
    pass.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                   mInsideFilterColor, kInvalidAssetId);
    pass.DisableFilter(mInsideFilterTime);
  }
}

void CScriptSafeZone::ApplyFog(CStateManager& mgr, const CSafeZoneFog& fog) {
  if (fog.mEnabled) {
    const TAreaId aid = GetCurrentAreaId();
    CGameArea::CAreaFog* areaFog = mgr.World()->Area(aid)->GetPostConstructed()->mAreaFog.get();
    if (fog.mMode != kRFM_None) {
      areaFog->FadeFog(fog.mMode, fog.mColor, fog.mRange, fog.mColorRate, fog.mRangeRate);
    } else {
      areaFog->RollFogOut(fog.mRangeRate.GetX(), fog.mColorRate, fog.mColor);
    }
  }
}

void CScriptSafeZone::PreRenderAllViewports(CStateManager& mgr) {
  float x = mActivation * GetScale().GetX();
  float y = mActivation * GetScale().GetY();
  float z = mActivation * GetScale().GetZ();
  if (x < mMinRadius) {
    x = mMinRadius;
  }
  if (y < mMinRadius) {
    y = mMinRadius;
  }
  if (z < mMinRadius) {
    z = mMinRadius;
  }
  const CVector3f center = GetTouchBounds()->GetCenterPoint();
  const CAABox bounds(center - CVector3f(x, y, z), center + CVector3f(x, y, z));
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

void CScriptSafeZone::InhabitantAdded(CActor& actor, CStateManager& mgr) {
  CScriptTrigger::InhabitantAdded(actor, mgr);
  CGameProjectile* proj = TCastToPtr< CGameProjectile >(actor);
  if (proj && HasInhabitant(proj->GetOwnerId())) {
    if (rstl::find< rstl::list< TUniqueId >::const_iterator >(
            mProjectiles.begin(), mProjectiles.end(), proj->GetUniqueId()) == mProjectiles.end()) {
      mProjectiles.push_back(proj->GetUniqueId());
    }
  } else {
    HandleProjectile(actor, mgr);
  }
  mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XENZ, kInvalidUniqueId);
  UpdatePlayerInside(actor, true, mgr);
}

void CScriptSafeZone::SpawnImpactEffect(const CVector3f& position, float scale) {
  CElementGen* gen =
      rs_new CElementGen(mImpactEffect, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  gen->SetTranslation(position);
  CVector3f lookFrom = GetTranslation();
  if (GetShape() == kST_Cylinder) {
    lookFrom.SetZ(position.GetZ());
  }
  CTransform4f xf = CTransform4f::LookAt(lookFrom, position, CVector3f::Up());
  gen->SetOrientation(xf.GetRotation());
  gen->SetGlobalScale(scale * gen->GetGlobalScale());
  mImpactGens.push_back(rstl::auto_ptr< CElementGen >(gen));
}

// Guessed name. Ray against a Z-aligned cylinder; the entry point is skipped when the ray starts
// inside the circle.
static bool RayCylinderIntersection(const CVector3f& center, const CVector3f& start,
                                    const CVector3f& dir, float& t, CVector3f& hit, float radius,
                                    float halfHeight, float maxDist) {
  const CVector2f delta = center.ToVec2f() - start.ToVec2f();
  const float b = CVector2f::Dot(delta, dir.ToVec2f());
  const float b2 = b * b;
  const float c = CVector2f::Dot(delta, delta);
  const float r2 = radius * radius;
  if (b < 0.f && c > r2) {
    return false;
  }
  const float disc = r2 - (c - b2);
  if (disc < 0.f) {
    return false;
  }
  const float s = CMath::SqrtF(disc);
  const float t0 = b - s;
  const float t1 = b + s;
  const float zMin = center.GetZ() - halfHeight;
  const float zMax = halfHeight + center.GetZ();
  for (int i = 0; i < 2; ++i) {
    float tt;
    if (i == 0) {
      if (c <= r2) {
        continue;
      }
      tt = t0;
    } else {
      tt = t1;
    }
    if (tt < maxDist || maxDist == 0.f) {
      const CVector3f point = start + tt * dir;
      if (point.GetZ() >= zMin && point.GetZ() <= zMax) {
        hit = point;
        t = tt;
        return true;
      }
    }
  }
  return false;
}

void CScriptSafeZone::HandleProjectile(CActor& actor, CStateManager& mgr) {
  if (CGameProjectile* proj = TCastToPtr< CGameProjectile >(actor)) {
    if (rstl::find< rstl::list< TUniqueId >::const_iterator >(
            mProjectiles.begin(), mProjectiles.end(), proj->GetUniqueId()) == mProjectiles.end()) {
      const CVector3f delta = proj->GetTranslation() - proj->GetPreviousPos();
      if (delta.CanBeNormalized()) {
        const CVector3f dir = delta.AsNormalized();
        const float mag = delta.Magnitude();
        const CVector3f start = proj->GetPreviousPos() - 2.f * (mag * dir);
        const float maxDist = mag * 4.f;
        float t = 0.f;
        CVector3f hit = CVector3f::Zero();
        switch (GetShape()) {
        case kST_Cylinder:
          if (!RayCylinderIntersection(GetTranslation(), start, dir, t, hit, GetScale().GetX(),
                                       GetScale().GetZ(), maxDist)) {
            return;
          }
          break;
        case kST_Ellipsoid:
          if (!CollisionUtil::RaySphereIntersection(CSphere(GetTranslation(), GetScale().GetX()),
                                                    start, dir, maxDist, t, hit)) {
            return;
          }
          break;
        default:
          return;
        }
        SpawnImpactEffect(hit, 1.f);
      }
    }
  }
}

void CScriptSafeZone::InhabitantExited(CActor& actor, CStateManager& mgr) {
  CScriptTrigger::InhabitantExited(actor, mgr);
  HandleProjectile(actor, mgr);
  mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XEXZ, kInvalidUniqueId);
  UpdatePlayerInside(actor, false, mgr);
}

void CScriptSafeZone::UpdatePlayerInside(CActor& actor, bool inside, CStateManager& mgr) {
  if (TCastToPtr< CGameCamera >(actor) &&
      TCastToPtr< CGameCamera >(actor)->GetCameraManager(mgr).GetCurrentCameraId(true) ==
          actor.GetUniqueId()) {
    mCameraInside = inside;
    if (!mIgnoreCinematicCamera || !TCastToPtr< CCinematicCamera >(actor)) {
      if (mCameraInside) {
        PlaySound(mgr, mCurrentInfo->x6_, 0x40000000);
      } else {
        PlaySound(mgr, mCurrentInfo->x8_, 0x40000000);
      }
    } else {
      mFogDirty = true;
    }
  }
}

void CScriptSafeZone::AddToRenderer(const CStateManager& mgr) const {
  if (GetActive()) {
    CScriptTriggerEllipsoid::AddToRenderer(mgr);
    EnsureRendered(mgr);
  }
  for (rstl::list< rstl::auto_ptr< CElementGen > >::const_iterator it = mImpactGens.begin();
       it != mImpactGens.end(); ++it) {
    gpRender->AddParticleGen(**it);
  }
}

bool CScriptSafeZone::ShouldSendScriptMsgs(CActor& actor, CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(actor) != nullptr;
}

void CScriptSafeZone::RenderDarkVisorSpot(const CStateManager& mgr) const {
  if (mCurrentInfo->x10_) {
    (*mCurrentInfo->x10_)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    const CTransform4f& view = CGraphics::GetViewMatrix();
    const CVector3f pos = GetTranslation();
    const CVector3f toSpot = pos - view.GetTranslation();
    if (toSpot.CanBeNormalized()) {
      const CVector3f forward = view.GetForward();
      const float dot = CVector3f::Dot(toSpot.AsNormalized(), forward);
      if (!(dot < 0.f)) {
        const float alpha = mActivation * dot;
        const float size = mCurrentInfo->x20_ * alpha;
        const CVector3f right = size * view.GetRight();
        const CVector3f up = size * view.GetUp();
        CGraphics::SetModelMatrix(CTransform4f::Identity());
        CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
        CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
        CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
        CGraphics::SetDepthWriteMode(false, kE_Always, false);
        const CColor color(alpha, alpha, alpha, alpha);
        CGraphics::StreamColor(color);
        CGraphics::StreamBegin(kP_TriangleFan);
        CGraphics::StreamTexcoord(0.f, 0.f);
        CGraphics::StreamVertex((pos - right) + up);
        CGraphics::StreamTexcoord(1.f, 0.f);
        CGraphics::StreamVertex((pos - right) - up);
        CGraphics::StreamTexcoord(1.f, 1.f);
        CGraphics::StreamVertex((pos + right) - up);
        CGraphics::StreamTexcoord(0.f, 1.f);
        CGraphics::StreamVertex((pos + right) + up);
        CGraphics::StreamEnd();
      }
    }
  }
}

void CScriptSafeZone::ApplyRenderEffect(CStateManager& mgr) {
  bool inside = HasInhabitant(
      mgr.CameraManager(mgr.MaskUIdNumPlayers(mgr.mCurrentRenderPlayer->GetUniqueId()))
          ->GetCurrentCameraId(false));
  const float flash = mFlashTimer * mFlashTime;
  float pulse;
  if (inside) {
    pulse = 0.25f * mShellPulse + 0.5f * flash;
  } else {
    pulse = mShellPulse + flash;
  }
  float spotSize = 0.1f;
  if (!inside) {
    const CGameCamera* camera =
        mgr.CameraManager(mgr.mCurrentRenderPlayerIndex)->GetCurrentCamera(mgr, true);
    const float fov = camera->GetFov();
    const float invSin = 1.f / sinf(0.017453292f * fov);
    const CVector3f delta = GetTranslation() - camera->GetTranslation();
    const float invDist = CMath::FastInvSqrtF(delta.MagSquared());
    const float angle =
        57.295776f * acosf(CVector3f::Dot(camera->GetTransform().GetForward(), invDist * delta));
    const float edge = rstl::min_val(1.f, (2.f * fabsf(angle)) / (fov * camera->GetAspectRatio()));
    const float edgeScale = edge < 0.8f ? 1.f : 0.7f;
    const float maxScale =
        rstl::max_val(GetScale().GetX(), rstl::max_val(GetScale().GetY(), GetScale().GetZ()));
    spotSize = invSin * (mActivation * maxScale * invDist) * edgeScale;
    spotSize *= mMobile ? 2.25f : 2.5f;
  }
  const CVector3f& radii = mActivation * GetScale();
  const float alpha = CMath::Clamp(0.f, 255.f * pulse, 255.f);
  const float insideAlpha = CMath::Clamp(0.f, 127.f * mInsideAlpha, 255.f);
  mgr.AddDarkWorldSphereToRenderer(
      GetTranslation(), radii, CCast::ToUint8(alpha), CCast::ToUint8(insideAlpha), inside, spotSize,
      mCurrentInfo->mScroll1, mCurrentInfo->mScroll2, mCurrentInfo->mTexScale1,
      mCurrentInfo->mTexScale2, **mCurrentInfo->mEnvironment, **mCurrentInfo->mCloud1,
      **mCurrentInfo->mCloud2, mCurrentInfo->mColor, mCurrentInfo->mAdditiveColor,
      GetShape() == kST_Cylinder);
}

void CScriptSafeZone::Render(const CStateManager& mgr) const {
  if (mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    RenderDarkVisorSpot(mgr);
  }
  CScriptTriggerEllipsoid::Render(mgr);
}

void CScriptSafeZone::InhabitantIdle(CActor& actor, CStateManager& mgr, float dt) {
  CScriptTrigger::InhabitantIdle(actor, mgr, dt);
  if (IsAI(mgr, actor)) {
    DamageActor(mgr, actor.GetUniqueId(), dt);
  }
}

// Guessed helper; the per-frame damage is built in a by-value return slot.
static inline CDamageInfo ScaleDamage(const CDamageInfo& info, float dt) {
  return CDamageInfo(info, dt);
}

void CScriptSafeZone::DamageActor(CStateManager& mgr, TUniqueId id, float dt) {
  CDamageInfo damage =
      mZoneType == kZT_Normal ? ScaleDamage(mNormalDamage, dt) : ScaleDamage(mHurtfulDamage, dt);
  mgr.ApplyDamage(GetUniqueId(), id, GetUniqueId(), damage,
                  CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                  CVector3f::Zero());
}

void CScriptSafeZone::UpdateObstruction(CStateManager& mgr, bool enable) {
  if (mObstructionType != -1) {
    ModifyObstruction(mgr, -1, mObstructionType);
    mObstructionType = -1;
  }
  if (enable) {
    mObstructionRadius = GetScale().GetX();
    mObstructionPos = GetTranslation();
    mObstructionType = mZoneType;
    ModifyObstruction(mgr, 1, mObstructionType);
  }
}

void CScriptSafeZone::ModifyObstruction(CStateManager& mgr, int delta, int type) {
  const float radiusSq = mObstructionRadius * mObstructionRadius;
  const bool obstruction = type != 0;
  CPFArea* area = mgr.World()->Area(GetCurrentAreaId())->GetPostConstructed()->mPathArea;
  if (area != nullptr) {
    const CTransform4f& xf = area->GetTransform();
    const CVector3f point = xf.TransposeRotate(CVector3f(mObstructionPos.GetX() - xf.Get03(),
                                                         mObstructionPos.GetY() - xf.Get13(),
                                                         mObstructionPos.GetZ() - xf.Get23()));
    for (int i = 0; i < area->GetNumRegions(); ++i) {
      CPFRegion& region = area->GetRegion(i);
      const CVector3f d = region.GetCentroid() - point;
      if (d.MagSquared() <= radiusSq) {
        region.ModifyObstructionCount(static_cast< EPathFindObstructions >(obstruction), delta);
      }
    }
    mgr.InformListeners(mObstructionPos, static_cast< EListenNoiseType >(4));
  }
}

CSafeZoneFog::CSafeZoneFog(bool enabled, ERglFogMode mode, const CColor& color,
                           const CVector2f& range, float colorRate, const CVector2f& rangeRate)
: mEnabled(enabled)
, mMode(mode)
, mColor(color)
, mRange(range)
, mColorRate(colorRate)
, mRangeRate(rangeRate) {}

bool CScriptSafeZone::IsHurtful() const {
  return mZoneType == kZT_Hurtful || mZoneType == kZT_Echo;
}

// Guessed name.
CSafeZoneFog LdrToSafeZoneFog(const SLdrSafeZoneFog& data) {
  return CSafeZoneFog(data.enabled, FogSelectionToFogMode(data.mode), data.color,
                      LdrToVector2f(data.nearFarPlane), data.colorRate,
                      LdrToVector2f(data.distanceRate));
}

CEntity* LoadSafeZone(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSafeZone sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSafeZone.inc"

  const CVector3f forceField =
      mgr.World()->Area(info.GetAreaId())->GetTM().Rotate(sldrThis.trigger.forceField);
  const CVector3f scale = sldrThis.editorProperties.transform.scale;
  rstl::reserved_vector< CDarkWorldInfo, 3 > infos;
  const SLdrSafeZoneAttributes* attributes[3] = {
      &sldrThis.normalAttributes, &sldrThis.hurtfulAttributes, &sldrThis.echoAttributes};
  for (int i = 0; i < 3; ++i) {
    const SLdrSafeZoneAttributes& attr = *attributes[i];
    if (attr.shellEnvironmentMap == kInvalidAssetId || attr.shell1Texture == kInvalidAssetId ||
        attr.shell2Texture == kInvalidAssetId) {
      return nullptr;
    }
    infos.push_back(CDarkWorldInfo(
        attr.turnOnSound, attr.activeLoopSound, attr.turnOffSound, attr.playerEnterSound,
        attr.playerExitSound, attr.unknown_0xd4839a3f, attr.darkVisorSpotTexture,
        attr.darkVisorSpotMaxSize,
        CVector2f(attr.shell1AnimatedHorizRate, attr.shell1AnimatedVertRate),
        CVector2f(attr.shell2AnimatedHorizRate, attr.shell2AnimatedVertRate),
        CVector2f(attr.shell1ScaleHoriz, attr.shell1ScaleVert),
        CVector2f(attr.shell2ScaleHoriz, attr.shell2ScaleVert), attr.shellEnvironmentMap,
        attr.shell1Texture, attr.shell2Texture, attr.shellColor, attr.unknown_0xe68b1fa8));
  }

  const TUniqueId uid = mgr.AllocateUniqueId();
  const CScriptTriggerEllipsoid::EShapeType shape =
      static_cast< CScriptTriggerEllipsoid::EShapeType >(sldrThis.safezoneShape);
  return rs_new CScriptSafeZone(
      uid, sldrThis.editorProperties.name, LdrToEntityInfo(info, sldrThis.editorProperties), scale,
      LdrToTransform4f(sldrThis.editorProperties), LdrToDamageInfo(sldrThis.trigger.damage),
      forceField, sldrThis.activationTime, sldrThis.deactivationTime, sldrThis.lifetime,
      sldrThis.randomLifetimeOffset, sldrThis.insideFadeStart,
      sldrThis.insideFadeStart + sldrThis.insideFadeTime, sldrThis.insideFadeMinAlpha,
      sldrThis.flashTime, sldrThis.trigger.flagsTrigger, sldrThis.deactivateOnEnter,
      sldrThis.deactivateOnExit, sldrThis.impactEffect, infos[0], infos[1], infos[2],
      LdrToDamageInfo(sldrThis.normalDamage), LdrToDamageInfo(sldrThis.hurtfulDamage),
      sldrThis.filterSoundEffects, sldrThis.filterCutoffFrequency, sldrThis.ignoreCinematicCamera,
      sldrThis.mobile, sldrThis.generateMobileLight, sldrThis.mobileLightOffset, shape,
      LdrToSafeZoneFog(sldrThis.fogEntering), LdrToSafeZoneFog(sldrThis.fogLeaving),
      LdrToEchoParameters(sldrThis.echoParameters), sldrThis.flashBrightness, sldrThis.flashSound,
      sldrThis.unknown_0xe71b43e1, sldrThis.unknown_0x9f638987);
}

#ifndef MONOLITHIC
SSafeZone_FuncPtrs REL_loader_SafeZone;

void SetRelLoaderFunctionToLoader() {
  REL_loader_SafeZone.mLoadSafeZone = LoadSafeZone;
  REL_loader_SafeZone.mLoadSafeZoneCrystal = LoadSafeZoneCrystal;
  REL_loader_SafeZone.mApplyRenderEffect =
      static_cast< void (CEntity::*)(CStateManager&) >(&CScriptSafeZone::ApplyRenderEffect);
  SetSSafeZone_FuncPtrs(&REL_loader_SafeZone);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSSafeZone_FuncPtrs(nullptr); }
#endif
