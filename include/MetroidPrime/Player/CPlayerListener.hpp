#ifndef _CPLAYERLISTENER
#define _CPLAYERLISTENER

#include "types.h"

class CStateManager;

// Class and nonvirtual method names from MP2 Wii exports, corroborated by GC callers.
class CPlayerListener {
public:
  static const uint kInvalidPlayer = uint(-1);

  CPlayerListener(CStateManager& mgr, uint playerMask);
  virtual ~CPlayerListener() = 0;
  // Reconstructed callback name; native virtual slot and parameter ABI are established.
  virtual void OnGameEvent(CStateManager& mgr, uint sourceIndex, uint targetIndex, uint event,
                           const void* value) = 0;

  uint GetPlayerMask() const { return mPlayerMask; } // Guessed name.
  void KillListener(CStateManager& mgr);
  uint GetFirstPlayer(const CStateManager& mgr) const;
  uint GetNextPlayer(const CStateManager& mgr, uint playerIndex) const;

private:
  // Reconstructed member name; stores only registered player bits.
  uint mPlayerMask;
};
CHECK_SIZEOF(CPlayerListener, 8)

#endif // _CPLAYERLISTENER
