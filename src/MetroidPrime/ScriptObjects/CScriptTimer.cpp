#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTimer.hpp"

#include <float.h>

CEntity* LoadTimer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTimer sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTimer.inc"

  return rs_new CScriptTimer(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                             LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.time,
                             sldrThis.randomAdjust, sldrThis.autoReset, sldrThis.autoStart);
}

void CScriptTimer::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    bool should = false;
    if (IsTiming() && GetActive()) {
      should = true;
    }
    if (should) {
      ApplyTime(dt, mgr);
    }
  }
}

void CScriptTimer::ApplyTime(float dt, CStateManager& mgr) {
  if (mTime > 0.f && GetActive()) {
    if (mStartFrame == mgr.GetUpdateFrameIdx()) {
      return;
    }
    mTime -= dt;
    if (mTime <= 0.f) {
      SendScriptMsgs(kSS_Zero, mgr, mScriptMsg.GetOriginator(), kSM_None);

      mIsTiming = false;
      if (!mLoop) {
        return;
      }

      Reset(mgr);
      if (!mAutoStart) {
        return;
      }

      mIsTiming = true;
    }
  }
}

void CScriptTimer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();

  if (GetActive()) {
    switch (message) {
    case kSM_Activate:
      if (mIsTiming) {
        mScriptMsg = msg;
      }
      break;
    case kSM_Start:
      mScriptMsg = msg;
      StartTiming(true);
      mStartFrame = mgr.GetUpdateFrameIdx();
      break;
    case kSM_Stop:
      StartTiming(false);
      break;
    case kSM_Reset:
      Reset(mgr);
      if (mAutoStart) {
        mScriptMsg = msg;
        StartTiming(true);
        mStartFrame = mgr.GetUpdateFrameIdx();
      }
      break;
    case kSM_StopAndReset:
      Reset(mgr);
      StartTiming(false);
      break;
    case kSM_ResetAndStart:
      Reset(mgr);
      mScriptMsg = msg;
      StartTiming(true);
      mStartFrame = mgr.GetUpdateFrameIdx();
      break;
    case kSM_SetToZero:
      if (mIsTiming) {
        mTime = FLT_EPSILON;
      }
      break;
    default:
      break;
    }
  }

  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptTimer::Reset(CStateManager& mgr) {
  const float rDt = mgr.Random()->Float();
  mTime = mMaxRandDelay * rDt + mStartTime;
}

CScriptTimer::~CScriptTimer() {}

CScriptTimer::CScriptTimer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           float startTime, float maxRandDelay, bool loop, bool autoStart)
: CEntity(uid, info, name, 0)
, mStartFrame(0)
, mTime(startTime)
, mStartTime(startTime)
, mMaxRandDelay(maxRandDelay)
, mLoop(loop)
, mAutoStart(autoStart)
, mIsTiming(autoStart) {}
