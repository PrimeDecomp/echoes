#ifndef _CSCRIPTRANDOMRELAY
#define _CSCRIPTRANDOMRELAY

#include "MetroidPrime/CEntity.hpp"

class CScriptRandomRelay : public CEntity {
public:
  CScriptRandomRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     int sendSetSize, int sendSetVariance, bool percentSize, bool randomChance);

  // CEntity
  ~CScriptRandomRelay() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void SendLocalScriptMsgs(EScriptObjectState state, CStateManager& mgr, TUniqueId originator);

private:
  int mSendSetSize;
  int mSendSetVariance;
  bool mPercentSize : 1;
  bool mRandomChance : 1;
};
CHECK_SIZEOF(CScriptRandomRelay, 0x30)

#endif // _CSCRIPTRANDOMRELAY
