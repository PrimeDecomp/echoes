#ifndef _CINGAMEGUIMANAGERSET
#define _CINGAMEGUIMANAGERSET

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"
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

// Guessed names; the native aggregate owns the common loaders and per-player GUI managers.
class CInGameGuiManagerSet {
public:
  enum ELoadPhase { kLP_LoadDepsGroup, kLP_PreloadDeps, kLP_LoadPlayerGui, kLP_Done };

  CInGameGuiManagerSet(const CStateManager& mgr, CArchitectureQueue& queue);
  ~CInGameGuiManagerSet();

  const CInGameGuiManager& GetPlayerGuiManager(int playerIndex) const {
    return *mPlayerGuiManagers[playerIndex];
  }

private:
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
