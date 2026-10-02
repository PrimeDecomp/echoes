#include "MetroidPrime/CFluidPlaneManager.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "rstl/math.hpp"

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile;

void CFluidPlaneManager::CFluidProfile::Clear() {
  x10_ = 0.f;
  xc_ = 0.f;
  x8_ = 0.f;
  x4_ = 0.f;
  x0_ = 0.f;
}

CFluidPlaneManager::CFluidPlaneManager()
: mLastSplashPosition(CVector3f::Zero())
, mSplashCooldown(0.f)
, mUvTime(0.f)
, x118_(false)
, mFrameActive(false) {
  sProfile.Clear();
  for (CSplashRecord* it = mSplashes.begin(); it != mSplashes.end(); ++it) {
    it->SetTime(9999.f);
  }
}

void CFluidPlaneManager::Update(float dt) {
  mUvTime = dt + mUvTime;
  for (CSplashRecord* it = mSplashes.begin(); it != mSplashes.end(); ++it) {
    it->SetTime(dt + it->GetTime());
    if (it->GetTime() > 9999.f) {
      it->SetTime(9999.f);
    }
  }
  mSplashCooldown = rstl::max_val(0.f, mSplashCooldown - dt);
}

void CFluidPlaneManager::StartFrame(bool enabled) const {
  mFrameActive = enabled;
  sProfile.Clear();
}

void CFluidPlaneManager::EndFrame() const { mFrameActive = false; }

float CFluidPlaneManager::GetLastSplashDeltaTime(TUniqueId splasher) const {
  float newestTime = 9999.f;
  for (const CSplashRecord* it = mSplashes.begin(); it != mSplashes.end(); ++it) {
    if (splasher == it->GetUniqueId() && newestTime > it->GetTime()) {
      newestTime = it->GetTime();
    }
  }
  return newestTime;
}

void CFluidPlaneManager::CreateSplash(TUniqueId splasher, CStateManager& mgr,
                                      const CScriptWater& water, const CVector3f& pos, float factor,
                                      bool sfx) {
  if (!water.CanRippleAtPoint(pos)) {
    return;
  }

  if (sfx) {
    CSfxManager::AddEmitter(water.GetSplashSound(factor), pos, water.GetCurrentAreaId().Value(),
                            true, false, CSfxManager::kMedPriority);
  }
  if (mSplashCooldown > 0.f && (mLastSplashPosition - pos).MagSquared() < 4.f) {
    return;
  }

  mSplashCooldown = 0.4f;
  mLastSplashPosition = pos;
  float oldestTime = 0.f;
  CSplashRecord* oldestRecord = nullptr;
  for (CSplashRecord* it = mSplashes.begin(); it != mSplashes.end(); ++it) {
    if (it->GetTime() > oldestTime) {
      oldestRecord = it;
      oldestTime = it->GetTime();
    }
  }

  CSplashRecord newRecord(splasher);
  if (oldestRecord) {
    *oldestRecord = newRecord;
  } else {
    mSplashes.push_back(newRecord);
  }

  const float splashScale = water.GetSplashEffectScale(factor);
  if (water.GetSplashEffect(factor)) {
    CEntity* explosion = rs_new CExplosion(
        *water.GetSplashEffect(factor), mgr.AllocateUniqueId(),
        CEntityInfo(water.GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
        rstl::string_l("Splash"), CTransform4f(CMatrix3f::Identity(), pos), 0,
        CVector3f(splashScale, splashScale, splashScale), water.GetSplashColor(), -1);
    if (explosion) {
      mgr.AddObject(*explosion);
    }
  }
}
