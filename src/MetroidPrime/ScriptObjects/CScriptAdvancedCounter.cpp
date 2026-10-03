#include "MetroidPrime/ScriptObjects/CScriptAdvancedCounter.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAdvancedCounter.hpp"

CScriptAdvancedCounter::CScriptAdvancedCounter(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, int initial, int max,
                                               bool autoReset,
                                               const rstl::reserved_vector< int, 10 >& conditions)
: CEntity(uid, info, name, 0)
, mInitial(initial)
, mCurrent(initial)
, mMax(max)
, mConditions(conditions)
, mAutoReset(autoReset) {}

CScriptAdvancedCounter::~CScriptAdvancedCounter() {}

void CScriptAdvancedCounter::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_SetToZero:
    if (GetActive()) {
      mCurrent = 0;
      SendCounterStates(mgr);
    }
    break;
  case kSM_SetToMax:
    if (GetActive()) {
      mCurrent = mMax;
      SendCounterStates(mgr);
    }
    break;
  case kSM_Decrement:
    if (GetActive()) {
      if (mCurrent > 0) {
        --mCurrent;
        SendCounterStates(mgr);
      } else if (mAutoReset) {
        mCurrent = mInitial;
        SendCounterStates(mgr);
      }
    }
    break;
  case kSM_Increment:
    if (GetActive()) {
      if (mCurrent < mMax) {
        ++mCurrent;
        SendCounterStates(mgr);
      } else if (mAutoReset) {
        mCurrent = mInitial;
        SendCounterStates(mgr);
      }
    }
    break;
  case kSM_Reset:
    if (GetActive()) {
      mCurrent = mInitial;
      SendCounterStates(mgr);
    }
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptAdvancedCounter::SendCounterStates(CStateManager& mgr) {
  if (mCurrent == 0) {
    SendScriptMsgs(kSS_Zero, mgr, kSM_None);
  } else if (mCurrent == mMax) {
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
  }

  for (int i = 0; i < mConditions.size(); ++i) {
    if (mCurrent == mConditions[i]) {
      SendScriptMsgs(static_cast< EScriptObjectState >(kSS_InternalState00 + i), mgr, kSM_None);
    }
  }
}

CEntity* LoadAdvancedCounter(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrAdvancedCounter sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrAdvancedCounter.inc"

  rstl::reserved_vector< int, 10 > conditions;
  conditions.push_back(sldrThis.counterCondition1);
  conditions.push_back(sldrThis.counterCondition2);
  conditions.push_back(sldrThis.counterCondition3);
  conditions.push_back(sldrThis.counterCondition4);
  conditions.push_back(sldrThis.counterCondition5);
  conditions.push_back(sldrThis.counterCondition6);
  conditions.push_back(sldrThis.counterCondition7);
  conditions.push_back(sldrThis.counterCondition8);
  conditions.push_back(sldrThis.counterCondition9);
  conditions.push_back(sldrThis.counterCondition10);

  return rs_new CScriptAdvancedCounter(mgr.AllocateUniqueId(), sldrThis.editorProperties.name, info,
                                       sldrThis.initial_Count, sldrThis.max_Count,
                                       sldrThis.autoReset, conditions);
}
