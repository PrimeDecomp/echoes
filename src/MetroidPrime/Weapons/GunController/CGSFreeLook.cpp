#include "MetroidPrime/Weapons/GunController/CGSFreeLook.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

CGSFreeLook::CGSFreeLook()
: mDelay(0.f), mCueAnimId(-1), mLoopState(-1), mGunId(0), mSetId(-1), mIdle(false) {}

bool CGSFreeLook::Update(CAnimData& data, float dt, CStateManager& mgr) {
  if (mCueAnimId != -1) {
    mDelay -= dt;
    if (mDelay <= 0.f) {
      data.EnableLooping(mLoopState == 1);
      data.SetAnimation(CAnimPlaybackParms(mCueAnimId, -1, 1.f, true), false);
      mDelay = 0.f;
      mCueAnimId = -1;
    }
  } else if (!data.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"))) {
    switch (mLoopState) {
    case 0:
      SetAnim(data, mGunId, mSetId, 1, mgr, 0.f);
      break;
    case 2:
      mLoopState = -1;
      return true;
    default:
      break;
    }
  }
  return false;
}

int CGSFreeLook::SetAnim(CAnimData& data, int gunId, int setId, int loopState, CStateManager& mgr,
                         float delay) {
  const int useLoopState = mIdle ? 1 : loopState;
  mIdle = false;

  const CPASDatabase& pas = data.GetPASDatabase();
  const rstl::pair< float, int > anim = pas.FindBestAnimation(
      CPASAnimParmData(pas::kAS_Step, CPASAnimParm::FromInt32(gunId),
                       CPASAnimParm::FromInt32(setId), CPASAnimParm::FromEnum(useLoopState)),
      *mgr.Random(), -1);

  const CPASAnimParm selectedSet = pas.GetAnimState(pas::kAS_Step)->GetAnimParmData(anim.second, 1);
  mGunId = gunId;
  mSetId = selectedSet.GetInt32Value();
  mLoopState = useLoopState;
  if (delay != 0.f) {
    mDelay = delay;
    mCueAnimId = anim.second;
  } else {
    data.EnableLooping(loopState == 1);
    data.SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
  }
  return anim.second;
}
