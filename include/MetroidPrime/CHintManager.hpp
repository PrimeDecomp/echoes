#ifndef _CHINTMANAGER
#define _CHINTMANAGER

#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CStateManager;

// Supporting declaration only: the manager's remaining interface is not yet reconstructed.
class CHintManager {
public:
  virtual ~CHintManager();

  // Guessed names. These queue changes for the manager's next update.
  void AddHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr);
  void RemoveHint(TUniqueId hint, TUniqueId sender, CStateManager& mgr);
  void ForceRemoveHint(TUniqueId hint, CStateManager& mgr, TUniqueId sender);

private:
  struct SHint; // Guessed name: priority followed by a runtime CGameHint, stride 0x70.
  typedef rstl::pair< TUniqueId, TUniqueId > THintSender;

  int mPlayerIndex;
  TAreaId mAreaId;
  TUniqueId mCurrentHintId;
  int mPriority;
  rstl::vector< SHint > mHints;
  rstl::vector< THintSender > mRemovedHints;
  rstl::vector< THintSender > mAddedHints;
};
CHECK_SIZEOF(CHintManager, 0x44)

#endif // _CHINTMANAGER
