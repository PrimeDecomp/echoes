#ifndef _CPLAYERHINTMANAGER
#define _CPLAYERHINTMANAGER

#include "MetroidPrime/CHintManager.hpp"

// Guessed name: the player-settings specialization of the shared hint manager.
class CPlayerHintManager : public CHintManager {
public:
  CPlayerHintManager(int playerIndex, const rstl::string& name);

  // CHintManager
  ~CPlayerHintManager() override;
  bool SetHint(CHintState* hint, CStateManager& mgr, bool areaChanged, bool force) override;
  void ClearHint(CStateManager& mgr, bool areaChanged) override;
  bool SelectHintFromStack(CHintState* hint, CStateManager& mgr, bool areaChanged) override;

private:
  bool ApplyHint(const CHintState& hint, CStateManager& mgr);
};
CHECK_SIZEOF(CPlayerHintManager, 0x44)

#endif // _CPLAYERHINTMANAGER
