#include "MetroidPrime/CHintState.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "rstl/algorithm.hpp"

CHintState::CHintState(TUniqueId hint, int priority, float timer,
                       CGameHint::EBreakHintType breakType, uint requiredPresses, float unknown30,
                       CGameHint::SCallback onExpire, CGameHint::SCallback onBreak,
                       float breakDelay)
: mHintId(hint)
, mPriority(priority)
, mTimer(timer)
, mBreakType(breakType)
, mPressCount(0)
, mRequiredPresses(requiredPresses)
, x2c_(0.f)
, x30_(unknown30)
, mOnExpire(onExpire)
, mOnBreak(onBreak)
, mBreakDelay(breakDelay)
, mForceRemoval(false) {}

TUniqueId CHintState::GetFirstSender() const {
  if (mSenders.size() > 0) {
    return mSenders.front();
  }
  return kInvalidUniqueId;
}

void CHintState::AddSender(TUniqueId sender) {
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), sender);
  if (it != mSenders.end()) {
    return;
  }
  mSenders.push_back(sender);
}

void CHintState::RemoveSender(TUniqueId sender, CStateManager& mgr) {
  if (mSenders.empty()) {
    return;
  }
  rstl::reserved_vector< TUniqueId, 8 >::iterator it =
      rstl::find(mSenders.begin(), mSenders.end(), sender);
  if (it == mSenders.end()) {
    mSenders.erase(mSenders.begin());
  } else {
    mSenders.erase(it);
  }
}

const bool CHintState::ProcessInput(const CFinalInput& input, const CControlMapper& mapper,
                                    CStateManager& mgr) {
  if (mBreakType == CGameHint::kBHT_None) {
    return false;
  }
  if (mTimer > 0.f) {
    return false;
  }

  bool remove = false;
  bool broken = false;
  switch (mBreakType) {
  case CGameHint::kBHT_Fire:
    if (mapper.GetPressInput(CControlMapper::kC_FireOrBomb, input,
                             CControlMapper::kFT_Unfiltered) &&
        ++mPressCount >= mRequiredPresses) {
      broken = true;
    }
    break;
  case CGameHint::kBHT_Turn:
    if ((mapper.GetPressInput(CControlMapper::kC_TurnLeft, input, CControlMapper::kFT_Unfiltered) ||
         mapper.GetPressInput(CControlMapper::kC_TurnRight, input,
                              CControlMapper::kFT_Unfiltered)) &&
        ++mPressCount >= mRequiredPresses) {
      broken = true;
    }
    break;
  case CGameHint::kBHT_Jump:
    if ((mapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input,
                              CControlMapper::kFT_Unfiltered) ||
         mapper.GetPressInput(CControlMapper::kC_JumpOrBoost2, input,
                              CControlMapper::kFT_Unfiltered)) &&
        ++mPressCount >= mRequiredPresses) {
      broken = true;
    }
    break;
  case CGameHint::kBHT_TriggersAndB:
    if (input.DLTrigger() && input.DRTrigger() && input.DB()) {
      broken = true;
    }
    break;
  case CGameHint::kBHT_Unknown5:
  default:
    break;
  }
  if (broken) {
    if (mBreakDelay != 0.f) {
      mTimer = mBreakDelay;
    } else {
      remove = true;
    }
    mOnBreak(mgr, *this);
  }
  return remove;
}

void CHintState::OnExpire(CStateManager& mgr) { mOnExpire(mgr, *this); }
