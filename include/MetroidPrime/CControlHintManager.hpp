#ifndef _CCONTROLHINTMANAGER
#define _CCONTROLHINTMANAGER

#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptControlHint.hpp"

class CFinalInput;

// Guessed class and method names, based on the control-hint specialization.
class CControlHintManager : public CHintManager {
public:
  CControlHintManager(int playerIndex, const rstl::string& name);

  // CHintManager
  ~CControlHintManager() override;
  void Reset(CStateManager& mgr) override;
  bool SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged, bool force) override;
  void ClearHint(CStateManager& mgr, bool areaChanged) override;
  bool SelectHintFromStack(CHintState* hint, CStateManager& mgr, bool areaChanged) override;

  bool HasDisableFlags(uint flags, const CStateManager& mgr) const;
  TUniqueId CreateHint(CStateManager& mgr, const rstl::string& name, int priority, float timer,
                       uint disableFlags, const CScriptControlHint::TCommandStates& commandStates,
                       TUniqueId sender, CGameHint::EBreakHintType breakType, uint requiredPresses,
                       float unknown16c, CGameHint::SCallback onExpire,
                       CGameHint::SCallback onBreak, float breakDelay, int acrossAreas);
  void ProcessInput(const CFinalInput& input, const CControlMapper& mapper, CStateManager& mgr);

private:
  bool ApplyHint(const CHintState& hint, CStateManager& mgr);
};
CHECK_SIZEOF(CControlHintManager, 0x44)

#endif // _CCONTROLHINTMANAGER
