#include "MetroidPrime/CGameHintInfo.hpp"

#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CHintOptions.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"

#include <float.h>

const float CGameHintInfo::skHintTextTime = 3.f;

uint CHintOptions::GetBitCount(uint value) {
  uint count = 0;
  for (; value != 0; value >>= 1) {
    ++count;
  }
  return count;
}

CGameHintInfo::SHintLocation::SHintLocation(CInputStream& in)
: mMlvlId(in.ReadInt32())
, mMreaId(in.ReadInt32())
, mAreaId(in.ReadInt32())
, mStringId(in.ReadInt32()) {}

CGameHintInfo::CGameHint::CGameHint(CInputStream& in, int version)
: mName(in)
, mImmediateTime(in.ReadFloat())
, mNormalTime(in.ReadFloat())
, mStringId(in.ReadInt32())
, mTextTime(CGameHintInfo::skHintTextTime *
            static_cast< float >(version > 0 ? in.ReadInt32() : 1))
, mLocations(in) {}

CGameHintInfo::CGameHintInfo(CInputStream& in, int version) {
  mHints.reserve(in.ReadInt32());
  for (int i = 0; i < mHints.capacity(); ++i) {
    mHints.push_back_unsafe(CGameHint(in, version));
  }
}

CHintOptions::SHintState::SHintState() : mState(kHS_Zero), mTime(0.f), mDismissalTimer(0.f) {}

CHintOptions::SHintState::SHintState(EHintState state, float time)
: mState(state), mTime(time), mDismissalTimer(0.f) {}

bool CHintOptions::SHintState::CanContinue() const {
  return mTime / CGameHintInfo::skHintTextTime < 1.f;
}

CHintOptions::CHintOptions()
: mNextHintIdx(-1), mInRezbitState(false), mScanDisplayActive(false) {}

CHintOptions::CHintOptions(CBitStreamReader& in)
: mNextHintIdx(-1), mInRezbitState(false), mScanDisplayActive(false) {
  mHintStates.reserve(gpMemoryCard->GetHints().size());
  for (int i = 0; i < mHintStates.capacity(); ++i) {
    const EHintState state = static_cast< EHintState >(in.ReadBits(GetBitCount(kHS_Delayed)));
    const uint timeBits = in.ReadBits(32);
    const float hintTime = reinterpret_cast< const float& >(timeBits);
    mHintStates.push_back_unsafe(SHintState(
        state, state == kHS_Waiting || state == kHS_Displaying ? hintTime : 0.f));
    if (mNextHintIdx == -1 && state == kHS_Displaying) {
      mNextHintIdx = i;
    }
  }
}

void CHintOptions::EnsureHintNextTime() {
  if (mNextHintIdx != -1) {
    SHintState& state = mHintStates[mNextHintIdx];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
    state.mTime = rstl::max_val(state.mTime, 5.f + hint.GetTextTime());
  }
}

void CHintOptions::PutTo(CBitStreamWriter& out) const {
  for (rstl::vector< SHintState >::const_iterator it = mHintStates.begin();
       it != mHintStates.end(); ++it) {
    out.WriteBits(it->mState, GetBitCount(kHS_Delayed));
    out.WriteBits(reinterpret_cast< const uint& >(it->mTime), 32);
  }
}

void CHintOptions::InitializeMemoryState() {
  const int count = gpMemoryCard->GetHints().size();
  mHintStates.assign(count);
}

void CHintOptions::Update(float dt, CStateManager& mgr) {
  mNextHintIdx = -1;
  for (int i = 0; i < mHintStates.size(); ++i) {
    SHintState& state = mHintStates[i];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[i];
    switch (state.mState) {
    case kHS_Zero:
    case kHS_Delayed:
      break;
    case kHS_Waiting:
      state.mTime -= dt;
      if (mInRezbitState || mScanDisplayActive) {
        state.mTime = rstl::max_val(FLT_EPSILON, state.mTime);
      }
      if (state.mTime <= 0.f) {
        state.mState = kHS_Displaying;
        state.mTime = hint.GetTextTime();
      }
      break;
    case kHS_Displaying:
      if (mNextHintIdx == -1) {
        mNextHintIdx = i;
      }
      break;
    }
  }

  if (mNextHintIdx == -1) {
    return;
  }
  SHintState& state = mHintStates[mNextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
  if (mInRezbitState || mScanDisplayActive) {
    state.mTime = rstl::max_val(hint.GetTextTime(), state.mTime - dt);
  } else {
    state.mTime = rstl::max_val(0.f, state.mTime - dt);
  }
  if (state.mTime == 0.f && state.IsDismissed()) {
    state.mDismissalTimer -= dt;
    if (state.mDismissalTimer <= 0.f) {
      state.mTime = hint.GetNormalTime();
      state.mDismissalTimer = 20.f;
    }
  }
  if (state.mTime < hint.GetTextTime()) {
    const rstl::vector< CGameHintInfo::SHintLocation >& locations = hint.GetLocations();
    for (int i = 0; i < locations.size(); ++i) {
      const CGameHintInfo::SHintLocation& loc = locations[i];
      if (loc.mMlvlId == mgr.GetWorld()->GetWorldAssetId() &&
          loc.mAreaId == mgr.GetNextAreaId()) {
        state.mTime = hint.GetNormalTime();
        state.mDismissalTimer = 20.f;
        return;
      }
    }
  }
}

void CHintOptions::ActivateImmediateHintTimer(const rstl::string& name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[idx];
  SHintState& state = mHintStates[idx];
  if (state.mState == kHS_Zero) {
    state.mState = kHS_Waiting;
    state.mTime = hint.GetImmediateTime();
  }
}

void CHintOptions::DelayHint(const rstl::string& name) {
  const int idx = CGameHintInfo::FindHintIndex(name);
  if (idx == -1) {
    return;
  }
  SHintState& state = mHintStates[idx];
  if (idx == mNextHintIdx) {
    for (rstl::vector< SHintState >::iterator it = mHintStates.begin(); it != mHintStates.end();
         ++it) {
      it->mTime += 60.f;
    }
  }
  state.mState = kHS_Delayed;
}

void CHintOptions::ActivateContinueDelayHintTimer(const rstl::string& name) {
  int idx = mNextHintIdx;
  if (static_cast< int >(name.size()) != 0) {
    idx = CGameHintInfo::FindHintIndex(name);
  }
  if (idx == -1) {
    return;
  }
  SHintState& state = mHintStates[idx];
  if (state.mState != kHS_Displaying) {
    return;
  }
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[idx];
  state.mTime = hint.GetTextTime();
}

const CHintOptions::SHintState* CHintOptions::GetCurrentDisplayedHint() const {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    if (mNextHintIdx == -1) {
      return nullptr;
    }
    const SHintState& state = mHintStates[mNextHintIdx];
    const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
    if (state.mTime >= hint.GetTextTime()) {
      return nullptr;
    }
    if (state.mTime >= CGameHintInfo::skHintTextTime) {
      return state.IsDismissed() ? nullptr : &state;
    } else {
      return &state;
    }
  }
  return nullptr;
}

int CHintOptions::GetNextHintIdx() {
  if (gpGameState->GameOptions().GetIsHintSystemEnabled()) {
    return mNextHintIdx;
  }
  return -1;
}

int CGameHintInfo::FindHintIndex(const rstl::string& name) {
  const rstl::vector< CGameHint >& hints = gpMemoryCard->GetHints();
  for (int i = 0; i < hints.size(); ++i) {
    if (hints[i].GetName() == name) {
      return i;
    }
  }
  return -1;
}

void CHintOptions::DismissDisplayedHint() {
  if (mNextHintIdx == -1) {
    return;
  }
  SHintState& state = mHintStates[mNextHintIdx];
  const CGameHintInfo::CGameHint& hint = gpMemoryCard->GetHints()[mNextHintIdx];
  if (state.mTime < hint.GetTextTime()) {
    state.mTime = hint.GetNormalTime();
    state.mDismissalTimer = 20.f;
  }
}

const CFactoryFnReturn FHintFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& params) {
  in.ReadInt32();
  const int version = in.ReadInt32();
  return rs_new CGameHintInfo(in, version);
}
