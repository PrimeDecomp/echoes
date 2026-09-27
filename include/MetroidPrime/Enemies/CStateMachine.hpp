#ifndef _CSTATEMACHINE
#define _CSTATEMACHINE

#include "rstl/vector.hpp"
#include "string.h"
#include "types.h"

class CInputStream;
class CState;

// Guessed name. AFSM stores callback names; each owner binds its own member functions.
class CTrigger {
public:
  CTrigger()
  : mArg(0.f), mIndex(0), mAndTrigger(nullptr), mState(nullptr), mLNot(false), mDefault(true) {
    memset(mName, 0, sizeof(mName));
  }

  void Setup(const char* name, bool lnot, float arg, CTrigger* andTrigger);
  void Setup(const char* name, bool lnot, float arg, CState* state);
  const char* GetName() const { return mName; }
  float GetArg() const { return mArg; }
  int GetIndex() const { return mIndex; }
  void SetIndex(int index) { mIndex = index; }
  const CTrigger* GetAnd() const { return mAndTrigger; }
  const CState* GetState() const { return mState; }
  bool IsNot() const { return mLNot; }
  bool IsDefault() const { return mDefault; }

private:
  char mName[32];
  float mArg;
  int mIndex;
  CTrigger* mAndTrigger;
  CState* mState;
  bool mLNot : 1;
  bool mDefault : 1;
};
CHECK_SIZEOF(CTrigger, 0x34)

// Guessed name. Prime's CAiState becomes an owner-independent resource record in Echoes.
class CState {
public:
  explicit CState(const char* name);
  const char* GetName() const { return mName; }
  int GetIndex() const { return mIndex; }
  void SetIndex(int index) { mIndex = index; }
  int GetNumTriggers() const { return mNumTriggers; }
  void SetNumTriggers(int count) { mNumTriggers = count; }
  void SetTriggers(CTrigger* triggers) { mFirstTrigger = triggers; }
  CTrigger* GetTrig(int index) const { return &mFirstTrigger[index]; }
  bool IsComment() const { return mComment; }

private:
  char mName[32];
  int mIndex;
  int mNumTriggers;
  CTrigger* mFirstTrigger;
  bool mComment : 1;
};
CHECK_SIZEOF(CState, 0x30)

class CStateMachine {
public:
  explicit CStateMachine(CInputStream& in);
  const rstl::vector< CState >& GetStateVector() const { return mStates; }
  const rstl::vector< CTrigger >& GetTriggerVector() const { return mTriggers; }

private:
  rstl::vector< CState > mStates;
  rstl::vector< CTrigger > mTriggers;
};
CHECK_SIZEOF(CStateMachine, 0x20)

#endif // _CSTATEMACHINE
