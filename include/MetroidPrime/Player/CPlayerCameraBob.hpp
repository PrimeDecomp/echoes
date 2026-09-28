#ifndef _CPLAYERCAMERABOB
#define _CPLAYERCAMERABOB

#include "types.h"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

class CStateManager;
class CPlayer;

class CPlayerCameraBob {
public:
  // Guessed names; values and roles follow the G2ME01 motion paths.
  enum ECameraBobType {
    kCBT_Zero,
    kCBT_One,
  };
  enum ECameraBobState {
    kCBS_Walk,
    kCBS_Orbit,
    kCBS_InAir,
    kCBS_WalkNoBob,
    kCBS_GunFireNoBob,
    kCBS_TurningNoBob,
    kCBS_FreeLookNoBob,
    kCBS_GrappleNoBob,
    kCBS_Unspecified,
  };

  static float mCameraBobExtentX;
  static float mCameraBobExtentY;
  static float mCameraBobPeriod;
  static float mOrbitBobScale;
  static float mMaxOrbitBobScale;
  static float mSlowSpeedPeriodScale;
  static float mTargetMagnitudeTrackingRate;
  static float mLandingBobSpringConstant;
  static float mPeakNegativeVerticalSpeedForHeavyLanding;
  static float mMaxNegativeVerticalSpeedConsidered;
  static float mHeavyLandingBobSpringConstant;
  static float mHeavyLandingHelmetBobSpringConstant;
  static float mViewWanderRadius;
  static float mViewWanderSpeedMin;
  static float mViewWanderSpeedMax;
  static float mViewWanderRollVariation;
  static float mGunBobMagnitude;
  static float mHelmetBobMagnitude;
  static float mHeavyLandingViewDip;
  static float mLandingBobDamping;
  static float mHeavyLandingBobDamping;
  static float mHeavyLandingHelmetBobDamping;

  CPlayerCameraBob(ECameraBobType type,
                   const CVector2f& extent = CVector2f(mCameraBobExtentX, mCameraBobExtentY),
                   float bobPeriod = mCameraBobPeriod);

  CVector3f GetCameraBobTranslation() const { return mCameraBobTransform.GetTranslation(); }
  const CTransform4f& GetViewWanderTransform() const;
  CVector3f GetHelmetBobTranslation() const;
  CTransform4f GetGunBobTransformation() const;
  CTransform4f GetCameraBobTransformation() const;
  void SetPlayerVelocity(const CVector3f& velocity);
  void SetBobMagnitude(float magnitude);
  void SetBobTimeScale(float scale);
  void ResetCameraBobTime();
  void SetCameraBobTransform(const CTransform4f& xf) { mCameraBobTransform = xf; }
  void SetState(ECameraBobState state, CStateManager& mgr);
  void InitViewWander(CStateManager& mgr);
  void UpdateViewWander(float dt, CStateManager& mgr);
  void Update(float dt, CStateManager& mgr, const CPlayer& player);
  CVector3f CalculateRandomViewWanderPosition(CStateManager& mgr);
  float CalculateRandomViewWanderPitch(CStateManager& mgr);
  void CalculateMovingTranslation(float& x, float& z) const;
  float CalculateLandingTranslation() const;
  CTransform4f CalculateCameraBobTransformation() const;

  const float& GetViewWanderMagnitude() const { return mWanderMagnitude; }

  static float GetCameraBobExtentX() { return mCameraBobExtentX; }
  static float GetCameraBobExtentY() { return mCameraBobExtentY; }
  static float GetCameraBobPeriod() { return mCameraBobPeriod; }
  static float GetOrbitBobScale() { return mOrbitBobScale; }
  static float GetMaxOrbitBobScale() { return mMaxOrbitBobScale; }
  static float GetSlowSpeedPeriodScale() { return mSlowSpeedPeriodScale; }
  static float GetMaxNegativeVerticalSpeedConsidered() {
    return mMaxNegativeVerticalSpeedConsidered;
  }

private:
  ECameraBobType mType;
  CVector2f mBobExtent;
  float mBobPeriod;
  float mTargetBobMagnitude;
  float mBobMagnitude;
  float mBobTimeScale;
  float mBobTime;
  ECameraBobState mOldState;
  ECameraBobState mCurState;
  bool mApplyLandingTrans;
  bool mHardLand;
  CTransform4f mCameraBobTransform;
  CVector3f mPlayerVelocity;
  float mPlayerPeakFallVel;
  float mLandingVelocity;
  float mLandingTranslation;
  float mCamVelocity;
  float mCamTranslation;
  rstl::reserved_vector< CVector3f, 4 > mWanderPoints;
  rstl::reserved_vector< float, 4 > mWanderPitches;
  float mWanderTime;
  float mViewWanderSpeed;
  int mWanderIndex;
  CTransform4f mViewWanderXf;
  float mWanderMagnitude;
  float mTargetWanderMagnitude;
};
CHECK_SIZEOF(CPlayerCameraBob, 0x108)

#endif // _CPLAYERCAMERABOB
