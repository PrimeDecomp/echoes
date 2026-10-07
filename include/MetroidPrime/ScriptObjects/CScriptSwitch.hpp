#ifndef _CSCRIPTSWITCH
#define _CSCRIPTSWITCH

#include "MetroidPrime/CEntity.hpp"

class CScriptSwitch : public CEntity {
public:
  CScriptSwitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool opened,
                bool closeOnOpened);

  // CEntity
  ~CScriptSwitch() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  bool IsOpened() const { return mOpened; } // Guessed name.

private:
  bool mOpened;
  bool mCloseOnOpened;
};
CHECK_SIZEOF(CScriptSwitch, 0x28)

#endif // _CSCRIPTSWITCH
