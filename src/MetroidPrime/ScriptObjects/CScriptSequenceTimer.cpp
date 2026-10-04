#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"

#include "rstl/math.hpp"

#include <math.h>

// Guessed name.
rstl::pair< float, float > FindMinMaxConnectionTimes(const rstl::vector< SLdrConnection >&);

CScriptSequenceTimer::CScriptSequenceTimer(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info,
                                           const rstl::vector< SLdrConnection >& connections,
                                           float startTime, float maxTime, float loopStartTime,
                                           bool autoStart, bool loop, bool takeExternalTime)
: CEntity(uid, info, name, 0)
, mStartTime(startTime)
, mCurrentTime(startTime)
, mMaxTime(maxTime != 0.f ? maxTime : FindMinMaxConnectionTimes(connections).second)
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
  if (mLoop && mMaxTime <= mCurrentTime) {
    wrapped = true;
    mCurrentTime = mLoopStartTime + float(fmod(mCurrentTime, mMaxTime));
  }

  const float lowerTime = rstl::min_val(mCurrentTime, oldTime);
  const float upperTime = rstl::max_val(mCurrentTime, oldTime);
  for (rstl::vector< SLdrConnection >::iterator connection = mConnections.begin();
       connection != mConnections.end(); ++connection) {
    const uint connectionIndex = static_cast< ushort >(connection->connectionIndex);
    if (connection->unknown_0x00000002 && gpMain->IsMaxSpeed()) {
      continue;
    }

    for (rstl::vector< float >::iterator activation = connection->activationTimes.begin();
         activation != connection->activationTimes.end(); ++activation) {
      const float activationTime = *activation;
      const bool crossed = wrapped ? upperTime <= activationTime || activationTime < lowerTime
                                   : lowerTime <= activationTime && activationTime < upperTime;
      if (!crossed) {
        continue;
      }

      const SConnection& target = GetConnectionList()[connectionIndex];
      const CStateManager::TIdListResult ids = mgr.GetIdListForScript(target.objId);
      for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
        mgr.SendScriptMsg(CScriptMsg(GetUniqueId(), mStartMessage.GetOriginator(), id->second,
                                     target.msg, target.state));
      }
    }
  }

  if (!mLoop && mMaxTime <= mCurrentTime) {
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

void CScriptSequenceTimer::SetCurrentTime(float time) {
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
