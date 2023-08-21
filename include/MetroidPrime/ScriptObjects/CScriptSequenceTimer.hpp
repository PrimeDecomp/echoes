#ifndef _CSCRIPTSEQUENCETIMER
#define _CSCRIPTSEQUENCETIMER

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/ScriptLoader/SLdrConnection.hpp"

class CScriptSequenceTimer : public CEntity {
private:

public:
  CScriptSequenceTimer(TUniqueId, const rstl::string&, const CEntityInfo&, const rstl::vector< SLdrConnection >& connections, float startTime, float maxTime, float loopStartTime, bool isAutostart, bool isLoop, bool takeExternalTime);
  ~CScriptSequenceTimer();
  CEntity* TypesMatch(EEntityType) override;

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, CScriptMsg&) override;

  void fn_801e1c1c(float f, CStateManager& mgr);

  void SetCurrentTime(float);
  void fn_801e1af8(float f, CStateManager& mgr);

private:
  float m_startTime;  // 0x24
  float m_currentTime;  // 0x28
  float m_maxTime;  // 0x2c
  float m_loopStartTime;  // 0x30
  bool m_isAutostart : 1;
  bool m_isLoop : 1;
  bool m_takeExternalTime : 1;
  rstl::vector< SLdrConnection > m_connections;
  CScriptMsg m_scriptMsg; // 0x48
};

#endif // _CSCRIPTSEQUENCETIMER
