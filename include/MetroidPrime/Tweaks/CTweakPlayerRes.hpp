#ifndef _CTWEAKPLAYERRES
#define _CTWEAKPLAYERRES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakPlayerRes;

class CTweakPlayerRes {
public:
  explicit CTweakPlayerRes(const SLdrTweakPlayerRes& data) : mData(&data) { CacheResources(); }
  ~CTweakPlayerRes() {}

  CAssetId GetSaveStationIcon() const { return mSaveStationIcon; }
  CAssetId GetBallTransitionANCSId() const { return mBallTransitionsANCS; }
  CAssetId GetMissileStationIcon() const { return mMissileStationIcon; }
  CAssetId GetElevatorIcon() const { return mElevatorIcon; }
  CAssetId GetPortalIcon() const { return mPortalIcon; }
  CAssetId GetTranslatorDoorIcon() const { return mTranslatorDoorIcon; }
  CAssetId GetMinesFirstBreakTopIcon() const { return mMinesFirstBreakTopIcon; } // Guessed name
  CAssetId GetMinesFirstBreakBottomIcon() const {
    return mMinesFirstBreakBottomIcon;
  } // Guessed name

  CAssetId GetBallTransitionBeamResId(CPlayerState::EBeamId beam) const;
  // Guessed name
  CAssetId GetBallTransitionBeamResIdMultiplayer(CPlayerState::EBeamId beam) const;
  CAssetId GetCinematicBeamResId(CPlayerState::EBeamId beam) const;
  CAssetId GetCinematicGrappleResId() const;
  const CMayaSpline& GetMorphAlphaSpline() const { return mMorphAlphaSpline; }
  const CMayaSpline& GetUnmorphAlphaSpline() const { return mUnmorphAlphaSpline; }
  const CMayaSpline& GetMultiplayerMorphAlphaSpline() const { return mMultiplayerMorphAlphaSpline; }
  const CMayaSpline& GetMultiplayerUnmorphAlphaSpline() const {
    return mMultiplayerUnmorphAlphaSpline;
  }

private:
  void CacheResources();

  CAssetId mSaveStationIcon;
  CAssetId mMissileStationIcon;
  CAssetId mElevatorIcon;
  CAssetId mPortalIcon;
  CAssetId mTranslatorDoorIcon;
  CAssetId mMinesFirstBreakTopIcon;
  CAssetId mMinesFirstBreakBottomIcon;
  CAssetId mMinesSecondBreakTopIcon;
  CAssetId mMinesSecondBreakBottomIcon;

public:
  rstl::reserved_vector< CAssetId, 9 > mLStick;
  rstl::reserved_vector< CAssetId, 9 > mCStick;
  rstl::reserved_vector< CAssetId, 2 > mLTrigger;
  rstl::reserved_vector< CAssetId, 2 > mRTrigger;
  rstl::reserved_vector< CAssetId, 2 > mStartButton;
  rstl::reserved_vector< CAssetId, 2 > mAButton;
  rstl::reserved_vector< CAssetId, 2 > mBButton;
  rstl::reserved_vector< CAssetId, 2 > mXButton;
  rstl::reserved_vector< CAssetId, 2 > mYButton;

private:
  CAssetId mBallTransitionsANCS;
  CAssetId mBallTransitions[4];
  CAssetId mMultiplayerBallTransitions[4]; // Guessed name
  CAssetId mCineGun[4];
  CAssetId mCinematicGrapple;
  float mCinematicMoveOutofIntoPlayerDistance;
  // Guessed names
  CMayaSpline mMorphAlphaSpline;
  CMayaSpline mUnmorphAlphaSpline;
  CMayaSpline mMultiplayerMorphAlphaSpline;
  CMayaSpline mMultiplayerUnmorphAlphaSpline;
  CMayaSpline mMovementControlSpline;
  const SLdrTweakPlayerRes* mData;
};
CHECK_SIZEOF(CTweakPlayerRes, 0x25c)

extern rstl::single_ptr< CTweakPlayerRes > gpTweakPlayerRes;

#endif // _CTWEAKPLAYERRES
