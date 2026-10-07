#ifndef _CMFGAMELOADER
#define _CMFGAMELOADER

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"
#include "rstl/rc_ptr.hpp"

class CStateManager;
class CInGameGuiManagerSet;

class CMFGameLoader : public CIOWin {
public:
  CMFGameLoader();

  // CIOWin
  ~CMFGameLoader() override;
  EMessageReturn OnMessage(const CArchitectureMessage& message, CArchitectureQueue& queue) override;
  void Draw() const override;

private:
  // Guessed names for the gun-pak selection and loading interfaces.
  void UnloadGunPakSet(int set);
  void LoadGunPakSet(int set);
  void SelectGunPakSet();
  void ApplyGunPakSelection(int selected);
  bool IsGunPakSetLoaded(int set) const;
  void ClearGunPakSetLoaded(int set);
  void MarkGunPakSetLoaded(int set);
  void ScanLoadedGunPaks();
  void UpdateGunPaks();

  rstl::ncrc_ptr< CStateManager > mStateManager;
  rstl::ncrc_ptr< CInGameGuiManagerSet > mGuiManager;
  int mLoadedGunPakSets;
  bool mInitialized : 1;
  bool mTransitionFinished : 1;
};
CHECK_SIZEOF(CMFGameLoader, 0x2c)

#endif // _CMFGAMELOADER
