#ifndef _CINGAMEGUIMANAGERSET
#define _CINGAMEGUIMANAGERSET

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CDependencyGroup;
class CArchitectureQueue;
class CStateManager;
class CGuiFrameLoader;
class CInGameGuiManager;
class CMultiplayerGui;
class CFinalInput;

// Guessed names; the native aggregate owns the common loaders and per-player GUI managers.
class CInGameGuiManagerSet {
public:
  enum ELoadPhase { kLP_LoadDepsGroup, kLP_PreloadDeps, kLP_LoadPlayerGui, kLP_Done };

  CInGameGuiManagerSet(const CStateManager& mgr, CArchitectureQueue& queue);
  ~CInGameGuiManagerSet();

  void StopSounds();
  bool GetIsGameDraw() const;
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input, float dt);
  void DrawMultiplayerGui() const;
  void UpdateMultiplayerGui(float dt, const CStateManager& mgr);
  void Update(const CStateManager& mgr, float dt, CArchitectureQueue& queue, bool cameraActive,
              int playerIndex);
  void PauseGame(const CStateManager& mgr, EInGameGuiState state);
  void PreDraw(CStateManager& mgr, bool cameraActive);
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);
  bool IsInPausedState() const;
  void Draw(const CStateManager& mgr, int playerIndex) const;
  void StartFadeIn();
  bool CheckLoadComplete(const CStateManager& mgr);

  const CInGameGuiManager& GetPlayerGuiManager(int playerIndex) const {
    return *mPlayerGuiManagers[playerIndex];
  }

private:
  bool CheckPlayerGuiLoadComplete(const CStateManager& mgr);
  TToken< CDependencyGroup > mPreloadDGRP;
  rstl::vector< CToken > mPreloadTokens;
  ELoadPhase mLoadPhase;
  CRandom16 mRandom;
  CCameraFilterPass mFadeFilter;
  rstl::single_ptr< CGuiFrameLoader > mHudFrameLoader;
  rstl::single_ptr< CGuiFrameLoader > mHelmetFrameLoader;
  rstl::single_ptr< CGuiFrameLoader > mDarkMaskFrameLoader;
  rstl::single_ptr< CGuiFrameLoader > mMemoFrameLoader;
  rstl::reserved_vector< rstl::auto_ptr< CInGameGuiManager >, 4 > mPlayerGuiManagers;
  rstl::single_ptr< CMultiplayerGui > mMultiplayerGui;
};
CHECK_SIZEOF(CInGameGuiManagerSet, 0x84)

#endif // _CINGAMEGUIMANAGERSET
