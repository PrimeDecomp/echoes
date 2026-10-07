#include "MetroidPrime/Weapons/GunController/CGSFidget.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

CGSFidget::CGSFidget() : mUnknown(-1), mGunId(-1), mAnimSet(-1) {}

bool CGSFidget::Update(CAnimData& data, float dt, CStateManager& mgr) {
  return !data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"));
}

int CGSFidget::SetAnim(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr) {
  const CPASDatabase& pas = data.GetPASDatabase();
  const rstl::pair< float, int > anim = pas.FindBestAnimation(
      CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                       CPASAnimParm::FromInt32(animSet)),
      *mgr.Random(), -1);
  CPASAnimParm loopParm = pas.GetAnimState(pas::kAS_Getup)->GetAnimParmData(anim.second, 3);
  const bool loop = loopParm.GetBoolValue();
  mGunId = gunId;
  mAnimSet = animSet;
  if (anim.second != -1) {
    data.EnableLooping(loop);
    data.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
    UnLoadAnim();
  }
  return anim.second;
}

void CGSFidget::LoadAnimAsync(CAnimData& data, int type, int gunId, int animSet,
                              CStateManager& mgr) {
  const CPASDatabase& pas = data.GetPASDatabase();
  const rstl::pair< float, int > anim = pas.FindBestAnimation(
      CPASAnimParmData(pas::kAS_Getup, CPASAnimParm::FromEnum(type), CPASAnimParm::FromInt32(gunId),
                       CPASAnimParm::FromInt32(animSet)),
      *mgr.Random(), -1);
  if (anim.second != -1) {
    NWeaponTypes::get_token_vector(data, anim.second, mAnims, true);
  }
}

void CGSFidget::UnLoadAnim() {
  if (!mAnims.empty()) {
    mAnims = rstl::vector< CToken >();
  }
}

bool CGSFidget::IsAnimLoaded() const { return NWeaponTypes::are_tokens_ready(mAnims); }
