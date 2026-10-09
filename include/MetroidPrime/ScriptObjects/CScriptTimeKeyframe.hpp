#ifndef _CSCRIPTTIMEKEYFRAME
#define _CSCRIPTTIMEKEYFRAME

#include "MetroidPrime/CEntity.hpp"

// Class name from the Corruption prototype's (G2MEAB) CScriptTimeKeyframe.cpp asserts.
class CScriptTimeKeyframe : public CEntity {
public:
  CScriptTimeKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, float time);

  // CEntity
  ~CScriptTimeKeyframe() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void SetTime(float time, CStateManager& mgr); // Guessed name

private:
  void ApplyTime(TUniqueId id, CStateManager& mgr); // Guessed name

  float mTime; // Guessed name
};
CHECK_SIZEOF(CScriptTimeKeyframe, 0x28)

#endif
