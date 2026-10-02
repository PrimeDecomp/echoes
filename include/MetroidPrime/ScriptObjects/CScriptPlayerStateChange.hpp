#ifndef _CSCRIPTPLAYERSTATECHANGE
#define _CSCRIPTPLAYERSTATECHANGE

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

// Guessed name, correlated with Prime's inventory-changing script object.
class CScriptPlayerStateChange : public CEntity {
public:
  CScriptPlayerStateChange(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           CPlayerState::EItemType itemType, int amount, int capacityIncrease,
                           int command, int commandAction);

  // CEntity
  ~CScriptPlayerStateChange() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  CPlayerState::EItemType mItemType;
  int mAmount;
  int mCapacityIncrease;
  // Retained loader fields; command filtering is absent from the Echoes handler.
  int mCommand;
  int mCommandAction;
};
CHECK_SIZEOF(CScriptPlayerStateChange, 0x38)

#endif // _CSCRIPTPLAYERSTATECHANGE
