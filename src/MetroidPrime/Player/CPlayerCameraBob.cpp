#include "MetroidPrime/Player/CPlayerCameraBob.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/math.hpp"

float CPlayerCameraBob::mCameraBobExtentX = 0.071f;
float CPlayerCameraBob::mCameraBobExtentY = 0.142f;
float CPlayerCameraBob::mCameraBobPeriod = 0.47f;
float CPlayerCameraBob::mOrbitBobScale = 0.77f;
float CPlayerCameraBob::mMaxOrbitBobScale = 0.8f;
float CPlayerCameraBob::mSlowSpeedPeriodScale = 0.3f;
float CPlayerCameraBob::mTargetMagnitudeTrackingRate = 0.1f;
float CPlayerCameraBob::mLandingBobSpringConstant = 150.f;
float CPlayerCameraBob::mPeakNegativeVerticalSpeedForHeavyLanding = -30.f;
float CPlayerCameraBob::mMaxNegativeVerticalSpeedConsidered = -35.f;
float CPlayerCameraBob::mHeavyLandingBobSpringConstant = 40.f;
float CPlayerCameraBob::mHeavyLandingHelmetBobSpringConstant = 80.f;
float CPlayerCameraBob::mViewWanderRadius = 0.03f;
float CPlayerCameraBob::mViewWanderSpeedMin = 0.1f;
float CPlayerCameraBob::mViewWanderSpeedMax = 0.3f;
float CPlayerCameraBob::mViewWanderRollVariation = 0.3f;
float CPlayerCameraBob::mGunBobMagnitude = 0.3f;
float CPlayerCameraBob::mHelmetBobMagnitude = 2.f;
float CPlayerCameraBob::mHeavyLandingViewDip = 2.f;
float CPlayerCameraBob::mLandingBobDamping = CMath::SqrtF(mLandingBobSpringConstant) * 2.f;
float CPlayerCameraBob::mHeavyLandingBobDamping =
    CMath::SqrtF(mHeavyLandingBobSpringConstant) * 4.f;
float CPlayerCameraBob::mHeavyLandingHelmetBobDamping =
    CMath::SqrtF(mHeavyLandingHelmetBobSpringConstant) * 6.f;

CPlayerCameraBob::CPlayerCameraBob(ECameraBobType type, const CVector2f& extent, float bobPeriod)
: mType(type)
, mBobExtent(extent)
, mBobPeriod(bobPeriod)
, mTargetBobMagnitude(0.f)
, mBobMagnitude(0.f)
, mBobTimeScale(0.f)
, mBobTime(0.f)
, mOldState(kCBS_Unspecified)
, mCurState(kCBS_Unspecified)
, mApplyLandingTrans(false)
, mHardLand(false)
, mCameraBobTransform(CTransform4f::Identity())
, mPlayerVelocity(CVector3f(0.f, 0.f, 0.f))
, mPlayerPeakFallVel(0.f)
, mLandingVelocity(0.f)
, mLandingTranslation(0.f)
, mCamVelocity(0.f)
, mCamTranslation(0.f)
, mWanderTime(0.f)
, mViewWanderSpeed(mViewWanderSpeedMin)
, mWanderIndex(0)
, mViewWanderXf(CTransform4f::Identity())
, mWanderMagnitude(FLT_EPSILON)
, mTargetWanderMagnitude(0.f) {
  for (int i = 0; i < 4; ++i) {
    mWanderPoints.push_back(CVector3f(0.f, 1.f, 0.f));
  }
  for (int i = 0; i < 4; ++i) {
    mWanderPitches.push_back(0.f);
  }
}

void CPlayerCameraBob::Update(float dt, CStateManager& mgr, const CPlayer& player) {
  mBobTime += dt * mBobTimeScale;
  if (mApplyLandingTrans) {
    float damping = mLandingBobDamping;
    float spring = mLandingBobSpringConstant;
    if (mHardLand) {
      damping = mHeavyLandingBobDamping;
      spring = mHeavyLandingBobSpringConstant;
    }

    mLandingVelocity += dt * (-(damping * mLandingVelocity) - spring * mLandingTranslation);
    mLandingTranslation += mLandingVelocity * dt;
    mCamVelocity += dt * (-(mHeavyLandingHelmetBobDamping * mCamVelocity) -
                          mHeavyLandingHelmetBobSpringConstant * mCamTranslation);
    mCamTranslation += mCamVelocity * dt;
    if (CMath::AbsF(mLandingVelocity) < 0.005f && CMath::AbsF(mLandingTranslation) < 0.005f &&
        CMath::AbsF(mCamVelocity) < 0.005f && CMath::AbsF(mCamTranslation) < 0.005f) {
      mApplyLandingTrans = false;
      mLandingTranslation = 0.f;
      mCamTranslation = 0.f;
    }
  }

  if (mCurState == kCBS_WalkNoBob) {
    mTargetWanderMagnitude = 1.f;
  } else {
    mTargetWanderMagnitude = 0.f;
  }

  float magnitude = player.GetCameraManager()->GetCameraBobMagnitude();
  mLandingTranslation *= magnitude;
  mCamTranslation *= magnitude;
  mTargetWanderMagnitude *= magnitude;
  if (player.GetDoneSidewaysDashing()) {
    mLandingTranslation *= 0.2f;
    mCamTranslation *= 0.2f;
    mTargetWanderMagnitude *= 0.2f;
  }

  mWanderMagnitude += mTargetMagnitudeTrackingRate * (mTargetWanderMagnitude - mWanderMagnitude);
  if (mWanderMagnitude < 0.f) {
    mWanderMagnitude = 0.f;
  }
  mBobMagnitude += mTargetMagnitudeTrackingRate * (mTargetBobMagnitude - mBobMagnitude);
  UpdateViewWander(dt, mgr);
  mCameraBobTransform = CalculateCameraBobTransformation() * GetViewWanderTransform() *
                        CTransform4f::LookAt(CVector3f::Zero(),
                                             CVector3f(0.f, mHeavyLandingViewDip, mCamTranslation));
}

void CPlayerCameraBob::SetBobTimeScale(const float scale) {
  mBobTimeScale = scale;
  mBobTimeScale = rstl::max_val(mBobTimeScale, 0.f);
  mBobTimeScale = rstl::min_val(mBobTimeScale, 1.f);
}

void CPlayerCameraBob::SetBobMagnitude(const float scale) {
  mTargetBobMagnitude = scale;
  mTargetBobMagnitude = rstl::max_val(mTargetBobMagnitude, 0.f);
  mTargetBobMagnitude = rstl::min_val(mTargetBobMagnitude, 1.f);
}

CTransform4f CPlayerCameraBob::CalculateCameraBobTransformation() const {
  float x = 0.f;
  float z = 0.f;
  CalculateMovingTranslation(x, z);
  if (mApplyLandingTrans) {
    z += CalculateLandingTranslation();
  }

  return CTransform4f::Translate(x, 0.f, z);
}

CTransform4f CPlayerCameraBob::GetCameraBobTransformation() const { return mCameraBobTransform; }

CTransform4f CPlayerCameraBob::GetGunBobTransformation() const {
  return CTransform4f(
      CTransform4f::Translate(GetCameraBobTranslation() * (mGunBobMagnitude + 1.f)));
}

CVector3f CPlayerCameraBob::GetHelmetBobTranslation() const {
  return mHelmetBobMagnitude *
         (mCameraBobTransform.GetTranslation() - CVector3f(0.f, 0.f, mCamTranslation));
}

float CPlayerCameraBob::CalculateLandingTranslation() const { return mLandingTranslation; }

void CPlayerCameraBob::CalculateMovingTranslation(float& x, float& z) const {
  switch (mType) {
  case kCBT_Zero: {
    double angle = 2.0 * M_PI * CMath::ModF(mBobTime, 2.f * mBobPeriod) / mBobPeriod;
    x = (mBobMagnitude * mBobExtent[0]) * static_cast< float >(sin(angle));
    z = (mBobMagnitude * mBobExtent[1]) *
        static_cast< float >(cos(angle / 2.0) * fabs(cos(angle / 2.0)));
    break;
  }
  case kCBT_One: {
    float time = CMath::ModF(mBobTime, 2.f * mBobPeriod);
    double angle = (M_PI * time) / mBobPeriod;
    if (time > mBobPeriod) {
      x = (2.f - time / mBobPeriod) * (mBobMagnitude * mBobExtent[0]);
    } else {
      x = time / mBobPeriod * (mBobMagnitude * mBobExtent[0]);
    }
    float sine = static_cast< float >(sin(fmod(angle, M_PI)));
    z = ((1.f - sine) * (mBobMagnitude * mBobExtent[1])) / 2.f +
        0.5f * (-(sine * sine - 1.f) * (mBobMagnitude * mBobExtent[1]));
    break;
  }
  }
}

void CPlayerCameraBob::ResetCameraBobTime() { mBobTime = 0.f; }

void CPlayerCameraBob::SetState(ECameraBobState state, CStateManager& mgr) {
  if (state == mCurState) {
    return;
  }

  mOldState = mCurState;
  mCurState = state;

  if (mOldState == kCBS_InAir) {
    mApplyLandingTrans = true;
    mPlayerPeakFallVel = rstl::max_val(mPlayerPeakFallVel, mMaxNegativeVerticalSpeedConsidered);
    mHardLand = mPlayerPeakFallVel < mPeakNegativeVerticalSpeedForHeavyLanding;
    if (mHardLand) {
      mCamVelocity += mPlayerPeakFallVel;
    }
    mLandingVelocity += mPlayerPeakFallVel;
    mPlayerPeakFallVel = 0.f;
  }

  if (mCurState == kCBS_WalkNoBob && mWanderMagnitude) {
    InitViewWander(mgr);
  }
}

void CPlayerCameraBob::SetPlayerVelocity(const CVector3f& velocity) {
  mPlayerVelocity = velocity;
  mPlayerPeakFallVel = rstl::min_val(velocity[kDZ], mPlayerPeakFallVel);
}

void CPlayerCameraBob::InitViewWander(CStateManager& mgr) {
  mWanderPoints[0] = CVector3f(0.f, 1.f, 0.f);
  mWanderPoints[1] = mWanderPoints[0];
  mWanderPoints[2] = mWanderPoints[0];
  mWanderPoints[3] = CalculateRandomViewWanderPosition(mgr);
  mWanderPitches[0] = 0.f;
  mWanderPitches[1] = mWanderPitches[0];
  mWanderPitches[2] = mWanderPitches[0];
  mWanderPitches[3] = CalculateRandomViewWanderPitch(mgr);
  mViewWanderSpeed =
      (mViewWanderSpeedMax - mViewWanderSpeedMin) * mgr.Random()->Float() + mViewWanderSpeedMin;
  mWanderTime = 0.f;
  mWanderIndex = 0;
}

CVector3f CPlayerCameraBob::CalculateRandomViewWanderPosition(CStateManager& mgr) {
  float angle = 2.f * (M_PIF * mgr.Random()->Float());
  float radius = mViewWanderRadius * mgr.Random()->Float();
  return CVector3f(radius * sinf(angle), 1.f, radius * cosf(angle));
}

float CPlayerCameraBob::CalculateRandomViewWanderPitch(CStateManager& mgr) {
  return CRelAngle::FromDegrees(2.f * (mgr.Random()->Float() - 0.5f) * mViewWanderRollVariation)
      .AsRadians();
}

void CPlayerCameraBob::UpdateViewWander(float dt, CStateManager& mgr) {
  CVector3f point = CMath::GetCatmullRomSplinePoint(
      mWanderPoints[mWanderIndex], mWanderPoints[(mWanderIndex + 1) % 4],
      mWanderPoints[(mWanderIndex + 2) % 4], mWanderPoints[(mWanderIndex + 3) % 4], mWanderTime);
  float pitch = CMath::GetCatmullRomSplinePoint(
      mWanderPitches[mWanderIndex], mWanderPitches[(mWanderIndex + 1) % 4],
      mWanderPitches[(mWanderIndex + 2) % 4], mWanderPitches[(mWanderIndex + 3) % 4], mWanderTime);
  point = CVector3f(mWanderMagnitude * point[0], point[1], mWanderMagnitude * point[2]);
  mViewWanderXf = CTransform4f::LookAt(CVector3f(0.f, 0.f, 0.f), point) *
                  CTransform4f::RotateY(CRelAngle::FromRadians(pitch * mWanderMagnitude));

  mWanderTime += mViewWanderSpeed * dt;
  if (mWanderTime >= 1.f) {
    mWanderPoints[mWanderIndex] = CalculateRandomViewWanderPosition(mgr);
    mWanderPitches[mWanderIndex] = CalculateRandomViewWanderPitch(mgr);
    mViewWanderSpeed =
        (mViewWanderSpeedMax - mViewWanderSpeedMin) * mgr.Random()->Float() + mViewWanderSpeedMin;
    ++mWanderIndex;
    mWanderIndex %= 4;
    mWanderTime -= 1.f;
  }
}

const CTransform4f& CPlayerCameraBob::GetViewWanderTransform() const { return mViewWanderXf; }
