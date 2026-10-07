#include "MetroidPrime/Enemies/CStateMachine.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

void CTrigger::Setup(const char* name, bool lnot, float arg, CTrigger* andTrigger) {
  if (name != nullptr) {
    strncpy(mName, name, sizeof(mName));
    mDefault = strncmp(mName, "Default", 7) == 0 || mName[0] == '\0';
  }
  mArg = arg;
  mAndTrigger = andTrigger;
  mLNot = lnot;
}

void CTrigger::Setup(const char* name, bool lnot, float arg, CState* state) {
  if (name != nullptr) {
    strncpy(mName, name, sizeof(mName));
    mDefault = strncmp(mName, "Default", 7) == 0 || mName[0] == '\0';
  }
  mArg = arg;
  mState = state;
  mLNot = lnot;
}

CState::CState(const char* name)
: mIndex(0), mNumTriggers(0), mFirstTrigger(nullptr), mComment(false) {
  strncpy(mName, name, sizeof(mName));
  if (strlen(mName) >= 2) {
    mComment = mName[0] == '/' && mName[1] == '/';
  }
}

CStateMachine::CStateMachine(CInputStream& in) {
  CTrigger* lastTrigger = nullptr;
  const int stateCount = in.ReadInt32();
  mStates.reserve(stateCount);

  char name[32];
  for (int i = 0; i < stateCount; ++i) {
    for (int j = 0; j < sizeof(name); ++j) {
      name[j] = in.ReadInt8();
      if (name[j] == '\0') {
        break;
      }
    }
    mStates.push_back_unsafe(CState(name));
  }

  mTriggers.reserve(in.ReadInt32());
  for (int i = 0; i < stateCount; ++i) {
    CState& state = mStates[i];
    const int firstTrigger = mTriggers.size();
    state.SetNumTriggers(in.ReadInt32());
    if (state.GetNumTriggers() == 0) {
      continue;
    }
    for (int j = 0; j < state.GetNumTriggers(); ++j) {
      mTriggers.push_back_unsafe(CTrigger());
    }
    state.SetTriggers(&mTriggers[firstTrigger]);

    for (int j = 0; j < state.GetNumTriggers(); ++j) {
      const int triggerCount = in.ReadInt32();
      for (int k = 0; k < triggerCount; ++k) {
        for (int n = 0; n < sizeof(name); ++n) {
          name[n] = in.ReadInt8();
          if (name[n] == '\0') {
            break;
          }
        }
        const float arg = in.ReadFloat();
        const bool lnot = name[0] == '!';
        const char* triggerName = lnot ? name + 1 : name;
        CTrigger* trigger;
        if (k < triggerCount - 1) {
          mTriggers.push_back_unsafe(CTrigger());
          trigger = &mTriggers.back();
        } else {
          trigger = state.GetTrig(j);
        }
        if (k == 0) {
          trigger->Setup(triggerName, lnot, arg, &mStates[in.ReadInt32()]);
        } else {
          trigger->Setup(triggerName, lnot, arg, lastTrigger);
        }
        lastTrigger = trigger;
      }
    }
  }

  for (int i = 0; i < mStates.size(); ++i) {
    mStates[i].SetIndex(i);
  }
  for (int i = 0; i < mTriggers.size(); ++i) {
    mTriggers[i].SetIndex(i);
  }
}
