#include "MetroidPrime/ScriptObjects/CScriptSequenceTimer.hpp"

#include "MetroidPrime/ScriptLoader/Struct/SLdrSequenceTimer.hpp"

#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Math/CMath.hpp"

rstl::pair< float, float > FindMinMaxConnectionTimes(const rstl::vector< SLdrConnection >&);

CScriptSequenceTimer::CScriptSequenceTimer(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info,
                                           const rstl::vector< SLdrConnection >& connections,
                                           float startTime, float maxTime, float loopStartTime,
                                           bool isAutostart, bool isLoop, bool takeExternalTime)
: CEntity(uid, info, name, false)

, m_startTime(startTime)
, m_currentTime(startTime)
, m_maxTime(maxTime != 0.0f ? maxTime : FindMinMaxConnectionTimes(connections).second)
, m_loopStartTime(loopStartTime)
, m_isAutostart(isAutostart)
, m_isLoop(isLoop)
, m_takeExternalTime(takeExternalTime)
, m_connections(connections)
, m_scriptMsg() {}

CScriptSequenceTimer::~CScriptSequenceTimer() {}

void CScriptSequenceTimer::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (m_isAutostart && !m_takeExternalTime) {
      fn_801e1c1c(m_currentTime + dt + 0.1f, mgr); // maybe 0.0000099999997f
    }
  }
}

void CScriptSequenceTimer::fn_801e1c1c(float changeTo, CStateManager& mgr) {
  float oldTime = m_currentTime;
  m_currentTime = changeTo;

  bool exceedMaxTime = false;
  if (m_isLoop) {
    if (m_maxTime <= m_currentTime) {
      exceedMaxTime = true;
      float t = fmod(m_currentTime, m_maxTime);
      m_currentTime = m_loopStartTime + t;
    }
  }

  float oldestTime = m_currentTime;
  float latestTime = oldestTime;
  if (oldestTime < oldestTime) {
    latestTime = oldTime;
  }
  if (oldestTime < oldTime) {
    oldestTime = oldTime;
  }

  for (rstl::vector< SLdrConnection >::iterator connection = m_connections.begin();
       connection != m_connections.end(); ++connection) {
    if (!connection->unknown || !gpMain->fn_80008A1C()) {
      for (rstl::vector< float >::iterator activation = connection->activationTimes.begin();
           activation != connection->activationTimes.end(); ++activation) {
        bool bVar6 = false;
        if (!exceedMaxTime) {
          if (latestTime <= *activation && *activation < oldestTime) {
            bVar6 = true;
          }
        } else if (oldestTime <= *activation || *activation < latestTime) {
          bVar6 = true;
        }
        if (bVar6) {
          const SConnection& con = GetConnectionList()[connection->connectionIndex];
          CStateManager::TIdListResult search = mgr.GetIdListForScript(con.objId);
          CStateManager::TIdList::const_iterator current = search.first;
          CStateManager::TIdList::const_iterator end = search.second;
          while (current != end) {
            mgr.SendScriptMsg_fn_80037100(CScriptMsg(GetUniqueId(), m_scriptMsg.GetOriginator(),
                                                     current->second, con.msg, con.state));
            ++current;
          }
        }
      }
    }
  }

  if (!m_isLoop && m_maxTime <= m_currentTime) {
    SendScriptMsgs(kSS_MaxReached, mgr, m_scriptMsg.GetOriginator(), kSM_None);
    m_isAutostart = false;
  }
}

void CScriptSequenceTimer::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Start:
      m_isAutostart = true;
      m_scriptMsg = msg;
      m_currentTime = m_startTime;
      break;

    case kSM_Stop:
      m_isAutostart = false;
      break;

    case kSM_Play:
      m_isAutostart = true;
      break;

    default:
      break;
    }
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

void CScriptSequenceTimer::SetCurrentTime(float f) { m_currentTime = fmod(f, m_maxTime); }

void CScriptSequenceTimer::fn_801e1af8(float f, CStateManager& mgr) {
  fn_801e1c1c(f, mgr);
  return;
}

CScriptSequenceTimer* LoadSequenceTimer(CStateManager& mgr, CInputStream& input,
                                        CEntityInfo& info) {
  SLdrSequenceTimer sldrThis;

  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x255a4580:
      LoadTypedefSLdrEditorProperties(sldrThis.editorProperties, input);
      break;

    case 0xef5c94e9:
      sldrThis.sequenceConnections = SLdrSequenceConnections(input);
      break;

    case 0xb8bd2175:
      sldrThis.startTime = input.ReadFloat();
      break;

    case 0x3e7b2b4:
      sldrThis.maxTime = input.ReadFloat();
      break;

    case 0xacf9ca5f:
      sldrThis.loopStartTime = input.ReadFloat();
      break;

    case 0x42c6e2b2:
      sldrThis.isAutostart = input.ReadBool();
      break;

    case 0xc08d1b93:
      sldrThis.isLoop = input.ReadBool();
      break;

    case 0x27b3b082:
      sldrThis.takeExternalTime = input.ReadBool();
      break;

    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }

  return new CScriptSequenceTimer(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                  LdrToEntityInfo(info, sldrThis.editorProperties),
                                  sldrThis.sequenceConnections.array, sldrThis.startTime,
                                  sldrThis.maxTime, sldrThis.loopStartTime, sldrThis.isAutostart,
                                  sldrThis.isLoop, sldrThis.takeExternalTime);
}
