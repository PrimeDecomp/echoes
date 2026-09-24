#ifndef _CTARGETRETICLES
#define _CTARGETRETICLES

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CModel;
class CTexture;
class CStateManager;

class CTargetReticleRenderState {
public:
  CTargetReticleRenderState(TUniqueId target, float radius, CVector3f position, float factor,
                            float minimumViewportScale, bool orbitZoneIdlePosition);

private:
  TUniqueId mTarget;
  float mRadius;
  CVector3f mPosition;
  float mFactor;
  float mMinimumViewportScale;
  bool mOrbitZoneIdlePosition;
};

class CCompoundTargetReticle {
public:
  struct SOuterItemInfo {
    explicit SOuterItemInfo(const char* modelName);
    TCachedToken< CModel > mModel;
    float mOffshootBaseAngle;
    float mRotationAngle;
    float mBaseAngle;
    float mOffshootAngleDelta;
  };

  CCompoundTargetReticle(const CStateManager& mgr, int playerIndex);
  ~CCompoundTargetReticle();

private:
  int mPlayerIndex;
  CQuaternion mLeadingOrientation;
  CQuaternion mLaggingOrientation;
  int mPreviousState;
  int mNextState;
  mutable int mNoDrawTicks;
  float mOvershootOffsetHalf;
  float mPremultipliedOvershootOffset;
  TCachedToken< CModel > mCrosshairs;
  TCachedToken< CModel > mSeeker;
  TCachedToken< CModel > mTargetFlower;
  TCachedToken< CModel > mMissileBracket;
  TCachedToken< CModel > mInnerBeamIcon;
  TCachedToken< CModel > mLockConfirm;
  TCachedToken< CModel > mLockFire;
  TCachedToken< CModel > mLockDagger;
  TCachedToken< CModel > mGrapple;
  TCachedToken< CModel > mChargeTickFirst;
  TCachedToken< CModel > mScanTargetCenter;
  TCachedToken< CModel > mScanTargetLeft;
  TCachedToken< CModel > mScanTargetRight;
  SOuterItemInfo mChargeGauge;
  rstl::vector< SOuterItemInfo > mOuterBeamIconSquares;
  TCachedToken< CTexture > mQuarterCurve;
  TCachedToken< CModel > mSeekerMissileLockConfirm;
  TCachedToken< CModel > mSeekerMissileCrosshair;
  TCachedToken< CTexture > mRadarPaintFirst;
  TCachedToken< CTexture > mRadarPaintSecond;
  TUniqueId mTargetId;
  TUniqueId mNextTargetId;
  CVector3f mTargetPosition;
  CVector3f mLaggingTargetPosition;
  CTargetReticleRenderState mCurrentGroupInterpolated;
  CTargetReticleRenderState mCurrentGroupA;
  CTargetReticleRenderState mCurrentGroupB;
  float mCurrentGroupDuration;
  float mCurrentGroupTimer;
  CTargetReticleRenderState mNextGroupInterpolated;
  CTargetReticleRenderState mNextGroupA;
  CTargetReticleRenderState mNextGroupB;
  float mNextGroupDuration;
  float mNextGroupTimer;
  TUniqueId mGrapplePointA;
  TUniqueId mGrapplePointB;
  float mGrapplePointFactorA;
  float mGrapplePointFactorB;
  TUniqueId mVulnerabilityTarget;
  CDamageVulnerability mTargetVulnerability;
  float mCrosshairsScale;
  float mSeekerAngle;
  float x270;
  float x274;
  float x278;
  bool mMissileActive;
  float mMissileBracketTimer;
  float mMissileBracketScaleTimer;
  CPlayerState::EBeamId mBeam;
  float mChargeGaugeOvershootTimer;
  float mLockOnTimer;
  float x294;
  float mLockFireTimer;
  float mFullChargeFadeTimer;
  float x2a0;
  float x2a4;
  bool mBeamShot : 1;
  bool mMissileShot : 1;
  bool mFullyCharged : 1;
};

class COrbitPointMarker {
public:
  explicit COrbitPointMarker(int playerIndex);

private:
  int mPlayerIndex;
  float mZOffset;
  bool mCameraRelativeZ;
  float mLagAzimuth;
  float mAzimuth;
  CVector3f mLagTargetPosition;
  bool mLastFreeOrbit;
  float mInterpolationTimer;
  float mCurrentTime;
  TCachedToken< CModel > mOrbitPointModel;
};

class CTargetingManager {
public:
  CTargetingManager(const CStateManager& mgr, int playerIndex);
  ~CTargetingManager();
  bool CheckLoadComplete();
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, bool hideLockOn) const;

private:
  int mPlayerIndex;
  CCompoundTargetReticle mTargetReticle;
  COrbitPointMarker mOrbitPointMarker;
};

CHECK_SIZEOF(CTargetReticleRenderState, 0x20)
CHECK_SIZEOF(CCompoundTargetReticle, 0x2ac)
CHECK_SIZEOF(COrbitPointMarker, 0x38)
CHECK_SIZEOF(CTargetingManager, 0x2e8)

#endif // _CTARGETRETICLES
