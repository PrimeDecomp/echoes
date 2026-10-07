#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"

#include "rstl/math.hpp"

#include <math.h>

CScriptSequenceTimer::CScriptSequenceTimer(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info,
                                           const SLdrSequenceConnections& connections,
                                           float startTime, float maxTime, float loopStartTime,
                                           const bool autoStart, const bool loop,
                                           const bool takeExternalTime)
: CEntity(uid, info, name, 0)
, mStartTime(startTime)
, mCurrentTime(startTime)
, mMaxTime(maxTime != 0.f ? maxTime : FindMinMaxConnectionTimes(connections.mConnections).second)
, mLoopStartTime(loopStartTime)
, mRunning(autoStart)
, mLoop(loop)
, mTakeExternalTime(takeExternalTime)
, mConnections(connections)
, mStartMessage() {}

CScriptSequenceTimer::~CScriptSequenceTimer() {}

void CScriptSequenceTimer::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mRunning && !mTakeExternalTime) {
    ApplyTime(0.00001f + (mCurrentTime + dt), mgr);
  }
}

void CScriptSequenceTimer::ApplyTime(float time, CStateManager& mgr) {
  const float oldTime = mCurrentTime;
  mCurrentTime = time;

  bool wrapped = false;
  if (mLoop && mCurrentTime >= mMaxTime) {
    wrapped = true;
    mCurrentTime = mLoopStartTime + float(fmod(mCurrentTime, mMaxTime));
  }

  const float lowerTime = rstl::min_val(mCurrentTime, oldTime);
  const float upperTime = rstl::max_val(mCurrentTime, oldTime);
  for (rstl::vector< SLdrConnection >::iterator connection = mConnections.mConnections.begin();
       connection != mConnections.mConnections.end(); ++connection) {
    const uint connectionIndex = connection->connectionIndex;
    if (connection->mActivation.second && gpMain->IsMaxSpeed()) {
      continue;
    }

    for (rstl::vector< float >::iterator activation = connection->mActivation.first.begin();
         activation != connection->mActivation.first.end(); ++activation) {
      const float activationTime = *activation;
      bool crossed = false;
      if (!wrapped) {
        if (lowerTime <= activationTime && activationTime < upperTime) {
          crossed = true;
        }
      } else if (upperTime <= activationTime || activationTime < lowerTime) {
        crossed = true;
      }
      if (!crossed) {
        continue;
      }

      const SConnection& target = GetConnectionList()[connectionIndex];
      const CStateManager::TIdListResult ids = mgr.GetIdListForScript(target.objId);
      for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
        mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), id->second, target.msg,
                                     mStartMessage.GetOriginator(), target.state));
      }
    }
  }

  if (!mLoop && mCurrentTime >= mMaxTime) {
    SendScriptMsgs(kSS_MaxReached, mgr, mStartMessage.GetOriginator(), kSM_None);
    mRunning = false;
  }
}

void CScriptSequenceTimer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Start:
      mRunning = true;
      mStartMessage = msg;
      mCurrentTime = mStartTime;
      break;
    case kSM_Stop:
      mRunning = false;
      break;
    case kSM_Play:
      mRunning = true;
      break;
    default:
      break;
    }
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptSequenceTimer::SetCurrentTime(float time, CStateManager&) {
  mCurrentTime = float(fmod(time, mMaxTime));
}

void CScriptSequenceTimer::ReceiveExternalTime(CStateManager& mgr, float time) {
  ApplyTime(time, mgr);
}

CEntity* LoadSequenceTimer(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSequenceTimer sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSequenceTimer.inc"

  return rs_new CScriptSequenceTimer(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                     LdrToEntityInfo(info, sldrThis.editorProperties),
                                     sldrThis.sequenceConnections, sldrThis.startTime,
                                     sldrThis.maxTime, sldrThis.loopStartTime, sldrThis.isAutostart,
                                     sldrThis.isLoop, sldrThis.takeExternalTime);
}
