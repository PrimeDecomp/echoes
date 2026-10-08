#include "MetroidPrime/CTargetReticles.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerTargeting.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHUDHint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Native reference cast for entity type 131; the concrete class remains unidentified.
extern "C" CEntity* fn_80097FA8(const CEntity& entity);

// Guessed name, corroborated by Prime.
static CTargetReticleRenderState skZeroRenderState(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f,
                                                   1.f, true);

// Guessed name; the three layout factors precede the native asset-name pool.
static const float skViewportLayoutScale[] = {1.f, 0.8f, 0.6f};

// Guessed name; native scan-lock scaling is independent of viewport radius clamps.
static const float skScanLockLayoutScale[] = {1.f, 1.4f, 1.f};

static bool IsDamageOrbit(CPlayer::EPlayerOrbitRequest request) {
  // Native damage/lock-break requests; the remaining enumerator names are unresolved.
  switch (static_cast< int >(request)) {
  case 5:
  case 8:
  case 9:
  case 10:
  case 11:
    return true;
  default:
    return false;
  }
}

static float offshoot_func(float amplitude, float angularScale, float time) {
  return amplitude * CMath::FastSinR((time - 0.5f) * angularScale) + 0.5f;
}

static float calculate_premultiplied_overshoot_offset(float overshoot) {
  const float angle = static_cast< float >(asin(1.f / overshoot));
  return 2.f * (M_PIF - angle);
}

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* modelName)
: mModel(gpSimplePool->GetObj(modelName))
, mOffshootBaseAngle(0.f)
, mRotationAngle(0.f)
, mBaseAngle(0.f)
, mOffshootAngleDelta(0.f) {}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex)
, mLeadingOrientation(CQuaternion::FromMatrix(
      mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true)->GetTransform()))
, mLaggingOrientation(CQuaternion::FromMatrix(
      mgr.GetCameraManager(playerIndex)->GetCurrentCamera(mgr, true)->GetTransform()))
, mPreviousState(kRS_Unspecified)
, mNextState(kRS_Unspecified)
, mNoDrawTicks(0)
, mOvershootOffsetHalf(0.5f * gpTweakTargeting->GetChargeGaugeOvershootOffset())
, mPremultipliedOvershootOffset(
      calculate_premultiplied_overshoot_offset(gpTweakTargeting->GetChargeGaugeOvershootOffset()))
, mCrosshairs(gpSimplePool->GetObj("CMDL_Crosshairs"))
, mSeeker(gpSimplePool->GetObj("CMDL_Seeker"))
, mTargetFlower(gpSimplePool->GetObj("CMDL_TargetFlower"))
, mMissileBracket(gpSimplePool->GetObj("CMDL_MissileBracket"))
, mInnerBeamIcon(gpSimplePool->GetObj("CMDL_InnerBeamIcon"))
, mLockConfirm(gpSimplePool->GetObj("CMDL_LockConfirm"))
, mLockFire(gpSimplePool->GetObj("CMDL_LockFire"))
, mLockDagger(gpSimplePool->GetObj("CMDL_LockDagger0"))
, mGrapple(gpSimplePool->GetObj("CMDL_Grapple"))
, mChargeTickFirst(gpSimplePool->GetObj("CMDL_ChargeTickFirst"))
, mScanTargetCenter(gpSimplePool->GetObj("CMDL_ScanTargetCenter"))
, mScanTargetLeft(gpSimplePool->GetObj("CMDL_ScanTargetLeft"))
, mScanTargetRight(gpSimplePool->GetObj("CMDL_ScanTargetRight"))
, mChargeGauge("CMDL_ChargeGauge")
, mQuarterCurve(gpSimplePool->GetObj("TXTR_QuaterCurve"))
, mSeekerMissileLockConfirm(gpSimplePool->GetObj("CMDL_SeekerMissileLockConfirm"))
, mSeekerMissileCrosshair(gpSimplePool->GetObj("CMDL_SeekerMissileCrosshair"))
, mRadarPaintFirst(gpSimplePool->GetObj("TXTR_RadarPaint"))
, mRadarPaintSecond(gpSimplePool->GetObj("TXTR_RadarPaint"))
, mTargetId(kInvalidUniqueId)
, mNextTargetId(kInvalidUniqueId)
, mTargetPosition(CalculateOrbitZoneReticlePosition(mgr, false))
, mLaggingTargetPosition(CalculateOrbitZoneReticlePosition(mgr, true))
, mCurrentGroupInterpolated(skZeroRenderState)
, mCurrentGroupA(skZeroRenderState)
, mCurrentGroupB(skZeroRenderState)
, mCurrentGroupDuration(0.f)
, mCurrentGroupTimer(0.f)
, mNextGroupInterpolated(skZeroRenderState)
, mNextGroupA(skZeroRenderState)
, mNextGroupB(skZeroRenderState)
, mNextGroupDuration(0.f)
, mNextGroupTimer(0.f)
, mGrapplePointA(kInvalidUniqueId)
, mGrapplePointB(kInvalidUniqueId)
, mGrapplePointFactorA(0.f)
, mGrapplePointFactorB(0.f)
, mVulnerabilityTarget(kInvalidUniqueId)
, mTargetVulnerability(CDamageVulnerability::ImmuneVulnerabilty())
, mTargetHealth(0.f)
, mTargetHealthShadow(0.f)
, mCrosshairsDrawScale(0.f)
, mSeekerRotationAngle(0.f)
, mFlowerRotationAngle(0.f)
, mMissileActive(false)
, mMissileBracketTimer(0.f)
, mMissileBracketScaleTimer(0.f)
, mBeam(CPlayerState::kBI_Power)
, mChargeGaugeOvershootTimer(0.f)
, mLockOnTimer(0.f)
, mNextTargetFade(0.f)
, mLockFireTimer(0.f)
, mFullChargeFadeTimer(0.f)
, mScanBracketFactor(0.f)
, mScanTargetFactor(0.f)
, mBeamShot(false)
, mMissileShot(false)
, mFullyCharged(false) {
  mOuterBeamIconSquares.reserve(9);
  for (int i = 0; i < 9; ++i) {
    char name[64];
    sprintf(name, "%s%d", "CMDL_BeamSquare", i);
    mOuterBeamIconSquares.push_back_unsafe(SOuterItemInfo(name));
  }
  mCrosshairs.Lock();
  mQuarterCurve.Lock();
  mSeeker.Lock();
  mGrapple.Lock();
  mSeekerMissileLockConfirm.Lock();
  mSeekerMissileCrosshair.Lock();
  mRadarPaintFirst.Lock();
  mRadarPaintSecond.Lock();
}

bool CCompoundTargetReticle::CheckLoadComplete() { return true; }

EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const {
  switch (mgr.GetPlayerState(mPlayerIndex)->GetCurrentVisor()) {
  case CPlayerState::kPV_Scan:
    return kRS_Scan;
  case CPlayerState::kPV_Echo:
    return kRS_Echo;
  case CPlayerState::kPV_Combat:
    return kRS_Combat;
  case CPlayerState::kPV_Dark:
    return kRS_Dark;
  default:
    return kRS_Combat;
  }
}

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr) {
  // Orientation slerp
  CRelAngle angle = mLaggingOrientation.AngleFrom(mLeadingOrientation);
  float angleDeg = angle.AsDegrees();
  bool extreme = false;
  if (angleDeg < 0.1f || angleDeg > 45.f) {
    extreme = true;
  }
  float t;
  if (extreme) {
    t = 1.f;
  } else {
    float lagSpeed = gpTweakTargeting->GetAngularLagSpeed();
    t = rstl::min_val(1.f, lagSpeed * dt / angleDeg);
  }
  mLaggingOrientation = t == 1.f ? mLeadingOrientation
                                 : CQuaternion::Slerp(mLaggingOrientation, mLeadingOrientation, t);

  // Target positions
  mTargetPosition = CalculateOrbitZoneReticlePosition(mgr, false);
  mLaggingTargetPosition = CalculateOrbitZoneReticlePosition(mgr, true);

  // Sub-updates
  UpdateCurrLockOnGroup(dt, mgr);
  UpdateNextLockOnGroup(dt, mgr);
  UpdateOrbitZoneGroup(dt, mgr);

  // Reticle state transitions
  EReticleState desiredState = GetDesiredReticleState(mgr);
  if (desiredState != mPreviousState && mPreviousState == mNextState) {
    mNextState = desiredState;
    mNoDrawTicks = 2;
  }

  if (mPreviousState != mNextState && mNoDrawTicks <= 0) {
    mPreviousState = mNextState;
    bool combat = false;
    bool scan = false;
    switch (mNextState) {
    case kRS_Combat:
    case kRS_Echo:
    case kRS_Dark:
      combat = true;
      break;
    case kRS_Scan:
      scan = true;
      break;
    default:
      break;
    }

    if (combat) {
      mSeeker.Lock();
      mLockConfirm.Lock();
      mTargetFlower.Lock();
      mMissileBracket.Lock();
      mInnerBeamIcon.Lock();
      mLockFire.Lock();
      mLockDagger.Lock();
      mChargeTickFirst.Lock();
      mChargeGauge.mModel.Lock();
    } else {
      mSeeker.Unlock();
      mLockConfirm.Unlock();
      mTargetFlower.Unlock();
      mMissileBracket.Unlock();
      mInnerBeamIcon.Unlock();
      mLockFire.Unlock();
      mLockDagger.Unlock();
      mChargeTickFirst.Unlock();
      mChargeGauge.mModel.Unlock();
    }
    if (scan) {
      mGrapple.Unlock();
      mScanTargetCenter.Lock();
      mScanTargetLeft.Lock();
      mScanTargetRight.Lock();
    } else {
      mGrapple.Lock();
      mScanTargetCenter.Unlock();
      mScanTargetLeft.Unlock();
      mScanTargetRight.Unlock();
    }
    for (rstl::vector< SOuterItemInfo >::iterator it = mOuterBeamIconSquares.begin();
         it != mOuterBeamIconSquares.end(); ++it) {
      if (combat) {
        it->mModel.Lock();
      } else {
        it->mModel.Unlock();
      }
    }
  }

  // Charge gauge / fully charged
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  bool fullyCharged = player->GetPlayerState()->GetChargeBeamFactor() >= 1.f;
  if (fullyCharged != mFullyCharged) {
    mFullyCharged = fullyCharged;
  }
  if (mFullyCharged) {
    mFullChargeFadeTimer =
        rstl::min_val(gpTweakTargeting->GetChargeGaugeGlowTime(),
                      mFullChargeFadeTimer + dt / gpTweakTargeting->GetChargeGaugeGlowTime());
  } else {
    mFullChargeFadeTimer =
        rstl::max_val(0.f, mFullChargeFadeTimer - dt / gpTweakTargeting->GetChargeGaugeGlowTime());
  }

  // Missile active state
  bool missileActive = player->GetPlayerGun()->GetMissileMode();
  if (missileActive != mMissileActive) {
    if (mMissileBracketTimer != 0.f) {
      mMissileBracketTimer = FLT_EPSILON - mMissileBracketTimer;
    } else {
      mMissileBracketTimer = FLT_EPSILON;
    }
    mMissileActive = missileActive;
  }

  // Beam change
  CPlayerState::EBeamId beam = player->GetPlayerGun()->GetPrimaryWeaponId();
  if (beam != mBeam) {
    mChargeGaugeOvershootTimer = gpTweakTargeting->GetOuterBeamIconSwitchTime();
    for (int i = 0; i < 9; ++i) {
      SOuterItemInfo& icon = mOuterBeamIconSquares[i];
      float baseAngle = CMath::ClampRadians(gpTweakTargeting->GetOuterBeamIconAngle(beam, i));
      CRelAngle offshootAngleDelta = CRelAngle::FromRadians(baseAngle - icon.mRotationAngle);
      if (i % 2 == 1) {
        offshootAngleDelta =
            offshootAngleDelta.AsRadians() > 0.f
                ? CRelAngle::FromRadians(-1.f * (M_2PIF - offshootAngleDelta.AsRadians()))
                : CRelAngle::FromRadians(M_2PIF + offshootAngleDelta.AsRadians());
      }
      icon.mOffshootBaseAngle = icon.mRotationAngle;
      icon.mOffshootAngleDelta = offshootAngleDelta.AsRadians();
      icon.mBaseAngle = baseAngle;
    }

    float chargeBaseAngle = CMath::ClampRadians(gpTweakTargeting->GetChargeGaugeAngle(beam));
    bool odd = rand() % 2 == 1;
    CRelAngle chargeOffshootAngleDelta =
        CRelAngle::FromRadians(chargeBaseAngle - mChargeGauge.mRotationAngle);
    if (odd) {
      chargeOffshootAngleDelta =
          chargeOffshootAngleDelta.AsRadians() > 0.f
              ? CRelAngle::FromRadians(-1.f * (M_2PIF - chargeOffshootAngleDelta.AsRadians()))
              : CRelAngle::FromRadians(M_2PIF + chargeOffshootAngleDelta.AsRadians());
    }
    mChargeGauge.mOffshootBaseAngle = mChargeGauge.mRotationAngle;
    mChargeGauge.mOffshootAngleDelta = chargeOffshootAngleDelta.AsRadians();
    mChargeGauge.mBaseAngle = chargeBaseAngle;
    mBeam = beam;
    mLockOnTimer = 0.f;
  }

  // Beam shot / lock fire
  const CPlayerGun* gun = player->GetPlayerGun();
  if (gun->GetFiring() & 0x1) {
    if (!mBeamShot) {
      mLockFireTimer = gpTweakTargeting->GetLockFireAnimTime();
    }
    mBeamShot = true;
  } else {
    mBeamShot = false;
  }

  // Missile shot / missile bracket scale
  if (gun->GetFiring() & 0x2) {
    if (!mMissileShot) {
      mMissileBracketScaleTimer = gpTweakTargeting->GetMissileBracketMissileFireAnimTime();
    }
    mMissileShot = true;
  } else {
    mMissileShot = false;
  }

  // Grapple point tracking
  const CScriptGrapplePoint* grapplePoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mNextTargetId));
  if (grapplePoint != nullptr) {
    TUniqueId gpId = grapplePoint->GetUniqueId();
    if (gpId != mGrapplePointA) {
      float tmp;
      if (gpId == mGrapplePointB) {
        tmp = rstl::max_val(FLT_EPSILON, mGrapplePointFactorB);
      } else {
        tmp = FLT_EPSILON;
      }
      mGrapplePointB = mGrapplePointA;
      mGrapplePointFactorB = mGrapplePointFactorA;
      mGrapplePointFactorA = tmp;
      mGrapplePointA = gpId;
    }
  } else {
    if (mGrapplePointA != kInvalidUniqueId) {
      mGrapplePointB = mGrapplePointA;
      mGrapplePointFactorB = mGrapplePointFactorA;
      mGrapplePointFactorA = 0.f;
      mGrapplePointA = kInvalidUniqueId;
    }
  }

  // Grapple point interpolation timers
  if (mGrapplePointFactorA > 0.f) {
    mGrapplePointFactorA = rstl::min_val(1.f, mGrapplePointFactorA + dt / 0.5f);
  }
  if (mGrapplePointFactorB > 0.f) {
    mGrapplePointFactorB = rstl::max_val(0.f, mGrapplePointFactorB - dt / 0.5f);
    if (mGrapplePointFactorB == 0.f) {
      mGrapplePointB = kInvalidUniqueId;
    }
  }

  // Xray/seeker angle updates
  mFlowerRotationAngle = CMath::ClampRadians(
      mFlowerRotationAngle +
      CRelAngle::FromDegrees(dt * gpTweakTargeting->GetXRayReticleRotationRate()).AsRadians());
  mSeekerRotationAngle = CMath::ClampRadians(
      mSeekerRotationAngle +
      CRelAngle::FromDegrees(dt * gpTweakTargeting->GetSeekerZRotationRate()).AsRadians());
}

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  const TUniqueId targetId = player->GetOrbitTargetId();

  if (targetId != mTargetId) {
    if (mTargetId != targetId && targetId != kInvalidUniqueId) {
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(targetId))) {
        CSfxManager::SfxStart(0x1db, 127, player->GetSoundPan(CPlayer::kMSP_4));
      } else {
        CSfxManager::SfxStart(mgr.ReturnFirstIfSingleElseSecond(0x79, 0x2654), 127,
                              player->GetSoundPan(CPlayer::kMSP_4));
      }
    }

    mTargetVulnerability = CDamageVulnerability::ImmuneVulnerabilty();
    mVulnerabilityTarget = kInvalidUniqueId;

    if (kInvalidUniqueId == targetId) {
      CPlayer::EPlayerOrbitRequest orbitBrokenType = player->GetOrbitRequest();
      mCurrentGroupA = mCurrentGroupInterpolated;
      mCurrentGroupA.SetIsOrbitZoneIdlePosition(false);
      mCurrentGroupB.SetFactor(0.f);
      mCurrentGroupDuration =
          IsDamageOrbit(orbitBrokenType) ? 0.65f : gpTweakTargeting->GetCurrLockOnEnterDuration();
    } else {
      mCurrentGroupA = mCurrentGroupInterpolated;
      mCurrentGroupA.SetIsOrbitZoneIdlePosition(false);
      if (mTargetId == kInvalidUniqueId) {
        mCurrentGroupA.SetTargetId(targetId);
      }
      float scale =
          IsGrappleTarget(targetId, mgr) ? gpTweakTargeting->GetGrappleMinClampScale() : 1.f;
      mCurrentGroupB =
          CTargetReticleRenderState(targetId, 1.f, CVector3f::Zero(), 1.f, scale, false);
      mCurrentGroupDuration = (kInvalidUniqueId == mTargetId)
                                  ? gpTweakTargeting->GetCurrLockOnExitDuration()
                                  : gpTweakTargeting->GetCurrLockOnSwitchDuration();
      if (targetId != mgr.GetBossId()) {
        if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(targetId))) {
          if (const CScannableObjectInfo* info = actor->GetScannableObjectInfo()) {
            if (const_cast< CPlayerState* >(player->GetPlayerState())
                    ->GetScanTime(info->GetScannableObjectId()) >= 0.99999988f) {
              mTargetVulnerability = *actor->GetDamageVulnerability();
              mVulnerabilityTarget = actor->GetUniqueId();
              mTargetHealth = 0.f;
              mTargetHealthShadow = 0.f;
            }
          }
        }
      }
    }

    mCurrentGroupTimer = mCurrentGroupDuration;
    mTargetId = targetId;
  }

  if (mVulnerabilityTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mVulnerabilityTarget))) {
      if (actor->GetHealthInfo() && !close_enough(actor->GetHealthInfo()->GetInitialHP(), 0.f)) {
        float health = rstl::max_val(0.f, actor->GetHealthInfo()->GetHP());
        float initialHealth = rstl::max_val(0.f, actor->GetHealthInfo()->GetInitialHP());
        if (mTargetHealth > initialHealth) {
          mTargetHealth = health;
        }
        if (mTargetHealthShadow > initialHealth) {
          mTargetHealthShadow = health;
        }
        if (mTargetHealth > health) {
          mTargetHealth = rstl::max_val(
              health,
              mTargetHealth - initialHealth * dt / gpTweakTargeting->GetHealthMeterLagDrainTime());
        } else {
          mTargetHealth = rstl::min_val(
              health,
              mTargetHealth + initialHealth * dt / gpTweakTargeting->GetHealthMeterLagFillTime());
        }
        if (mTargetHealthShadow > mTargetHealth) {
          mTargetHealthShadow = rstl::max_val(
              mTargetHealth,
              mTargetHealthShadow -
                  initialHealth * dt / gpTweakTargeting->GetHealthMeterShadowDrainTime());
        } else {
          mTargetHealthShadow = mTargetHealth;
        }
      }
    }
  }

  if (mCurrentGroupTimer > 0.f) {
    UpdateTargetParameters(mCurrentGroupA, mgr);
    UpdateTargetParameters(mCurrentGroupB, mgr);
    mCurrentGroupTimer = rstl::max_val(0.f, mCurrentGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(
        mCurrentGroupA, mCurrentGroupInterpolated, mCurrentGroupB,
        1.f - mCurrentGroupTimer / mCurrentGroupDuration);
  } else {
    UpdateTargetParameters(mCurrentGroupInterpolated, mgr);
  }

  if (mMissileBracketTimer != 0.f &&
      mMissileBracketTimer < gpTweakTargeting->GetMissileBracketOpenHolsterTime()) {
    if (mMissileBracketTimer < 0.f) {
      mMissileBracketTimer = rstl::min_val(mMissileBracketTimer + dt, 0.f);
    } else {
      mMissileBracketTimer = rstl::min_val(mMissileBracketTimer + dt,
                                           gpTweakTargeting->GetMissileBracketOpenHolsterTime());
    }
  }

  if (mChargeGaugeOvershootTimer > 0.f) {
    mChargeGaugeOvershootTimer = rstl::max_val(mChargeGaugeOvershootTimer - dt, 0.f);
    if (mChargeGaugeOvershootTimer == 0.f) {
      for (int i = 0; i < 9; ++i) {
        mOuterBeamIconSquares[i].mRotationAngle = mOuterBeamIconSquares[i].mBaseAngle;
      }
      mChargeGauge.mRotationAngle = mChargeGauge.mBaseAngle;
      mLockOnTimer = FLT_EPSILON;
    } else {
      float offshoot = offshoot_func(mOvershootOffsetHalf, mPremultipliedOvershootOffset,
                                     1.f - mChargeGaugeOvershootTimer /
                                               gpTweakTargeting->GetOuterBeamIconSwitchTime());
      for (int i = 0; i < 9; ++i) {
        SOuterItemInfo& item = mOuterBeamIconSquares[i];
        float angleDelta = offshoot * item.mOffshootAngleDelta;
        item.mRotationAngle = CMath::ClampRadians(angleDelta + item.mOffshootBaseAngle);
      }
      mChargeGauge.mRotationAngle = CMath::ClampRadians(
          mChargeGauge.mOffshootBaseAngle + offshoot * mChargeGauge.mOffshootAngleDelta);
    }
  }

  if (mLockOnTimer > 0.f && mLockOnTimer < gpTweakTargeting->GetInnerBeamIconOpenTime()) {
    mLockOnTimer = rstl::min_val(mLockOnTimer + dt, gpTweakTargeting->GetInnerBeamIconOpenTime());
  }

  if (mLockFireTimer > 0.f) {
    mLockFireTimer = rstl::max_val(0.f, mLockFireTimer - dt);
  }

  if (mMissileBracketScaleTimer > 0.f) {
    mMissileBracketScaleTimer = rstl::max_val(0.f, mMissileBracketScaleTimer - dt);
  }

  player = mgr.GetPlayer(mPlayerIndex);
  if (mPreviousState == kRS_Scan &&
      player->GetTargeting()->GetResolvedTargetId() != kInvalidUniqueId) {
    mScanBracketFactor =
        rstl::min_val(1.f, mScanBracketFactor + dt / gpTweakTargeting->GetScanLockTransitionTime());
  } else {
    mScanBracketFactor =
        rstl::max_val(0.f, mScanBracketFactor - dt / gpTweakTargeting->GetScanLockTransitionTime());
  }
  if (player->GetScanningObject() == kInvalidUniqueId) {
    mScanTargetFactor = rstl::min_val(1.f, 4.f * dt + mScanTargetFactor);
  } else {
    mScanTargetFactor = rstl::max_val(0.f, mScanTargetFactor - 4.f * dt);
  }
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  TUniqueId nextTargetId = player->GetOrbitNextTargetId();
  if (mPreviousState == kRS_Scan) {
    nextTargetId = kInvalidUniqueId;
  }

  if (nextTargetId != mNextTargetId) {
    if (kInvalidUniqueId == nextTargetId) {
      mNextGroupA = mNextGroupInterpolated;
      mNextGroupA.SetIsOrbitZoneIdlePosition(false);
      bool lag = mPreviousState == kRS_Echo || mPreviousState == kRS_Dark;
      mNextGroupB = CTargetReticleRenderState(
          kInvalidUniqueId, 1.f, lag ? mLaggingTargetPosition : mTargetPosition, 0.f, 1.f, true);
      mNextGroupDuration = gpTweakTargeting->GetNextLockOnExitDuration();
      mNextGroupTimer = mNextGroupDuration;
      mNextTargetId = kInvalidUniqueId;
    } else {
      mNextGroupA = mNextGroupInterpolated;
      mNextGroupA.SetIsOrbitZoneIdlePosition(false);
      float scale =
          IsGrappleTarget(nextTargetId, mgr) ? gpTweakTargeting->GetGrappleMinClampScale() : 1.f;
      mNextGroupB =
          CTargetReticleRenderState(nextTargetId, 1.f, CVector3f::Zero(), 1.f, scale, true);
      mNextGroupDuration = kInvalidUniqueId == mNextTargetId
                               ? gpTweakTargeting->GetNextLockOnEnterDuration()
                               : gpTweakTargeting->GetNextLockOnSwitchDuration();
      mNextGroupTimer = mNextGroupDuration;
      mNextTargetId = nextTargetId;
    }
  }

  if (mNextGroupTimer > 0.f) {
    UpdateTargetParameters(mNextGroupA, mgr);
    UpdateTargetParameters(mNextGroupB, mgr);
    mNextGroupTimer = rstl::max_val(0.f, mNextGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(mNextGroupA, mNextGroupInterpolated,
                                                    mNextGroupB,
                                                    1.f - mNextGroupTimer / mNextGroupDuration);
  } else {
    UpdateTargetParameters(mNextGroupInterpolated, mgr);
  }
}

void CCompoundTargetReticle::UpdateOrbitZoneGroup(float dt, const CStateManager& mgr) {
  if (mTargetId == kInvalidUniqueId && mNextTargetId != kInvalidUniqueId) {
    mNextTargetFade = rstl::min_val(2.f * dt + mNextTargetFade, 1.f);
  } else {
    mNextTargetFade = rstl::max_val(mNextTargetFade - 2.f * dt, 0.f);
  }

  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  if (player->GetDrawCrosshairs() &&
      player->GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mCrosshairsDrawScale = rstl::min_val(
        mCrosshairsDrawScale + dt / gpTweakTargeting->GetCrosshairsFadeInOutTime(), 1.f);
  } else {
    mCrosshairsDrawScale = rstl::max_val(
        mCrosshairsDrawScale - dt / gpTweakTargeting->GetCrosshairsFadeInOutTime(), 0.f);
  }
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockOn) const {
  if (mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      !mgr.GetCameraManager(mPlayerIndex)->IsInCinematicCamera()) {
    CTransform4f cameraXf =
        mgr.GetCameraManager(mPlayerIndex)->GetCurrentCameraTransform(mgr, true);
    CGraphics::SetViewPointMatrix(cameraXf);
    CMatrix3f rotation = cameraXf.BuildMatrix3f();
    CGraphics::SetCullMode(kCM_None);

    if (!hideLockOn) {
      DrawCurrLockOnGroup(rotation, mgr);
      DrawSeeker(rotation, mgr);
      DrawCrosshairs(rotation, mgr);
      DrawScanTargetGroup(rotation, mgr);
      DrawNextLockOnGroup(rotation, mgr);
      DrawOrbitZoneGroup(rotation, mgr);
    }
    DrawGrappleGroup(rotation, mgr, hideLockOn);
    CGraphics::SetCullMode(kCM_Front);
  }

  if (mNoDrawTicks > 0) {
    --mNoDrawTicks;
  }
}

void CCompoundTargetReticle::DrawGrappleGroup(const CMatrix3f& rotation, const CStateManager& mgr,
                                              bool hideLockOn) const {
  if (mNoDrawTicks > 0) {
    return;
  }
  const_cast< TCachedToken< CModel >& >(mGrapple).TryCache();
  if (mGrapple.GetObject() == nullptr) {
    return;
  }
  if (mPreviousState == kRS_Scan) {
    return;
  }

  const rstl::list< CEntity* >& list = mgr.GetGrapplePointList();
  if (!mgr.GetPlayerState(mPlayerIndex)->HasPowerUp(CPlayerState::kIT_GrappleBeam)) {
    return;
  }
  float depthNear = CGraphics::GetDepthNear();
  float depthFar = CGraphics::GetDepthFar();
  CGraphics::SetDepthRange(0.125f, 1.f);

  if (hideLockOn) {
    for (rstl::list< CEntity* >::const_iterator it = list.begin(); it != list.end(); ++it) {
      const CScriptGrapplePoint* point = static_cast< const CScriptGrapplePoint* >(*it);
      if (point == nullptr || !point->GetActive() ||
          !(point->GetValidTargetPlayers() & (1 << mPlayerIndex))) {
        continue;
      }
      TAreaId areaId = point->GetCurrentAreaId();
      if (areaId != kInvalidAreaId) {
        const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
        if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
          continue;
        }
      }

      float factor = 0.f;
      TUniqueId id = point->GetUniqueId();
      if (id == mGrapplePointA) {
        factor = mGrapplePointFactorA;
      } else if (id == mGrapplePointB) {
        factor = mGrapplePointFactorB;
      }
      if (close_enough(factor, 0.f, 0.00001f)) {
        DrawGrapplePoint(*point, factor, mgr, rotation, true);
      }
    }
  } else {
    const CScriptGrapplePoint* pointA =
        TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mGrapplePointA));
    const CScriptGrapplePoint* pointB =
        TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mGrapplePointB));
    for (int i = 0; i < 2; ++i) {
      const CScriptGrapplePoint* point = i == 0 ? pointA : pointB;
      float factor = i == 0 ? mGrapplePointFactorA : mGrapplePointFactorB;
      if (point != nullptr) {
        DrawGrapplePoint(*point, factor, mgr, rotation, true);
      }
    }
  }

  CGraphics::SetDepthRange(depthNear, depthFar);
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float factor,
                                              const CStateManager& mgr, const CMatrix3f& rotation,
                                              bool zEqual) const {
  CVector3f orbitPosition = point.GetOrbitPosition(mgr);
  CColor selectedColor;
  if (const CScriptGrapplePoint* grapple = TCastToConstPtr< CScriptGrapplePoint >(point)) {
    selectedColor = grapple->GetGrappleParameters().GetConstrainToAxis()
                        ? gpTweakTargeting->GetLockedGrapplePointColor()
                        : gpTweakTargeting->GetGrappleIconColor();
  } else {
    selectedColor = CColor::White();
  }
  CColor color =
      CColor::Lerp(gpTweakTargeting->GetGrappleIconColorInactive(), selectedColor, factor);
  const CTweakTargeting* tweak = gpTweakTargeting.get();
  factor =
      (1.f - factor) * tweak->GetGrappleIconScaleInactive() + factor * tweak->GetGrappleIconScale();
  float scale =
      CalculateClampedScale(orbitPosition, 1.f, gpTweakTargeting->GetGrappleIconMinRadiusViewport(),
                            gpTweakTargeting->GetGrappleIconMaxRadiusViewport(), mgr, mPlayerIndex);
  scale *= factor;

  CMatrix3f scaledRotation = rotation * CMatrix3f::Scale(scale);
  gpRender->SetModelMatrix(CTransform4f(scaledRotation, orbitPosition));
  const CModel* model = mGrapple.GetObject();
  model->Draw(CModelFlags::Additive(color).DepthCompareUpdate(zEqual, false));
}

void CCompoundTargetReticle::DrawCurrLockOnGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  if (mNoDrawTicks > 0)
    return;

  CVector3f position = mCurrentGroupInterpolated.GetTargetPositionWorld();
  float radius = mCurrentGroupInterpolated.GetRadiusWorld();

  if (mGrapplePointFactorA + mGrapplePointFactorB > 0.f)
    return;

  float factor = mCurrentGroupInterpolated.GetFactor();
  float lockBreakAlpha = factor;
  if (0.f == factor)
    return;

  float visorFactor = mgr.GetPlayerState(mPlayerIndex)->GetVisorTransitionFactor();
  float minVpClampScale = mCurrentGroupInterpolated.GetMinViewportClampScale();

  bool lockConfirm = false;
  bool lockReticule = false;

  bool healthMeter = false;
  switch (mPreviousState) {
  case kRS_Combat:
  case kRS_Echo:
  case kRS_Dark:
    lockConfirm = true;
    lockReticule = true;
    healthMeter = true;
    break;
  default:
    break;
  }

  CMatrix3f lockBreakXf(CMatrix3f::Identity());
  CColor lockBreakColor(0);

  if (IsDamageOrbit(mgr.GetPlayer(mPlayerIndex)->GetOrbitRequest()) &&
      mCurrentGroupB.GetFactor() == 0.f) {
    CVector3f columns[3] = {CVector3f::Right(), CVector3f::Forward(), CVector3f::Up()};

    for (int i = 0; i < 4; ++i) {
      int r1 = rand();
      int idx = rand() % 9;
      int col = idx % 3;
      int row = idx / 3;
      columns[col][row] += static_cast< float >(r1) / static_cast< float >(RAND_MAX) - 0.5f;
    }

    lockBreakXf = CMatrix3f(columns[0], columns[1], columns[2]);

    if (factor > 0.8f) {
      lockBreakColor = CColor::White().WithAlphaOf(0.3f * (factor - 0.8f) / 0.2f);
    }

    if (factor > 0.75f) {
      lockBreakAlpha = 1.f;
    } else {
      lockBreakAlpha = rstl::max_val((factor - 0.55f) / 0.2f, 0.f);
    }
  }

  if (lockConfirm) {
    const_cast< TCachedToken< CModel >& >(mLockConfirm).TryCache();
    if (CModel* const model = mLockConfirm.GetObject()) {
      CTweakTargeting* tweak = gpTweakTargeting.get();
      float scale = CalculateClampedScale(
          position, radius, minVpClampScale * tweak->GetLockOnConfirmMinRadiusViewport(),
          tweak->GetLockOnConfirmMaxRadiusViewport(), mgr, mPlayerIndex);
      scale *= gpTweakTargeting->GetLockOnConfirmReticleScale();
      scale /= factor;

      CMatrix3f combined = rotation *
                           CMatrix3f::RotateY(CRelAngle::FromRadians(mSeekerRotationAngle)) *
                           CMatrix3f::Scale(scale);

      gpRender->SetModelMatrix(
          CTransform4f(lockBreakXf * combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

      model->Draw(CModelFlags::Additive(
                      CColor::Add(lockBreakColor,
                                  tweak->GetLockOnConfirmReticleColor().WithAlphaModulatedBy(
                                      lockBreakAlpha)))
                      .DepthCompareUpdate(false, false));
    }
  }

  if (lockReticule) {
    // Target flower
    const_cast< TCachedToken< CModel >& >(mTargetFlower).TryCache();
    if (CModel* const model = mTargetFlower.GetObject()) {
      float scale = CalculateClampedScale(
          position, radius, minVpClampScale * gpTweakTargeting->GetFlowerMinRadiusViewport(),
          gpTweakTargeting->GetFlowerMaxRadiusViewport(), mgr, mPlayerIndex);
      CTweakTargeting* tweak = gpTweakTargeting.get();
      scale *= tweak->GetFlowerReticleScale();
      scale /= lockBreakAlpha;

      CMatrix3f combined = rotation *
                           CMatrix3f::RotateY(CRelAngle::FromRadians(mFlowerRotationAngle)) *
                           CMatrix3f::Scale(scale);

      gpRender->SetModelMatrix(
          CTransform4f(lockBreakXf * combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

      model->Draw(
          CModelFlags::Additive(
              CColor::Add(lockBreakColor, tweak->GetFlowerReticleColor().WithAlphaModulatedBy(
                                              lockBreakAlpha * visorFactor)))
              .DepthCompareUpdate(true, false));
    }

    // Missile bracket
    if (mMissileBracketTimer != 0.f) {
      const_cast< TCachedToken< CModel >& >(mMissileBracket).TryCache();
      if (CModel* const bracketModel = mMissileBracket.GetObject()) {
        float bracketScale = CalculateClampedScale(
            position, radius,
            minVpClampScale * gpTweakTargeting->GetMissileBracketMinRadiusViewport(),
            gpTweakTargeting->GetMissileBracketMaxRadiusViewport(), mgr, mPlayerIndex);
        CTweakTargeting* tweak = gpTweakTargeting.get();
        float halfDur = 0.5f * tweak->GetMissileBracketMissileFireAnimTime();
        float t = CMath::AbsF((mMissileBracketScaleTimer - halfDur) / halfDur);
        float tscale = (1.f - t) * tweak->GetMissileBracketScaleEnd() +
                       t * tweak->GetMissileBracketScaleStart();
        float bracketFactor =
            CMath::AbsF(mMissileBracketTimer) / tweak->GetMissileBracketOpenHolsterTime();
        float s = bracketFactor * bracketScale * tscale / factor;

        CMatrix3f scaleMtx = CMatrix3f::Scale(s);

        for (int i = 0; i < 4; ++i) {
          float xSign = i < 2 ? 1.f : -1.f;
          float zSign = (i & 1) != 0 ? 1.f : -1.f;
          CMatrix3f combined = lockBreakXf * rotation *
                               CMatrix3f(xSign, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, zSign) *
                               scaleMtx;

          gpRender->SetModelMatrix(
              CTransform4f(combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

          bracketModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->GetMissileBracketColor().WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }

    // Outer beam icon squares
    {
      float outerScale = CalculateClampedScale(
          position, radius, minVpClampScale * gpTweakTargeting->GetOuterIconMinRadiusViewport(),
          gpTweakTargeting->GetOuterIconMaxRadiusViewport(), mgr, mPlayerIndex);
      outerScale = gpTweakTargeting->GetOuterBeamIconScale() * (1.f / factor * outerScale);

      CMatrix3f outerBeamXf = rotation * CMatrix3f::Scale(outerScale);
      int i;
      CTweakTargeting* tweak = gpTweakTargeting.get();

      for (i = 0; i < 9; ++i) {
        const SOuterItemInfo& info = mOuterBeamIconSquares[i];
        const_cast< TCachedToken< CModel >& >(info.mModel).TryCache();
        CModel* const outerModel = info.mModel.GetObject();
        if (outerModel != nullptr) {
          CRelAngle outerAngle = CRelAngle::FromRadians(info.mRotationAngle);
          CMatrix3f combined = outerBeamXf * CMatrix3f::RotateY(outerAngle);

          gpRender->SetModelMatrix(CTransform4f(
              lockBreakXf * combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

          outerModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->GetOuterBeamIconColor().WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }

    // Charge gauge
    {
      const_cast< SOuterItemInfo& >(mChargeGauge).mModel.TryCache();
      if (CModel* const gaugeModel = mChargeGauge.mModel.GetObject()) {
        float gaugeScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->GetOuterIconMinRadiusViewport(),
            gpTweakTargeting->GetOuterIconMaxRadiusViewport(), mgr, mPlayerIndex);
        gaugeScale = gaugeScale * gpTweakTargeting->GetChargeGaugeScale() / factor;

        CMatrix3f gaugeMtx = rotation * CMatrix3f::Scale(gaugeScale);
        CRelAngle gaugeAngle = CRelAngle::FromRadians(mChargeGauge.mRotationAngle);
        CMatrix3f chargeGaugeXf = gaugeMtx * CMatrix3f::RotateY(gaugeAngle);

        float chargeFadeFactor = mFullChargeFadeTimer / gpTweakTargeting->GetChargeGaugeGlowTime();
        float pulsePeriod = gpTweakTargeting->GetChargeGaugePulsePeriod();
        float secondsMod = CGraphics::GetSecondsMod900();
        float pulseT = CMath::AbsF(static_cast< float >(
            fmod(static_cast< double >(secondsMod), static_cast< double >(pulsePeriod))));
        float halfPeriod = 0.5f * pulsePeriod;
        float pulseRatio;
        if (pulseT < halfPeriod) {
          pulseRatio = pulseT / halfPeriod;
        } else {
          pulseRatio = (pulsePeriod - pulseT) / halfPeriod;
        }

        CColor pulseColor = CColor::Lerp(gpTweakTargeting->GetChargeGaugeGlowColor(),
                                         gpTweakTargeting->GetChargeGaugeGlowColorB(), pulseRatio);
        CColor gaugeColor =
            CColor::Lerp(gpTweakTargeting->GetChargeGaugeColor(), pulseColor, chargeFadeFactor);

        CTransform4f modelXf = CTransform4f(lockBreakXf * chargeGaugeXf,
                                            mCurrentGroupInterpolated.GetTargetPositionWorld());
        gpRender->SetModelMatrix(modelXf);

        gaugeModel->Draw(
            CModelFlags::Additive(CColor::Add(lockBreakColor, gaugeColor.WithAlphaModulatedBy(
                                                                  lockBreakAlpha * visorFactor)))
                .DepthCompareUpdate(false, false));

        // Charge ticks
        const_cast< TCachedToken< CModel >& >(mChargeTickFirst).TryCache();
        CModel* const tickModel = mChargeTickFirst.GetObject();
        if (tickModel != nullptr) {
          int numTicks =
              static_cast< int >(static_cast< float >(gpTweakTargeting->GetChargeTickCount()) *
                                 mgr.GetPlayerState(mPlayerIndex)->GetChargeBeamFactor());
          for (int i = 0; i < numTicks; ++i) {
            tickModel->Draw(CModelFlags::Additive(
                                CColor::Add(lockBreakColor, gaugeColor.WithAlphaModulatedBy(
                                                                lockBreakAlpha * visorFactor)))
                                .DepthCompareUpdate(false, false));
            modelXf.RotateLocalY(
                CRelAngle::FromRadians(gpTweakTargeting->GetChargeGaugeTickDeltaAngle()));
            gpRender->SetModelMatrix(modelXf);
          }
        }
      }
    }

    // Inner beam icon
    if (mLockOnTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mInnerBeamIcon).TryCache();
      if (CModel* const beamModel = mInnerBeamIcon.GetObject()) {
        const CColor* iconColor = &CColor::White();

        float beamScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->GetInnerIconMinRadiusViewport(),
            gpTweakTargeting->GetInnerIconMaxRadiusViewport(), mgr, mPlayerIndex);
        beamScale = beamScale * gpTweakTargeting->GetInnerBeamIconScale() *
                    (mLockOnTimer / gpTweakTargeting->GetInnerBeamIconOpenTime()) / factor;

        CMatrix3f beamMtx = rotation * CMatrix3f::Scale(beamScale);

        gpRender->SetModelMatrix(CTransform4f(lockBreakXf * beamMtx,
                                              mCurrentGroupInterpolated.GetTargetPositionWorld()));

        beamModel->Draw(
            CModelFlags::Additive(CColor::Add(lockBreakColor, iconColor->WithAlphaModulatedBy(
                                                                  lockBreakAlpha * visorFactor)))
                .DepthCompareUpdate(false, false));
      }
    }

    // Lock fire
    if (mLockFireTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mLockFire).TryCache();
      if (CModel* const fireModel = mLockFire.GetObject()) {
        CTweakTargeting* tweak = gpTweakTargeting.get();
        float lockFireFactor = mLockFireTimer / tweak->GetLockFireAnimTime();

        float fireScale = CalculateClampedScale(
            position, radius, minVpClampScale * tweak->GetLockFireMinRadiusViewport(),
            tweak->GetLockFireMaxRadiusViewport(), mgr, mPlayerIndex);
        fireScale = fireScale * gpTweakTargeting->GetLockFireReticleScale() / factor;

        CMatrix3f combined = rotation * CMatrix3f::Scale(fireScale) *
                             CMatrix3f::RotateY(CRelAngle::FromRadians(mFlowerRotationAngle));

        gpRender->SetModelMatrix(CTransform4f(lockBreakXf * combined,
                                              mCurrentGroupInterpolated.GetTargetPositionWorld()));

        fireModel->Draw(
            CModelFlags::Additive(
                CColor::Add(lockBreakColor, tweak->GetLockFireColor().WithAlphaModulatedBy(
                                                lockBreakAlpha * lockFireFactor * visorFactor)))
                .DepthCompareUpdate(false, false));
      }
    }

    // Lock dagger
    if (mLockOnTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mLockDagger).TryCache();
      if (CModel* const daggerModel = mLockDagger.GetObject()) {
        float daggerScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->GetLockDaggerMinRadiusViewport(),
            gpTweakTargeting->GetLockDaggerMaxRadiusViewport(), mgr, mPlayerIndex);
        CTweakTargeting* tweak = gpTweakTargeting.get();
        float halfDur = 0.5f * tweak->GetLockFireAnimTime();
        float t = CMath::AbsF((mLockFireTimer - halfDur) / halfDur);
        float tscale =
            (1.f - t) * tweak->GetLockDaggerScaleEnd() + t * tweak->GetLockDaggerNormalScale();
        daggerScale =
            daggerScale * tscale * (mLockOnTimer / tweak->GetInnerBeamIconOpenTime()) / factor;

        CMatrix3f daggerMtx = rotation * CMatrix3f::Scale(daggerScale);

        for (int i = 0; i < 3; ++i) {
          float ang;
          if (i == 0) {
            ang = gpTweakTargeting->GetLockDagger0Angle();
          } else if (i == 1) {
            ang = gpTweakTargeting->GetLockDagger1Angle();
          } else {
            ang = gpTweakTargeting->GetLockDagger2Angle();
          }

          CMatrix3f combined = daggerMtx * CMatrix3f::RotateY(CRelAngle::FromRadians(ang));

          gpRender->SetModelMatrix(CTransform4f(
              lockBreakXf * combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

          daggerModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->GetLockDaggerColor().WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }
  }

  if (healthMeter && mVulnerabilityTarget != kInvalidUniqueId) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mVulnerabilityTarget))) {
      if (actor->GetHealthInfo() && !close_enough(actor->GetHealthInfo()->GetInitialHP(), 0.f)) {
        float initialHealth = actor->GetHealthInfo()->GetInitialHP();
        const_cast< TCachedToken< CTexture >& >(mQuarterCurve).TryCache();
        if (CTexture* texture = mQuarterCurve.GetObject()) {
          texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
          float shadowAlpha = 0.5f * lockBreakAlpha;
          for (int pass = 0; pass < 2; ++pass) {
            float health = pass == 0 ? mTargetHealthShadow : mTargetHealth;
            if (health > initialHealth) {
              return;
            }
            float healthFactor = health / initialHealth;
            CColor color = gpTweakTargeting->GetHealthColor();
            CTweakTargeting* tweak = gpTweakTargeting.get();
            float scale = CalculateClampedScale(
                position, radius, minVpClampScale * tweak->GetLockOnConfirmMinRadiusViewport(),
                tweak->GetLockOnConfirmMaxRadiusViewport(), mgr, mPlayerIndex);
            scale = scale * gpTweakTargeting->GetLockOnConfirmReticleScale() / factor;
            CMatrix3f combined = rotation * CMatrix3f::Scale(scale);
            gpRender->SetModelMatrix(CTransform4f(
                lockBreakXf * combined, mCurrentGroupInterpolated.GetTargetPositionWorld()));

            int segments = static_cast< int >(CMath::CeilingF(12.f * healthFactor));
            float innerRadius = 0.75f * gpTweakTargeting->GetHealthMeterRadius();
            float outerRadius = 1.25f * gpTweakTargeting->GetHealthMeterRadius();
            CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
            CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
            CGraphics::SetCullMode(kCM_None);
            gpRender->SetBlendMode_AdditiveAlpha();
            CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
            CGraphics::StreamColor(
                color.WithAlphaModulatedBy(pass == 0 ? shadowAlpha : lockBreakAlpha));

            if (segments > 0) {
              CGraphics::StreamBegin(kP_TriangleStrip);
              float sweep = static_cast< float >(2.0 * M_PI) * healthFactor;
              for (int i = 0; i < segments + 1; ++i) {
                float angle = static_cast< float >(i) * sweep / static_cast< float >(segments);
                float textureAngle = 0.01f;
                if (i & 1) {
                  textureAngle += sweep / static_cast< float >(segments);
                }
                float innerU = CMath::AbsF(0.6f * CMath::FastSinR(textureAngle));
                float innerV = CMath::AbsF(2.f * (0.6f * CMath::FastCosR(textureAngle)));
                float outerU = CMath::AbsF(CMath::FastSinR(textureAngle));
                float outerV = CMath::AbsF(CMath::FastCosR(textureAngle));
                CGraphics::StreamTexcoord(2.f * innerU, innerV);
                float innerZ = innerRadius * CMath::FastCosR(angle);
                CGraphics::StreamVertex(innerRadius * CMath::FastSinR(angle), 0.f, innerZ);
                CGraphics::StreamTexcoord(2.f * outerU, 2.f * outerV);
                float outerZ = outerRadius * CMath::FastCosR(angle);
                CGraphics::StreamVertex(outerRadius * CMath::FastSinR(angle), 0.f, outerZ);
              }
              CGraphics::StreamEnd();
              CGraphics::SetCullMode(kCM_Front);
              CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
            }
          }
        }
      }
    }
  }
}

void CCompoundTargetReticle::DrawSeeker(const CMatrix3f& rotation, const CStateManager& mgr) const {
  if (mNoDrawTicks > 0) {
    return;
  }

  CVector3f position = mNextGroupInterpolated.GetTargetPositionWorld();
  float radius = mNextGroupInterpolated.GetRadiusWorld();
  float factor = mNextGroupInterpolated.GetFactor();
  bool scanReticle = false;
  switch (mPreviousState) {
  case kRS_Combat:
    break;
  case kRS_Scan:
    scanReticle = true;
    break;
  default:
    break;
  }

  float minimumScale = mNextGroupInterpolated.GetMinViewportClampScale();
  float visorFactor = mgr.GetPlayerState(mPlayerIndex)->GetVisorTransitionFactor();
  if (scanReticle && visorFactor > 0.f) {
    const_cast< TCachedToken< CModel >& >(mSeeker).TryCache();
    if (CModel* const model = mSeeker.GetObject()) {
      CTweakTargeting* tweak = gpTweakTargeting.get();
      float scale = CalculateClampedScale(position, radius,
                                          minimumScale * tweak->GetSeekerMinRadiusViewport(),
                                          tweak->GetSeekerMaxRadiusViewport(), mgr, mPlayerIndex);
      CColor color = gpTweakTargeting->GetSeekerReticleColor();
      scale *= gpTweakTargeting->GetSeekerTargetReticleScale();
      CMatrix3f combined = rotation *
                           CMatrix3f::RotateY(CRelAngle::FromRadians(mSeekerRotationAngle)) *
                           CMatrix3f::Scale(scale);
      gpRender->SetModelMatrix(
          CTransform4f(combined, mNextGroupInterpolated.GetTargetPositionWorld()));
      model->Draw(CModelFlags::Additive(color.WithAlphaModulatedBy(factor))
                      .DepthCompareUpdate(false, false));
    }
  }

  if (factor > 0.f) {
    const_cast< TCachedToken< CModel >& >(mSeeker).TryCache();
    if (CModel* const model = mSeeker.GetObject()) {
      CTweakTargeting* tweak = gpTweakTargeting.get();
      float scale = CalculateClampedScale(position, radius,
                                          minimumScale * tweak->GetSeekerMinRadiusViewport(),
                                          tweak->GetSeekerMaxRadiusViewport(), mgr, mPlayerIndex);
      CColor color = gpTweakTargeting->GetSeekerReticleColor();
      scale *= gpTweakTargeting->GetSeekerTargetReticleScale();
      CMatrix3f combined = rotation *
                           CMatrix3f::RotateY(CRelAngle::FromRadians(mSeekerRotationAngle)) *
                           CMatrix3f::Scale(scale);
      gpRender->SetModelMatrix(
          CTransform4f(combined, mNextGroupInterpolated.GetTargetPositionWorld()));
      model->Draw(CModelFlags::Additive(color.WithAlphaModulatedBy(factor))
                      .DepthCompareUpdate(false, false));
    }
  }
}

void CCompoundTargetReticle::DrawCrosshairs(const CMatrix3f& rotation,
                                            const CStateManager& mgr) const {
  if (mNoDrawTicks <= 0 && mCrosshairsDrawScale > 0.f) {
    const_cast< TCachedToken< CModel >& >(mCrosshairs).TryCache();
    CModel* const model = mCrosshairs.GetObject();
    if (model == nullptr) {
      return;
    }

    const CColor& color = gpTweakTargeting->GetCrosshairsColor();
    gpRender->SetModelMatrix(CTransform4f(rotation, mTargetPosition) *
                             CTransform4f::Scale(mCrosshairsDrawScale));
    model->Draw(CModelFlags::Additive(color.WithAlphaModulatedBy(mCrosshairsDrawScale))
                    .DepthCompareUpdate(false, false));
  }
}

void CCompoundTargetReticle::DrawScanTargetGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  if (mNoDrawTicks > 0) {
    return;
  }

  switch (mgr.GetPlayerState(mPlayerIndex)->GetCurrentVisor()) {
  case CPlayerState::kPV_Scan:
    break;
  default:
    return;
  }
  if (mPreviousState != kRS_Scan) {
    return;
  }

  const_cast< TCachedToken< CModel >& >(mScanTargetCenter).TryCache();
  const_cast< TCachedToken< CModel >& >(mScanTargetLeft).TryCache();
  const_cast< TCachedToken< CModel >& >(mScanTargetRight).TryCache();
  CModel* const center = mScanTargetCenter.GetObject();
  CModel* const left = mScanTargetLeft.GetObject();
  CModel* const right = mScanTargetRight.GetObject();
  if (center != nullptr && left != nullptr && right != nullptr) {

    const CColor& crosshairColor = gpTweakTargeting->GetScanLockCrossHairColor();
    float factor = mScanTargetFactor * skScanLockLayoutScale[mgr.GetViewportLayoutIndex()];
    gpRender->SetModelMatrix(CTransform4f(rotation, mTargetPosition) *
                             CTransform4f::Scale(factor * gpTweakTargeting->GetScanLockScale()));
    center->Draw(CModelFlags::Additive(crosshairColor.WithAlphaModulatedBy(factor))
                     .DepthCompareUpdate(false, false));

    CTweakTargeting* tweak = gpTweakTargeting.get();
    CColor bracketColor = CColor::Lerp(tweak->GetScanLockUnlockedColor(),
                                       tweak->GetScanLockLockedColor(), mScanBracketFactor);
    tweak = gpTweakTargeting.get();
    gpRender->SetModelMatrix(
        CTransform4f(rotation, mTargetPosition) *
        CTransform4f::Scale(factor * tweak->GetScanLockScale()) *
        CTransform4f::Translate((1.f - mScanBracketFactor) * -tweak->GetScanLockTranslation(), 0.f,
                                0.f));
    left->Draw(CModelFlags::Additive(bracketColor.WithAlphaModulatedBy(factor))
                   .DepthCompareUpdate(false, false));

    tweak = gpTweakTargeting.get();
    gpRender->SetModelMatrix(
        CTransform4f(rotation, mTargetPosition) *
        CTransform4f::Scale(factor * tweak->GetScanLockScale()) *
        CTransform4f::Translate((1.f - mScanBracketFactor) * tweak->GetScanLockTranslation(), 0.f,
                                0.f));
    right->Draw(CModelFlags::Additive(bracketColor.WithAlphaModulatedBy(factor))
                    .DepthCompareUpdate(false, false));
  }
}

void CCompoundTargetReticle::DrawNextLockOnGroup(const CMatrix3f& rotation,
                                                 const CStateManager& mgr) const {
  CColor seekingColor(static_cast< uchar >(244), static_cast< uchar >(103), static_cast< uchar >(5),
                      static_cast< uchar >(179));
  CColor unusedColor(0.f, 1.f, 0.7f, 1.f);
  CColor lockedColor(1.f, 0.5f, 0.5f, 1.f);
  CColor white = CColor::White();

  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  const CPlayerGun* gun = player->GetPlayerGun();
  float lockedTime = gun->GetAllSeekersLockedTime() - 0.2f;
  float chargeFactor = gun->GetSeekerChargeFactor();
  int maximumTargets = gun->GetMaxSeekerTargets();
  if (chargeFactor > 0.75f) {
    const_cast< TCachedToken< CModel >& >(mSeekerMissileCrosshair).TryCache();
    if (CModel* const crosshair = mSeekerMissileCrosshair.GetObject()) {
      float factor = (chargeFactor - 0.75f) / 0.25f;
      gpRender->SetModelMatrix(CTransform4f(rotation, mTargetPosition) *
                               CTransform4f::RotateY(CRelAngle::FromRadians(mSeekerRotationAngle)) *
                               CTransform4f::Scale(0.25f * factor));
      crosshair->Draw(CModelFlags::Additive(seekingColor.WithAlphaModulatedBy(factor))
                          .DepthCompareUpdate(false, false));
    }
  }

  const rstl::reserved_vector< rstl::pair< TUniqueId, float >, 5 >& targets =
      player->GetPlayerGun()->GetSeekerTargets();
  if (targets.empty()) {
    return;
  }

  bool allLocked = targets.size() == maximumTargets;
  CColor targetColor = seekingColor;
  if (allLocked) {
    for (int i = 0; i < targets.size(); ++i) {
      if (targets[i].second < 0.2f) {
        allLocked = false;
        break;
      }
    }
    if (allLocked) {
      if (lockedTime <= 0.1f) {
        targetColor = CColor::Lerp(seekingColor, white, lockedTime / 0.1f);
      } else {
        targetColor =
            CColor::Lerp(white, lockedColor, rstl::min_val((lockedTime - 0.1f) / 0.1f, 1.f));
      }
    }
  }

  const_cast< TCachedToken< CModel >& >(mSeekerMissileLockConfirm).TryCache();
  CModel* const confirm = mSeekerMissileLockConfirm.GetObject();
  if (confirm == nullptr) {
    return;
  }

  rstl::reserved_vector< int, 5 > counts(maximumTargets, 1);
  for (int i = 0; i < targets.size(); ++i) {
    bool first = true;
    for (int j = 0; j < i; ++j) {
      if (targets[i].first == targets[j].first) {
        counts[i] = 0;
        first = false;
        break;
      }
    }
    if (first) {
      for (int j = i + 1; j < targets.size(); ++j) {
        if (targets[i].first == targets[j].first) {
          ++counts[i];
        }
      }
    }
  }

  for (int i = 0; i < targets.size(); ++i) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(targets[i].first))) {
      bool paint = counts[i] != 0;
      bool drawConfirm = targets[i].second <= 0.2f || paint;
      if (!paint && !drawConfirm) {
        continue;
      }

      CVector3f position = CalculatePositionWorld(*actor, mgr);
      float radius = CalculateRadiusWorld(*actor, mgr);
      CTweakTargeting* tweak = gpTweakTargeting.get();
      float scale = CalculateClampedScale(position, radius, tweak->GetSeekerMinRadiusViewport(),
                                          tweak->GetSeekerMaxRadiusViewport(), mgr, mPlayerIndex);
      float factor = CMath::Clamp(1.2f, 8.f - 34.f * targets[i].second, 8.f);
      scale = factor * scale * gpTweakTargeting->GetSeekerTargetReticleScale();
      if (drawConfirm) {
        CMatrix3f combined = rotation *
                             CMatrix3f::RotateY(CRelAngle::FromRadians(mSeekerRotationAngle)) *
                             CMatrix3f::Scale(scale);
        gpRender->SetModelMatrix(CTransform4f(combined, position));
        float alpha = CMath::Clamp(0.f, 3.5f * targets[i].second, 0.7f);
        confirm->Draw(
            CModelFlags::Additive((allLocked ? targetColor : seekingColor).WithAlphaOf(alpha))
                .DepthCompareUpdate(false, false));
      }

      const_cast< TCachedToken< CTexture >& >(mRadarPaintFirst).TryCache();
      CTexture* texture = mRadarPaintFirst.GetObject();
      if (paint && texture != nullptr) {
        texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        int count = counts[i];
        float angleDelta = 360.f / static_cast< float >(count);
        float baseAngle = 270.f + 180.f / static_cast< float >(count);
        for (int j = 0; j < count; ++j) {
          float x = count == 1
                        ? 0.f
                        : 0.5f * CMath::FastCosR(
                                     M_PIF *
                                     ((baseAngle + angleDelta * static_cast< float >(j)) / 180.f));
          float z = count == 1
                        ? 0.f
                        : 0.5f * CMath::FastSinR(
                                     M_PIF *
                                     ((baseAngle + angleDelta * static_cast< float >(j)) / 180.f));
          gpRender->SetModelMatrix(CTransform4f(rotation * CMatrix3f::Scale(scale), position));
          CGraphics::StreamBegin(kP_TriangleStrip);
          CGraphics::StreamColor(allLocked ? targetColor : seekingColor);
          CGraphics::StreamTexcoord(0.f, 1.f);
          CGraphics::StreamVertex(CVector3f(-0.1f + x, 0.f, 0.1f + z));
          CGraphics::StreamTexcoord(0.f, 0.f);
          CGraphics::StreamVertex(CVector3f(-0.1f + x, 0.f, -0.1f + z));
          CGraphics::StreamTexcoord(1.f, 1.f);
          CGraphics::StreamVertex(CVector3f(0.1f + x, 0.f, 0.1f + z));
          CGraphics::StreamTexcoord(1.f, 0.f);
          CGraphics::StreamVertex(CVector3f(0.1f + x, 0.f, -0.1f + z));
          CGraphics::StreamEnd();
        }
      }
    }
  }
}

void CCompoundTargetReticle::DrawOrbitZoneGroup(const CMatrix3f& rotation,
                                                const CStateManager& mgr) const {
  const rstl::vector< TUniqueId >& ids = mgr.GetPlayerState(mPlayerIndex)->GetIds();
  if (!ids.empty()) {
    gpRender->SetBlendMode_AdditiveAlpha();
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
  }

  for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    if (const CScriptHUDHint* hint = TCastToConstPtr< CScriptHUDHint >(mgr.GetObjectById(*it))) {
      if (CTexture* texture = hint->GetTexture()) {
        CScriptHUDHint::TTextureCoordinates coordinates = hint->GetTextureCoordinates();
        CVector3f position = hint->GetTranslation();
        float scale =
            CalculateClampedScale(position, hint->GetIconScale(), hint->GetMinScreenSize(),
                                  hint->GetMaxScreenSize(), mgr, mPlayerIndex);
        gpRender->SetModelMatrix(CTransform4f(rotation * CMatrix3f::Scale(scale), position));

        texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        CGraphics::StreamBegin(kP_TriangleStrip);
        CGraphics::StreamColor(CColor::White());
        CGraphics::StreamTexcoord(coordinates.first.GetX(), coordinates.second.GetY());
        CGraphics::StreamVertex(CVector3f(-1.f, 0.f, 1.f));
        CGraphics::StreamTexcoord(coordinates.first.GetX(), coordinates.first.GetY());
        CGraphics::StreamVertex(CVector3f(-1.f, 0.f, -1.f));
        CGraphics::StreamTexcoord(coordinates.second.GetX(), coordinates.second.GetY());
        CGraphics::StreamVertex(CVector3f(1.f, 0.f, 1.f));
        CGraphics::StreamTexcoord(coordinates.second.GetX(), coordinates.first.GetY());
        CGraphics::StreamVertex(CVector3f(1.f, 0.f, -1.f));
        CGraphics::StreamEnd();
      }
    }
  }

  if (!ids.empty()) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  }
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state,
                                                    const CStateManager& mgr) {
  if (const CActor* actor = TCastToConstPtr< CActor >(
          mgr.GetObjectListById(kOL_All).GetObjectById(state.GetTargetId()))) {
    state.SetRadiusWorld(CalculateRadiusWorld(*actor, mgr));
    CVector3f position = CalculatePositionWorld(*actor, mgr);
    state.SetTargetPositionWorld(position);
  } else if (state.GetIsOrbitZoneIdlePosition()) {
    state.SetRadiusWorld(1.f);
    state.SetTargetPositionWorld((mPreviousState == kRS_Echo || mPreviousState == kRS_Dark)
                                     ? mLaggingTargetPosition
                                     : mTargetPosition);
  }
}

float CCompoundTargetReticle::CalculateRadiusWorld(const CActor& actor,
                                                   const CStateManager& mgr) const {
  rstl::optional_object< CAABox > touchBounds = actor.GetTouchBounds();
  const CAABox& bounds =
      touchBounds.valid() ? *touchBounds
                          : CAABox(actor.GetAimPosition(mgr, 0.f), actor.GetAimPosition(mgr, 0.f));
  const CVector3f min = bounds.GetMinPoint();
  const CVector3f max = bounds.GetMaxPoint();

  float radius;
  switch (gpTweakTargeting->GetTargetRadiusMode()) {
  case 0:
    radius = rstl::min_val(max[0] - min[0], rstl::min_val(max[2] - min[2], max[1] - min[1])) * 0.5f;
    break;
  case 1:
    radius = rstl::max_val(max[0] - min[0], rstl::max_val(max[2] - min[2], max[1] - min[1])) * 0.5f;
    break;
  case 2:
  default: {
    float width = max[0] - min[0];
    float height = max[1] - min[1];
    float depth = max[2] - min[2];
    radius = (width + depth + height) * (1.f / 6.f);
    break;
  }
  }

  if (fn_80097FA8(actor)) {
    radius = 0.f;
  }
  return radius > 0.f ? radius : 1.f;
}

CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& actor,
                                                         const CStateManager& mgr) const {
  return mPreviousState == kRS_Scan ? actor.GetOrbitPosition(mgr) : actor.GetAimPosition(mgr, 0.f);
}

CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr,
                                                                    bool lag) const {
  const CGameCamera& camera = *mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true);
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  float halfFov = camera.GetFov() * 0.5f;
  float halfHeight =
      static_cast< float >(player->GetTweakPlayer()->GetOrbitZoneHeight(CPlayer::kZI_Targeting));
  float distance = 224.f / halfHeight;
  distance /= static_cast< float >(tan(halfFov * (1.f / 360.f) * (2.f * M_PIF)));

  CTransform4f cameraXf = mgr.GetCameraManager(mPlayerIndex)->GetCurrentCameraTransform(mgr, true);
  CVector3f forward = cameraXf.GetForward();
  if (lag) {
    forward = mLaggingOrientation.Transform(forward);
  }

  return cameraXf.GetTranslation() + distance * forward;
}

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId id, const CStateManager& mgr) {
  return TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(id)) != nullptr;
}

float CCompoundTargetReticle::CalculateClampedScale(CVector3f position, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr,
                                                    int playerIndex) {
  float layoutScale = skViewportLayoutScale[mgr.GetViewportLayoutIndex()];
  const float minScale = layoutScale * clampMin;
  const float maxScale = layoutScale * clampMax;
  const CCameraManager* cameraManager = mgr.GetCameraManager(playerIndex);
  const CGameCamera& camera = *cameraManager->GetCurrentCamera(mgr, true);
  CTransform4f cameraXf = cameraManager->GetCurrentCameraTransform(mgr, true);
  const CTransform4f& camXf = camera.GetTransform();
  CVector3f viewSpace = camXf.TransposeRotate(position - camXf.GetTranslation());
  float projectedX = camera.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace).GetX();
  float pixelScale = camera.GetPerspectiveMatrix()
                         .MultiplyOneOverW(viewSpace + CVector3f(scale, 0.f, 0.f))
                         .GetX() -
                     projectedX;
  pixelScale *= 640.f * layoutScale;
  return scale * (CMath::Clamp(minScale, pixelScale, maxScale) / pixelScale);
}

CTargetReticleRenderState::CTargetReticleRenderState(TUniqueId target, float radius,
                                                     CVector3f position, float factor,
                                                     float minimumViewportScale,
                                                     bool orbitZoneIdlePosition)
: mTarget(target)
, mRadius(radius)
, mPosition(position)
, mFactor(factor)
, mMinimumViewportScale(minimumViewportScale)
, mOrbitZoneIdlePosition(orbitZoneIdlePosition) {}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a,
                                                     CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t) {
  float clampedT = CMath::Clamp(0.f, t, 1.f);
  float oneMinusT = 1.f - clampedT;
  out.mRadius = oneMinusT * a.mRadius + clampedT * b.mRadius;
  out.mFactor = oneMinusT * a.mFactor + clampedT * b.mFactor;
  out.mMinimumViewportScale =
      oneMinusT * a.mMinimumViewportScale + clampedT * b.mMinimumViewportScale;
  out.mPosition = CVector3f::Lerp(a.mPosition, b.mPosition, clampedT);
  if (clampedT == 1.f) {
    out.SetTargetId(b.GetTargetId());
  } else if (clampedT == 0.f) {
    out.SetTargetId(a.GetTargetId());
  } else {
    out.SetTargetId(kInvalidUniqueId);
  }
}

CTargetingManager::CTargetingManager(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex), mTargetReticle(mgr, playerIndex), mOrbitPointMarker(playerIndex) {}

bool CTargetingManager::CheckLoadComplete() {
  return mTargetReticle.CheckLoadComplete() && mOrbitPointMarker.CheckLoadComplete();
}

void CTargetingManager::Update(float dt, const CStateManager& mgr) {
  mTargetReticle.Update(dt, mgr);
  mOrbitPointMarker.Update(dt, mgr);
}

void CTargetingManager::Draw(const CStateManager& mgr, bool hideLockOn) const {
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  mOrbitPointMarker.Draw(mgr);
  const CGameCamera& camera = *mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true);
  CTransform4f cameraXf = mgr.GetCameraManager(mPlayerIndex)->GetCurrentCameraTransform(mgr, true);
  CGraphics::SetViewPointMatrix(cameraXf);
  float height = static_cast< float >(CGraphics::GetViewport().mHeight);
  float width = static_cast< float >(CGraphics::GetViewport().mWidth);
  gpRender->SetPerspective(camera.GetFov(), width, height, camera.GetNearClipDistance(),
                           camera.GetFarClipDistance());
  mTargetReticle.Draw(mgr, hideLockOn);
}

void CCompoundTargetReticle::Touch() const {
  if (mCrosshairs.GetObject()) {
    mCrosshairs.GetObject()->Touch(0);
  }
  if (mSeeker.GetObject()) {
    mSeeker.GetObject()->Touch(0);
  }
  if (mGrapple.GetObject()) {
    mGrapple.GetObject()->Touch(0);
  }
  if (mSeekerMissileLockConfirm.GetObject()) {
    mSeekerMissileLockConfirm.GetObject()->Touch(0);
  }
  if (mSeekerMissileCrosshair.GetObject()) {
    mSeekerMissileCrosshair.GetObject()->Touch(0);
  }
  if (mTargetFlower.GetObject()) {
    mTargetFlower.GetObject()->Touch(0);
  }
  if (mMissileBracket.GetObject()) {
    mMissileBracket.GetObject()->Touch(0);
  }
  if (mInnerBeamIcon.GetObject()) {
    mInnerBeamIcon.GetObject()->Touch(0);
  }
  if (mLockFire.GetObject()) {
    mLockFire.GetObject()->Touch(0);
  }
  if (mLockDagger.GetObject()) {
    mLockDagger.GetObject()->Touch(0);
  }
  if (mGrapple.GetObject()) {
    mGrapple.GetObject()->Touch(0);
  }
  if (mChargeTickFirst.GetObject()) {
    mChargeTickFirst.GetObject()->Touch(0);
  }
  if (mChargeGauge.mModel.GetObject()) {
    mChargeGauge.mModel.GetObject()->Touch(0);
  }
  if (mScanTargetCenter.GetObject()) {
    mScanTargetCenter.GetObject()->Touch(0);
  }
  if (mScanTargetLeft.GetObject()) {
    mScanTargetLeft.GetObject()->Touch(0);
  }
  if (mScanTargetRight.GetObject()) {
    mScanTargetRight.GetObject()->Touch(0);
  }
  for (rstl::vector< SOuterItemInfo >::const_iterator it = mOuterBeamIconSquares.begin();
       it != mOuterBeamIconSquares.end(); ++it) {
    if (it->mModel.GetObject()) {
      it->mModel.GetObject()->Touch(0);
    }
  }
}

void CTargetingManager::Touch() const { mTargetReticle.Touch(); }

COrbitPointMarker::COrbitPointMarker(int playerIndex)
: mPlayerIndex(playerIndex)
, mZOffset(gpTweakTargeting->GetOrbitPointZOffset())
, mCameraRelativeZ(true)
, mLagAzimuth(0.f)
, mAzimuth(0.f)
, mLagTargetPosition(CVector3f::Zero())
, mLastFreeOrbit(false)
, mInterpolationTimer(0.f)
, mCurrentTime(0.f)
, mOrbitPointModel(gpSimplePool->GetObj("CMDL_OrbitPoint")) {
  mOrbitPointModel.Lock();
}

bool COrbitPointMarker::CheckLoadComplete() { return mOrbitPointModel.TryCache(); }

void COrbitPointMarker::Update(float dt, const CStateManager& mgr) {
  mCurrentTime += dt;
  const CPlayer* player = mgr.GetPlayer(mPlayerIndex);
  CPlayer::EPlayerOrbitState orbitState = player->GetOrbitState();
  const CGameCamera& camera = *mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true);
  bool freeOrbit = orbitState == CPlayer::kOS_OrbitPoint || orbitState == CPlayer::kOS_OrbitCarcass;

  if (mLastFreeOrbit != freeOrbit) {
    if (orbitState == CPlayer::kOS_OrbitPoint || orbitState == CPlayer::kOS_OrbitCarcass) {
      ResetInterpolationTimer(gpTweakTargeting->GetOrbitPointInterpolateInTime());
      mLagTargetPosition = !mCameraRelativeZ
                               ? player->GetHUDOrbitTargetPosition() + CVector3f(0.f, 0.f, mZOffset)
                               : CVector3f(player->GetHUDOrbitTargetPosition().GetX(),
                                           player->GetHUDOrbitTargetPosition().GetY(),
                                           mZOffset + camera.GetTranslation().GetZ());
      CEulerAngles euler =
          CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(camera.GetTransform()));
      mLagAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
    } else if (orbitState == CPlayer::kOS_NoOrbit) {
      ResetInterpolationTimer(gpTweakTargeting->GetOrbitPointInterpolateOutTime());
    } else {
      ResetInterpolationTimer(0.01f);
    }
    mLastFreeOrbit = !mLastFreeOrbit;
  }

  if (mInterpolationTimer > 0.f) {
    mInterpolationTimer = rstl::max_val(0.f, mInterpolationTimer - dt);
  }

  if (!mCameraRelativeZ) {
    CVector3f orbitPosition = player->GetHUDOrbitTargetPosition();
    float targetZ = mZOffset + orbitPosition.GetZ();
    float delta = targetZ - mLagTargetPosition.GetZ();
    if (delta < 0.1f) {
      mLagTargetPosition = orbitPosition + CVector3f(0.f, 0.f, mZOffset);
    } else if (delta < 0.f) {
      mLagTargetPosition =
          CVector3f(orbitPosition.GetX(), orbitPosition.GetY(), mLagTargetPosition.GetZ() - 0.1f);
    } else {
      mLagTargetPosition =
          CVector3f(orbitPosition.GetX(), orbitPosition.GetY(), mLagTargetPosition.GetZ() + 0.1f);
    }
  } else {
    mLagTargetPosition = CVector3f(player->GetHUDOrbitTargetPosition().GetX(),
                                   player->GetHUDOrbitTargetPosition().GetY(),
                                   mZOffset + player->GetHUDOrbitTargetPosition().GetZ());
  }

  if (mLastFreeOrbit) {
    CEulerAngles euler =
        CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(camera.GetTransform()));
    float newAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
    float delta = newAzimuth - mAzimuth;
    if (player->IsInFreeLook()) {
      mLagAzimuth += delta;
    }
    mAzimuth = newAzimuth;
  }
}

void COrbitPointMarker::Draw(const CStateManager& mgr) const {
  if ((mLastFreeOrbit || mInterpolationTimer > 0.f) && gpTweakTargeting->GetDrawOrbitPoint()) {
    const_cast< TCachedToken< CModel >& >(mOrbitPointModel).TryCache();
    if (mOrbitPointModel.GetObject() != nullptr) {
      const CGameCamera& camera = *mgr.GetCameraManager(mPlayerIndex)->GetCurrentCamera(mgr, true);
      CTransform4f cameraXf =
          mgr.GetCameraManager(mPlayerIndex)->GetCurrentCameraTransform(mgr, true);
      CGraphics::SetViewPointMatrix(cameraXf);
      float height = static_cast< float >(CGraphics::GetViewport().mHeight);
      float width = static_cast< float >(CGraphics::GetViewport().mWidth);
      gpRender->SetPerspective(camera.GetFov(), width, height, camera.GetNearClipDistance(),
                               camera.GetFarClipDistance());

      float scale;
      if (mLastFreeOrbit) {
        scale = 1.f - mInterpolationTimer / gpTweakTargeting->GetOrbitPointInterpolateInTime();
      } else {
        scale = mInterpolationTimer / gpTweakTargeting->GetOrbitPointInterpolateOutTime();
      }

      CColor color = gpTweakTargeting->GetOrbitPointModelColor();
      CTransform4f modelXf = CTransform4f::RotateZ(CRelAngle::FromRadians(mLagAzimuth));
      modelXf.ScaleBy(scale);
      modelXf.AddTranslation(mLagTargetPosition);
      gpRender->SetModelMatrix(modelXf);
      CModel* model = mOrbitPointModel.GetObject();
      CModelFlags flags =
          CModelFlags::Additive(color.WithAlphaModulatedBy(scale)).DepthCompareUpdate(false, false);
      model->Draw(flags);
    }
  }
}

void COrbitPointMarker::ResetInterpolationTimer(float time) { mInterpolationTimer = time; }
