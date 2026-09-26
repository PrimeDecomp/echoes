#ifndef _CTWEAKPLAYERRES
#define _CTWEAKPLAYERRES

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakPlayerRes;

class CTweakPlayerRes {
public:
  explicit CTweakPlayerRes(const SLdrTweakPlayerRes& data);
  ~CTweakPlayerRes();

  CAssetId GetSaveStationIcon() const { return mSaveStationIcon; }
  CAssetId GetMissileStationIcon() const { return mMissileStationIcon; }
  CAssetId GetElevatorIcon() const { return mElevatorIcon; }
  CAssetId GetPortalIcon() const { return mPortalIcon; }
  CAssetId GetTranslatorDoorIcon() const { return mTranslatorDoorIcon; }
  CAssetId GetDownArrowIcon() const { return mDownArrowIcon; } // Guessed name
  CAssetId GetUpArrowIcon() const { return mUpArrowIcon; }     // Guessed name

private:
  CAssetId mSaveStationIcon;
  CAssetId mMissileStationIcon;
  CAssetId mElevatorIcon;
  CAssetId mPortalIcon;
  CAssetId mTranslatorDoorIcon;
  CAssetId mDownArrowIcon; // Guessed name
  CAssetId mUpArrowIcon;   // Guessed name
  CAssetId x1c_;
  CAssetId x20_;

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
  SLdrSpline mBallTransitionSplines[4]; // Guessed name
  SLdrSpline mMovementControlSpline;    // Guessed name
  const SLdrTweakPlayerRes* mData;
};
CHECK_SIZEOF(CTweakPlayerRes, 0x25c)

extern rstl::single_ptr< CTweakPlayerRes > gpTweakPlayerRes;

#endif // _CTWEAKPLAYERRES
