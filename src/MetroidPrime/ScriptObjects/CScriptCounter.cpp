#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCounter.hpp"

CEntity* LoadCounter(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCounter sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCounter.inc"

  return rs_new CScriptCounter(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                               LdrToEntityInfo(info, sldrThis.editorProperties),
                               sldrThis.initial_Count, sldrThis.max_Count, sldrThis.autoReset,
                               sldrThis.wrap);
}

void CScriptCounter::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();

  if (GetActive()) {
    switch (message) {
    case kSM_SetToZero:
      mCurrent = 0;
      SendScriptMsgs(kSS_Zero, mgr);
      if (mAutoReset) {
        mCurrent = mInitial;
      }
      break;
    case kSM_SetToMax:
      mCurrent = mMax;
      SendScriptMsgs(kSS_MaxReached, mgr);
      if (mAutoReset) {
        mCurrent = mInitial;
      }
      break;
    case kSM_Decrement:
      if (mCurrent == 0 && !mWrap) {
        return;
      }
      if (mCurrent == 0) {
        SendScriptMsgs(kSS_NonZero, mgr);
      }
      --mCurrent;
      if (mCurrent == -mMax) {
        mCurrent = 0;
      }
      if (mCurrent == 0) {
        SendScriptMsgs(kSS_Zero, mgr);
        if (mAutoReset) {
          mCurrent = mInitial;
        }
      }
      break;
    case kSM_Increment:
      if (mCurrent == mMax && !mWrap) {
        return;
      }
      if (mCurrent == 0) {
        SendScriptMsgs(kSS_NonZero, mgr);
      }
      ++mCurrent;
      if (mWrap && mCurrent == mMax) {
        mCurrent = 0;
      }
      if (mCurrent == 0) {
        SendScriptMsgs(kSS_Zero, mgr);
      }
      if (mCurrent == mMax) {
        SendScriptMsgs(kSS_MaxReached, mgr);
        if (mAutoReset) {
          mCurrent = mInitial;
        }
      }
      break;
    case kSM_Reset:
      mCurrent = mInitial;
      break;
    default:
      break;
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

CScriptCounter::~CScriptCounter() {}

CScriptCounter::CScriptCounter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               int initial, int max, const bool autoReset, const bool wrap)
: CEntity(uid, info, name, 0)
, mInitial(initial)
, mCurrent(initial)
, mMax(max)
, mAutoReset(autoReset)
, mWrap(wrap) {}
