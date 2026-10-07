#include "MetroidPrime/Weapons/GunController/CGunController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

CGunController::CGunController(CModelData& modelData)
: mModelData(modelData)
, mGunState(kGS_Inactive)
, mCurAnimId(-1)
, mAnimDone(true)
, mEnteredComboFire(false) {}

void CGunController::EnterFreeLook(CStateManager& mgr, int gunId, int setId) {
  if (mGunState != kGS_ComboFire && !mEnteredComboFire) {
    mCurAnimId = mFreeLook.SetAnim(*mModelData.AnimationData(), gunId, setId, 0, mgr, 0.f);
  } else {
    mFreeLook.SetLoopState(mComboFire.GetLoopState());
  }
  mGunState = kGS_FreeLook;
}

void CGunController::EnterComboFire(CStateManager& mgr, int gunId) {
  if (mGunState != kGS_FreeLook) {
    mCurAnimId = mComboFire.SetAnim(*mModelData.AnimationData(), gunId, 0, mgr, 0.f);
  } else {
    mComboFire.SetLoopState(mFreeLook.GetLoopState());
  }
  mGunState = kGS_ComboFire;
  mEnteredComboFire = true;
}

void CGunController::EnterFidget(CStateManager& mgr, int type, int gunId, int animSet) {
  mCurAnimId = mFidget.SetAnim(*mModelData.AnimationData(), type, gunId, animSet, mgr);
  mGunState = kGS_Fidget;
}

void CGunController::EnterStruck(CStateManager& mgr, float angle, bool bigStrike,
                                 bool notInFreeLook) {
  switch (mGunState) {
  case kGS_FreeLook:
    mFreeLook.SetIdle(true);
    break;
  case kGS_Inactive:
  case kGS_Fidget:
    break;
  default:
    return;
  }

  const CPASAnimParmData parms =
      CPASAnimParmData(pas::kAS_LieOnGround, CPASAnimParm::FromInt32(mFreeLook.GetGunId()),
                       CPASAnimParm::FromReal32(angle), CPASAnimParm::FromBool(bigStrike),
                       CPASAnimParm::FromBool(notInFreeLook));
  CAnimData& data = *mModelData.AnimationData();
  const rstl::pair< float, int > anim =
      data.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  data.EnableLooping(false);
  data.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mGunState = bigStrike ? kGS_BigStrike : kGS_Strike;
}

void CGunController::LoadFidgetAnimAsync(CStateManager& mgr, int type, int gunId, int animSet) {
  mFidget.LoadAnimAsync(*mModelData.AnimationData(), type, gunId, animSet, mgr);
}

int CGunController::Update(float dt, CStateManager& mgr) {
  CAnimData& data = *mModelData.AnimationData();
  mAnimDone = false;
  switch (mGunState) {
  case kGS_FreeLook:
    mAnimDone = mFreeLook.Update(data, dt, mgr);
    if (mAnimDone && mEnteredComboFire) {
      EnterComboFire(mgr, mFreeLook.GetGunId());
      mAnimDone = false;
    }
    break;
  case kGS_ComboFire:
    mAnimDone = mComboFire.Update(data, dt, mgr);
    break;
  case kGS_Fidget:
    mAnimDone = mFidget.Update(data, dt, mgr);
    break;
  case kGS_Strike:
    if (!data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"))) {
      mCurAnimId = mFreeLook.SetAnim(*mModelData.AnimationData(), mFreeLook.GetGunId(),
                                     mFreeLook.GetSetId(), 0, mgr, 0.f);
      mGunState = kGS_FreeLook;
    }
    break;
  case kGS_BigStrike:
  case kGS_Unknown8:
    mAnimDone = !data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"));
    break;
  case kGS_Inactive:
  case kGS_Default:
  case kGS_Idle:
  default:
    break;
  }
  if (mAnimDone) {
    mGunState = kGS_Inactive;
    mEnteredComboFire = false;
    return true;
  }
  return false;
}

void CGunController::EnterIdle(CStateManager& mgr) {
  CPASAnimParm parm = CPASAnimParm::NoParameter();
  switch (mGunState) {
  case kGS_FreeLook:
    parm = CPASAnimParm::FromEnum(1);
    mFreeLook.SetIdle(true);
    break;
  case kGS_ComboFire:
    parm = CPASAnimParm::FromEnum(1);
    mComboFire.SetIdle(true);
    break;
  default:
    return;
  }

  CAnimData& data = *mModelData.AnimationData();
  const rstl::pair< float, int > anim = data.GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_Locomotion, parm), *mgr.Random(), -1);
  data.EnableLooping(false);
  data.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mGunState = kGS_Idle;
  mEnteredComboFire = false;
}

void CGunController::ReturnToDefault(CStateManager& mgr, float delay, bool setState) {
  CAnimData& data = *mModelData.AnimationData();
  switch (mGunState) {
  case kGS_Strike:
    mGunState = kGS_FreeLook;
  case kGS_Idle:
    mFreeLook.SetIdle(false);
  case kGS_FreeLook:
    if (!setState) {
      mCurAnimId =
          mFreeLook.SetAnim(data, mFreeLook.GetGunId(), mFreeLook.GetSetId(), 2, mgr, delay);
      mEnteredComboFire = false;
    }
    break;
  case kGS_ComboFire:
    mCurAnimId = mComboFire.SetAnim(data, mComboFire.GetGunId(), 2, mgr, delay);
    break;
  case kGS_Fidget:
    ReturnToBasePosition(mgr);
    break;
  case kGS_BigStrike:
    mFreeLook.SetIdle(false);
    break;
  default:
    break;
  }
  if (setState) {
    mGunState = kGS_Default;
  }
}

void CGunController::Reset() {
  mAnimDone = true;
  mEnteredComboFire = false;
  mGunState = kGS_Inactive;
}

void CGunController::ReturnToBasePosition(CStateManager& mgr) {
  CAnimData& data = *mModelData.AnimationData();
  const rstl::pair< float, int > anim = data.GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(pas::kAS_KnockBack), *mgr.Random(), -1);
  data.EnableLooping(false);
  data.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  mCurAnimId = anim.second;
  mEnteredComboFire = false;
}
