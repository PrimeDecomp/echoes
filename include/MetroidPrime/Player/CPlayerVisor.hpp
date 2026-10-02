#ifndef _CPLAYERVISOR
#define _CPLAYERVISOR

#include "types.h"

#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"

class CModel;
class CStateManager;
class CTargetingManager;

class CPlayerVisor {
  struct SScanObjectIndicatorInfo {
    SScanObjectIndicatorInfo(TUniqueId id, float timer, float inRangeTimer)
    : mObjId(id), mTimer(timer), mInRangeTimer(inRangeTimer), mInBox(false) {}

    TUniqueId mObjId;
    float mTimer;
    float mInRangeTimer;
    bool mInBox;
  };

  enum EScanWindowState {
    kSWS_NotInScanVisor,
    kSWS_Idle,
    kSWS_Scan,
  };

public:
  CPlayerVisor(const CStateManager& mgr, int playerIndex);
  ~CPlayerVisor();

  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, const CTargetingManager* tgtMgr) const;
  float GetDesiredViewportScaleX(const CStateManager& mgr) const;
  float GetDesiredViewportScaleY(const CStateManager& mgr) const;
  CVector2i GetScanWindowViewportSize(const CStateManager& mgr) const; // Guessed name

private:
  void BeginTransitionOut(const CStateManager& mgr);
  void FinishTransitionOut(const CStateManager& mgr);
  void BeginTransitionIn(const CStateManager& mgr);
  void FinishTransitionIn(const CStateManager& mgr);
  void UpdateCurrentVisor(float transFactor);
  void DrawDarkEffect(const CStateManager& mgr) const;
  void DrawEchoEffect(const CStateManager& mgr) const;
  void DrawScanEffect(const CStateManager& mgr, const CTargetingManager* tgtMgr) const;
  void LockUnlockAssets();
  EScanWindowState GetDesiredScanWindowState(const CStateManager& mgr) const;
  void UpdateScanWindow(float dt, const CStateManager& mgr);

  uint mPlayerIndex;
  rstl::reserved_vector< CVector2f, 3 > mScanWindowSizes;
  CPlayerState::EPlayerVisor mCurVisor;
  CPlayerState::EPlayerVisor mNextVisor;
  uchar mVisorSfxVol;
  bool mVisorTransitioning : 1;
  bool x29_25_ : 1;
  float mScanTimer;
  float mScanDimInterp;
  EScanWindowState mPrevState;
  EScanWindowState mNextState;
  float mWindowInterpDuration;
  float mWindowInterpTimer;
  CVector2f mPrevWindowDims;
  CVector2f mInterpWindowDims;
  CVector2f mNextWindowDims;
  float mScanMagInterp;
  CSfxHandle mVisorLoopSfx;
  CSfxHandle mScanningLoopSfx;
  CCameraFilterPass mScanDim;
  CCameraBlurPass mBlur;
  float mVpScaleX;
  float mVpScaleY;
  TCachedToken< CModel > mScanFrameFixedCorner;
  TCachedToken< CModel > mScanFrameCenterLeft;
  TCachedToken< CModel > mScanFrameCenterTop;
  TCachedToken< CModel > mScanFrameBottomLeftCorner;
  TCachedToken< CModel > mScanFrameStretchCorner;
  TCachedToken< CModel > mScanFrameLowerRight;
  TCachedToken< CModel > mScanFrameWindow;
  int mAssetLockCountdown;
  rstl::reserved_vector< SScanObjectIndicatorInfo, 64 > mScanTargets;
  float mScanFrameColorInterp;
  float mScanFrameColorImpulseInterp;
};
CHECK_SIZEOF(CPlayerVisor, 0x520)

#endif // _CPLAYERVISOR
