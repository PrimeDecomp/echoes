#ifndef _CSCRIPTCONDITIONALRELAY
#define _CSCRIPTCONDITIONALRELAY

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed type, enum and method names, based on the CRLY loader and native queries.
class CConditionalRelayQuery {
public:
  enum EBoolean { kB_Disabled, kB_And, kB_Or };
  enum EField { kF_Amount, kF_Capacity };
  enum EComparison {
    kC_Equal,
    kC_NotEqual,
    kC_Greater,
    kC_Less,
    kC_GreaterOrEqual,
    kC_LessOrEqual,
    kC_GreaterThanAllPlayers,
    kC_LessThanAllPlayers
  };

  CConditionalRelayQuery(EBoolean boolean, CPlayerState::EItemType item, EField field,
                          EComparison comparison, int value);
  bool IsConditionSatisfied(CStateManager& mgr, uint playerIndex) const;
  EBoolean GetBoolean() const { return mBoolean; }

private:
  EBoolean mBoolean;
  CPlayerState::EItemType mItem;
  EField mField;
  EComparison mComparison;
  int mValue;
};
CHECK_SIZEOF(CConditionalRelayQuery, 0x14)

// Guessed class and private method names; the selected native allocation is 0x80.
class CScriptConditionalRelay : public CEntity {
public:
  CScriptConditionalRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                          uint playerMask,
                          const rstl::reserved_vector< CConditionalRelayQuery, 4 >& conditions,
                          bool setToZeroOnAreaLoaded);

  // CEntity
  ~CScriptConditionalRelay() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  void OnSetToZero(CStateManager& mgr, TUniqueId originator);
  bool VerifyConditions(CStateManager& mgr, TUniqueId originator) const;

  uint mPlayerMask;
  rstl::reserved_vector< CConditionalRelayQuery, 4 > mConditions;
  bool mSetToZeroOnAreaLoaded;
};
CHECK_SIZEOF(CScriptConditionalRelay, 0x80)

#endif // _CSCRIPTCONDITIONALRELAY
