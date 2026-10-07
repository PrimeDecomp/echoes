#include "MetroidPrime/CHintManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/algorithm.hpp"

CHintManager::CHintManager(int playerIndex, const rstl::string& name)
: mPlayerIndex(playerIndex)
, mAreaId(kInvalidAreaId)
, mCurrentHintId(kInvalidUniqueId)
, mPriority(1000) {
  mHints.reserve(8);
  mRemovedHints.reserve(8);
  mAddedHints.reserve(8);
}

CHintManager::~CHintManager() {}

CHintState* CHintManager::GetBestHintState() {
  return mHints.empty() ? nullptr : &mHints.front().mState;
}

const CHintState* CHintManager::GetHintState(TUniqueId hint) const {
  for (rstl::vector< SHint >::const_iterator it = mHints.begin(); it != mHints.end(); ++it) {
    if (it->mState.GetHintId() == hint) {
      return &(*it).mState;
    }
  }
  return nullptr;
}

CHintState* CHintManager::GetHintState(TUniqueId hint) {
  for (rstl::vector< SHint >::iterator it = mHints.begin(); it != mHints.end(); ++it) {
    CHintState& state = it->mState;
    if (state.GetHintId() == hint) {
      return &state;
    }
  }
  return nullptr;
}

bool CHintManager::SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged, bool force) {
  bool selected = false;
  if (CEntity* entity = mgr.ObjectById(hint->GetHintId())) {
    if (mCurrentHintId == hint->GetHintId()) {
      selected = true;
    } else {
      mCurrentHintId = hint->GetHintId();
      mPriority = hint->GetPriority();
      entity->SendScriptMsgs(kSS_Active, mgr, entity->GetUniqueId(), kSM_None);
      selected = true;
    }
  }
  return selected;
}

void CHintManager::ClearHint(CStateManager& mgr, bool areaChanged) { ClearCurrentHint(1000); }

const bool CHintManager::ProcessRemovedHints(CStateManager& mgr) {
  bool removedCurrent = false;
  if (!mRemovedHints.empty()) {
    for (rstl::vector< THintSender >::iterator request = mRemovedHints.begin();
         request != mRemovedHints.end(); ++request) {
      const TUniqueId hint = request->first;
      const CGameHint* actor = TCastToConstPtr< CGameHint >(mgr.GetObjectById(hint));
      bool found = false;
      for (rstl::vector< SHint >::iterator it = mHints.begin(); it != mHints.end(); ++it) {
        CHintState& state = it->mState;
        if (state.GetHintId() == hint) {
          found = true;
          state.RemoveSender(request->second, mgr);
        }
        if (found && (!state.HasSenders() || state.GetForceRemoval())) {
          mHints.erase(it);
          if (actor && actor->GetDeleteOnRemoval()) {
            mgr.DeleteObjectRequest(actor->GetUniqueId());
          }
          if (hint == mCurrentHintId) {
            removedCurrent = true;
            OnHintRemoved(mgr);
          }
          break;
        }
      }
    }
    mRemovedHints.clear();
  }
  return removedCurrent;
}

bool CHintManager::AddHintState(int priority, const CHintState& hint) {
  if (mHints.size() == mHints.capacity()) {
    mHints.reserve(mHints.capacity() * 2);
  }
  mHints.push_back_unsafe(SHint(priority, hint));
  return true;
}

bool CHintManager::ProcessAddedHints(CStateManager& mgr) {
  bool added = false;
  if (!mAddedHints.empty()) {
    for (rstl::vector< THintSender >::iterator request = mAddedHints.begin();
         request != mAddedHints.end(); ++request) {
      const TUniqueId hint = request->first;
      const CGameHint* actor = TCastToConstPtr< CGameHint >(mgr.GetObjectById(hint));
      if (!actor) {
        continue;
      }

      bool found = false;
      for (rstl::vector< SHint >::iterator it = mHints.begin(); it != mHints.end(); ++it) {
        CHintState& state = it->mState;
        if (state.GetHintId() == hint) {
          state.AddSender(request->second);
          state.SetForceRemoval(false);
          found = true;
          break;
        }
      }
      if (found) {
        continue;
      }

      bool cancelledRemoval = false;
      for (rstl::vector< THintSender >::iterator it = mRemovedHints.begin();
           it != mRemovedHints.end(); ++it) {
        if (it->first == hint) {
          cancelledRemoval = true;
          mRemovedHints.erase(it);
          break;
        }
      }
      if (!cancelledRemoval) {
        CHintState state(hint, actor->GetPriority(), actor->GetTimer(), actor->GetBreakType(),
                         actor->GetRequiredPresses(), actor->GetUnknown16c(), actor->GetOnExpire(),
                         actor->GetOnBreak(), actor->GetBreakDelay());
        state.AddSender(request->second);
        added = AddHintState(actor->GetPriority(), state);
      }
    }
    mAddedHints.clear();
  }
  return added;
}

bool CHintManager::SelectHintFromStack(CHintState* hint, CStateManager& mgr, bool areaChanged) {
  return hint->GetPriority() < mPriority;
}

void CHintManager::Update(float dt, CStateManager& mgr) {
  bool invalidRemoved = false;
  for (rstl::vector< SHint >::iterator it = mHints.begin(); it != mHints.end();) {
    CHintState& state = it->mState;
    if (!TCastToPtr< CGameHint >(mgr.ObjectById(state.GetHintId()))) {
      if (!mHints.empty()) {
        it = mHints.erase(it);
        invalidRemoved = true;
      }
      continue;
    }
    if (state.GetTimer() > 0.f) {
      state.SetTimer(state.GetTimer() - dt);
      if (state.GetTimer() <= 0.f) {
        state.OnExpire(mgr);
        ForceRemoveHint(state.GetHintId(), mgr, kInvalidUniqueId);
      }
    }
    ++it;
  }

  const bool added = ProcessAddedHints(mgr);
  const uchar removed = ProcessRemovedHints(mgr);
  bool areaChanged = false;
  const TAreaId area = mgr.GetPlayer(mPlayerIndex)->GetCurrentAreaId();
  if (area != mAreaId) {
    areaChanged = true;
    mAreaId = area;
  }
  if (!removed && !added && !invalidRemoved && !areaChanged) {
    return;
  }

  rstl::less< SHint > compare;
  rstl::sort(mHints.begin(), mHints.end(), compare);
  if ((removed || invalidRemoved) && mHints.empty()) {
    ClearHint(mgr, areaChanged);
    return;
  }

  CGameHint* actor = nullptr;
  CHintState* state = nullptr;
  bool usable = false;
  for (rstl::vector< SHint >::iterator it = mHints.begin(); it != mHints.end(); ++it) {
    state = &it->mState;
    actor = TCastToPtr< CGameHint >(mgr.ObjectById(state->GetHintId()));
    if (actor) {
      if (actor->GetCurrentAreaId() != mAreaId) {
        if (actor->GetAcrossAreas()) {
          usable = true;
        }
      } else {
        usable = true;
        break;
      }
    }
  }
  if (!usable && !mHints.empty()) {
    ClearHint(mgr, areaChanged);
  }
  if (actor && usable) {
    if (mHints.size() == 1) {
      SetHint(state, mgr, areaChanged, false);
    } else {
      SelectHintFromStack(state, mgr, areaChanged);
    }
  }
}

bool CHintManager::ContainsHint(const rstl::vector< THintSender >& hints, TUniqueId hint) {
  for (rstl::vector< THintSender >::const_iterator it = hints.begin(); it != hints.end(); ++it) {
    if (it->first == hint) {
      return true;
    }
  }
  return false;
}

bool CHintManager::ContainsHint(const rstl::vector< SHint >& hints, TUniqueId hint) {
  for (rstl::vector< SHint >::const_iterator it = hints.begin(); it != hints.end(); ++it) {
    if (it->mState.GetHintId() == hint) {
      return true;
    }
  }
  return false;
}

void CHintManager::AddHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr) {
  if (TCastToPtr< CGameHint >(mgr.ObjectById(hint))) {
    if (mAddedHints.size() == mAddedHints.capacity()) {
      mAddedHints.reserve(mAddedHints.capacity() * 2);
    }
    mAddedHints.push_back_unsafe(THintSender(hint, sender));
  }
}

void CHintManager::ForceRemoveHint(TUniqueId hint, CStateManager& mgr, TUniqueId sender) {
  if (!TCastToPtr< CGameHint >(mgr.ObjectById(hint))) {
    return;
  }
  if (ContainsHint(mRemovedHints, hint)) {
    return;
  }
  if (!ContainsHint(mHints, hint)) {
    return;
  }
  if (mRemovedHints.size() == mRemovedHints.capacity()) {
    mRemovedHints.reserve(mRemovedHints.capacity() * 2);
  }
  if (CHintState* state = GetHintState(hint)) {
    state->SetForceRemoval(true);
  }
  mRemovedHints.push_back_unsafe(THintSender(hint, sender));
}

void CHintManager::RemoveHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr) {
  if (TCastToPtr< CGameHint >(mgr.ObjectById(hint))) {
    if (mRemovedHints.size() == mRemovedHints.capacity()) {
      mRemovedHints.reserve(mRemovedHints.capacity() * 2);
    }
    mRemovedHints.push_back_unsafe(THintSender(hint, sender));
  }
}

void CHintManager::RefreshHint(CStateManager& mgr) {}

const CGameHint* CHintManager::GetCurrentHint(const CStateManager& mgr) const {
  return TCastToConstPtr< CGameHint >(mgr.GetObjectById(mCurrentHintId));
}

bool CHintManager::HasHint(const CStateManager& mgr) const {
  if (!mHints.empty() && mCurrentHintId != kInvalidUniqueId) {
    if (mgr.GetObjectById(mCurrentHintId)) {
      return true;
    }
  }
  return false;
}

void CHintManager::OnHintRemoved(CStateManager& mgr) {}

void CHintManager::Reset(CStateManager& mgr) {
  mHints.clear();
  mRemovedHints.clear();
  mAddedHints.clear();
  ClearCurrentHint(1000);
}
