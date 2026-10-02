#ifndef _CSCRIPTTIMER
#define _CSCRIPTTIMER

#include "MetroidPrime/CEntity.hpp"

class CScriptTimer : public CEntity {
public:
  CScriptTimer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, float startTime,
               float maxRandDelay, bool loop, bool autoStart);

  // CEntity
  ~CScriptTimer() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void Reset(CStateManager& mgr);
  void ApplyTime(float dt, CStateManager& mgr);

  bool IsTiming() const { return mIsTiming; }
  void StartTiming(bool isTiming) { mIsTiming = isTiming; }

private:
  uint mStartFrame;
  float mTime;
  float mStartTime;
  float mMaxRandDelay;
  bool mLoop : 1;
  bool mAutoStart : 1;
  bool mIsTiming : 1;
  CScriptMsg mScriptMsg; // Message that started the timer; its originator receives kSS_Zero.
};
CHECK_SIZEOF(CScriptTimer, 0x48)

#endif // _CSCRIPTTIMER
