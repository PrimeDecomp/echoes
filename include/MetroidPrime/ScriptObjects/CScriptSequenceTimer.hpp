#ifndef _CSCRIPTSEQUENCETIMER
#define _CSCRIPTSEQUENCETIMER

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/ScriptLoader/SLdrSequenceTimer.hpp"

class CScriptSequenceTimer : public CEntity {
public:
  CScriptSequenceTimer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const SLdrSequenceConnections& connections, float startTime, float maxTime,
                       float loopStartTime, bool autoStart, bool loop, bool takeExternalTime);

  // CEntity
  ~CScriptSequenceTimer() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void SetCurrentTime(float time); // Guessed name
  void ReceiveExternalTime(CStateManager& mgr, float time);

private:
  void ApplyTime(float time, CStateManager& mgr); // Guessed name

  // Guessed field names, derived from initialization and schedule playback.
  float mStartTime;
  float mCurrentTime;
  float mMaxTime;
  float mLoopStartTime;
  bool mRunning : 1;
  bool mLoop : 1;
  bool mTakeExternalTime : 1;
  SLdrSequenceConnections mConnections;
  CScriptMsg mStartMessage;
};
CHECK_SIZEOF(CScriptSequenceTimer, 0x58)

#endif // _CSCRIPTSEQUENCETIMER
