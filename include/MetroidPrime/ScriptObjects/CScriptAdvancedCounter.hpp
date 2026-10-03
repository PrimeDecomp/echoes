#ifndef _CSCRIPTADVANCEDCOUNTER
#define _CSCRIPTADVANCEDCOUNTER

#include "MetroidPrime/CEntity.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed name: the ACNT script object with ten independently configured conditions.
class CScriptAdvancedCounter : public CEntity {
public:
  CScriptAdvancedCounter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         int initial, int max, bool autoReset,
                         const rstl::reserved_vector< int, 10 >& conditions);

  // CEntity
  ~CScriptAdvancedCounter() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  // Guessed name.
  void SendCounterStates(CStateManager& mgr);

  int mInitial;
  int mCurrent;
  int mMax;
  rstl::reserved_vector< int, 10 > mConditions;
  bool mAutoReset : 1;
};
CHECK_SIZEOF(CScriptAdvancedCounter, 0x60)

#endif // _CSCRIPTADVANCEDCOUNTER
