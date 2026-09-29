#include "MetroidPrime/Weapons/GunController/CGunMotion.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CStateManager.hpp"

CGunMotion::CGunMotion(CAssetId ancsId, const CVector3f& scale)
: mModelData(CAnimRes(ancsId, 0, scale, 0, false))
, mGunController(mModelData)
, mAnimPlaying(false) {
  LoadAnimations();
}

CGunMotion::~CGunMotion() {}

bool CGunMotion::PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle,
                             bool bigStrike) {
  CAnimData& data = *mModelData.AnimationData();
  const CPASDatabase& pas = data.GetPASDatabase();

  bool loop = true;
  int animId = -1;
  switch (state) {
  case SamusGun::kAS_Wander: {
    const rstl::pair< float, int > anim =
        pas.FindBestAnimation(CPASAnimParmData(pas::EAnimationState(state)), *mgr.Random(), -1);
    animId = anim.second;
    break;
  }
  case SamusGun::kAS_Idle: {
    const rstl::pair< float, int > anim = pas.FindBestAnimation(
        CPASAnimParmData(pas::EAnimationState(state), CPASAnimParm::FromEnum(0)), *mgr.Random(),
        -1);
    animId = anim.second;
    break;
  }
  case SamusGun::kAS_Struck: {
    const rstl::pair< float, int > anim = pas.FindBestAnimation(
        CPASAnimParmData(pas::EAnimationState(state), CPASAnimParm::FromInt32(0),
                         CPASAnimParm::FromReal32(angle), CPASAnimParm::FromBool(bigStrike),
                         CPASAnimParm::FromBool(false)),
        *mgr.Random(), -1);
    animId = anim.second;
    loop = false;
    break;
  }
  case SamusGun::kAS_FreeLook:
    mGunController.EnterFreeLook(mgr, 0, -1);
    break;
  case SamusGun::kAS_ComboFire:
    mGunController.EnterComboFire(mgr, 0);
    break;
  default:
    break;
  }

  if (animId != -1) {
    mAnimPlaying = true;
    data.EnableLooping(loop);
    data.SetAnimation(CAnimPlaybackParms(animId, -1, 1.f, true), false);
  }
  return loop;
}

void CGunMotion::Update(float dt, CStateManager& mgr) {
  mModelData.AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
  if (mGunController.Update(dt, mgr)) {
    mAnimPlaying = false;
  }
}

void CGunMotion::Draw(const CStateManager& mgr, const CTransform4f& xf) const {
  mModelData.Render(mgr, xf, nullptr, CModelFlags::Normal());
}

void CGunMotion::ReturnToDefault(CStateManager& mgr, bool reset) {
  mGunController.ReturnToDefault(mgr, 0.f, reset);
}

static inline int GetBasePositionAnimation(bool bigStrikeReset) {
  int animation = 0;
  if (bigStrikeReset) {
    animation = 19;
  }
  return animation;
}

void CGunMotion::BasePosition(bool bigStrikeReset) {
  CAnimData& data = *mModelData.AnimationData();
  data.EnableLooping(false);
  data.SetAnimation(CAnimPlaybackParms(GetBasePositionAnimation(bigStrikeReset), -1, 1.f, true),
                    false);
}

void CGunMotion::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet) {
  mAnimPlaying = true;
  mGunController.EnterFidget(mgr, int(type), 0, animSet);
}
