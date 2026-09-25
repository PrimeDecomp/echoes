#ifndef _CTARGETRETICLES
#define _CTARGETRETICLES

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CActor;
class CMatrix3f;
class CScriptGrapplePoint;
class CModel;
class CTexture;
class CStateManager;

enum EReticleState { kRS_Combat, kRS_Scan, kRS_Echo, kRS_Dark, kRS_Four, kRS_Unspecified };

class CTargetReticleRenderState {
public:
  CTargetReticleRenderState(TUniqueId target, float radius, CVector3f position, float factor,
                            float minimumViewportScale, bool orbitZoneIdlePosition);

  static void InterpolateWithClamp(const CTargetReticleRenderState& a,
                                   CTargetReticleRenderState& out,
                                   const CTargetReticleRenderState& b, float t);

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

  void Touch() const;
  static float CalculateClampedScale(CVector3f position, float scale, float clampMin,
                                     float clampMax, const CStateManager& mgr, int playerIndex);
  static bool IsGrappleTarget(TUniqueId id, const CStateManager& mgr);
  CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const;
  CVector3f CalculatePositionWorld(const CActor& actor, const CStateManager& mgr) const;
  float CalculateRadiusWorld(const CActor& actor, const CStateManager& mgr) const;
  void UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr);
  void DrawOrbitZoneGroup(const CMatrix3f& rotation, const CStateManager& mgr) const;
  void DrawNextLockOnGroup(const CMatrix3f& rotation, const CStateManager& mgr) const;
  void DrawScanTargetGroup(const CMatrix3f& rotation,
                           const CStateManager& mgr) const;                   // Guessed name
  void DrawCrosshairs(const CMatrix3f& rotation) const;                       // Guessed name
  void DrawSeeker(const CMatrix3f& rotation, const CStateManager& mgr) const; // Guessed name
  void DrawCurrLockOnGroup(const CMatrix3f& rotation, const CStateManager& mgr) const;
  void DrawGrapplePoint(const CScriptGrapplePoint& point, float factor, const CStateManager& mgr,
                        const CMatrix3f& rotation, bool zEqual) const;
  void DrawGrappleGroup(const CMatrix3f& rotation, const CStateManager& mgr, bool hideLockOn) const;
  void Draw(const CStateManager& mgr, bool hideLockOn) const;
  void UpdateOrbitZoneGroup(float dt, const CStateManager& mgr);
  void UpdateNextLockOnGroup(float dt, const CStateManager& mgr);
  void UpdateCurrLockOnGroup(float dt, const CStateManager& mgr);
  void Update(float dt, const CStateManager& mgr);
  EReticleState GetDesiredReticleState(const CStateManager& mgr) const;
  bool CheckLoadComplete();

private:
  int mPlayerIndex;
  CQuaternion mLeadingOrientation;
  CQuaternion mLaggingOrientation;
  EReticleState mPreviousState;
  EReticleState mNextState;
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
  float mCrosshairsDrawScale;
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
  float mScanBracketFactor;
  float mScanTargetFactor;
  bool mBeamShot : 1;
  bool mMissileShot : 1;
  bool mFullyCharged : 1;
};

class COrbitPointMarker {
public:
  explicit COrbitPointMarker(int playerIndex);

  void ResetInterpolationTimer(float time);
  void Draw(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr);
  bool CheckLoadComplete();

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

  void Touch() const;

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
