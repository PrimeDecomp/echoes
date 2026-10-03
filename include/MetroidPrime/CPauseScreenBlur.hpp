#ifndef _CPAUSESCREENBLUR
#define _CPAUSESCREENBLUR

#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"

#include "Kyoto/TToken.hpp"

#include "math.h"

class CInGameGuiManager;
class CStateManager;
class CTexture;

class CPauseScreenBlur {
public:
  enum EState { kS_InGame, kS_MapScreen, kS_SaveGame, kS_HUDMessage, kS_Pause };

  CPauseScreenBlur();
  virtual ~CPauseScreenBlur();

  void OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr,
                           const CInGameGuiManager& guiMgr);
  bool IsGameDraw() const { return mGameDraw; }
  void Update(float dt, const CStateManager& mgr, bool b);
  void Draw(const CStateManager& mgr);
  float GetBlurAmt() const;
  bool IsNotTransitioning() const { return mPrevState == mNextState; }

private:
  TLockedToken< CTexture > mMapLightQuarter;
  EState mPrevState;
  EState mNextState;
  float mBlurAmt;
  CCameraBlurPass mCamBlur;
  bool mBlurring : 1;
  bool mGameDraw : 1;

  void OnBlurComplete(bool b);
  void SetState(EState state, const CInGameGuiManager& guiMgr);
  float GetBlurAmtInline() const { return fabs(mBlurAmt); }
};
CHECK_SIZEOF(CPauseScreenBlur, 0x40)

#endif // _CPAUSESCREENBLUR
