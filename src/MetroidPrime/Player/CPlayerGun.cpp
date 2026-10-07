#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMatrix4f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWeaponMgr.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CAnnihilatorBeam.hpp"
#include "MetroidPrime/Weapons/CAuxWeapon.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CDarkBeam.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"
#include "MetroidPrime/Weapons/CLightBeam.hpp"
#include "MetroidPrime/Weapons/CPowerBeam.hpp"
#include "MetroidPrime/Weapons/CPowerBomb.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXManage.h"
#include "dolphin/types.h"
#include <math.h>

static const TStateMachineState< CPlayerGun >::STriggerFunction skGunTriggerFunctions[] = {
    {"ShouldHolster", &CPlayerGun::ShouldHolster},
    {"IsHolstered", &CPlayerGun::IsHolstered},
    {"IsNotHolstered", &CPlayerGun::IsNotHolstered},
    {"StartCharge", &CPlayerGun::StartCharge},
    {"InitiateCombo", &CPlayerGun::InitiateCombo},
    {"Discharge", &CPlayerGun::Discharge},
    {"TransitionToMorphball", &CPlayerGun::TransitionToMorphball},
    {"TransitionToPlayer", &CPlayerGun::TransitionToPlayer},
    {"AnimOver", &CPlayerGun::AnimOver},
    {"ActivateMissile", &CPlayerGun::ActivateMissile},
    {"CloseMissile", &CPlayerGun::CloseMissile},
    {"ChargeDone", &CPlayerGun::ChargeDone},
    {"ButtonRelease", &CPlayerGun::ButtonRelease},
    {"ComboOver", &CPlayerGun::ComboOver},
    {"InterruptEvent", &CPlayerGun::InterruptEvent},
    {"GunLoaded", &CPlayerGun::GunLoaded},
    {"Scanning", &CPlayerGun::Scanning},
    {"InCinematic", &CPlayerGun::InCinematic},
    {"StartFidget", &CPlayerGun::StartFidget},
    {"FidgetOver", &CPlayerGun::FidgetOver},
    {"Grappling", &CPlayerGun::Grappling},
    {"IsAlive", &CPlayerGun::IsAlive},
    {"InPhazon", &CPlayerGun::InPhazon},
};
static const TStateMachineState< CPlayerGun >::SStateFunction skGunStateFunctions[] = {
    {"Start", &CPlayerGun::Start},
    {"Main", &CPlayerGun::Main},
    {"InMorphball", &CPlayerGun::InMorphball},
    {"Charging", &CPlayerGun::Charging},
    {"Recoil", &CPlayerGun::Recoil},
    {"ComboActive", &CPlayerGun::ComboActive},
    {"Holstered", &CPlayerGun::Holstered},
    {"Fidgeting", &CPlayerGun::Fidgeting},
    {"MissileActive", &CPlayerGun::MissileActive},
    {"MissileClosing", &CPlayerGun::MissileClosing},
    {"EventHandler", &CPlayerGun::EventHandler},
};

static const float kFactorMultiplierForBeamCombo =
    1.0f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeDtFactor = 1.0f / CPlayerState::GetMissileComboChargeFactor();
static const ushort skEmptyBeamSfx[] = {
    0x524,
    0x25A4,
};

static const ushort skEmptyMissileSfx[] = {0xBF, 0x25B0};

static const CMaterialFilter skWeaponCollisionFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision));
static const CMaterialFilter skSeekerTargetFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_SeekerTarget), CMaterialList(kMT_NoPlatformCollision));

static inline bool just_froze(bool frozen, bool playerFrozen) {
  return (frozen ^ playerFrozen) & playerFrozen;
}

static inline bool just_thawed(bool frozen, bool playerFrozen) {
  return (frozen ^ playerFrozen) & frozen;
}

bool IsSeekerTargetInRange(const CActor& target, const CPlayer& player, const CStateManager& mgr,
                           float radius) {
  const CVector3f aimPosition = target.GetAimPosition(mgr, 0.f);
  return (aimPosition - player.GetEyePosition()).MagSquared() < radius * radius;
}

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr) {
  CPlayer* player = GetPlayerFromAll(mgr);
  CPlayerState* playerState = player->GetPlayerState();

  mFidget.ResetAll();

  if (playerState->GetItemAmount(CPlayerState::kIT_BeamWeaponsDisabled, true) != 0) {
    return;
  }

  CPlayerState::EItemType beamAmmoTypeA = CPlayerState::kIT_Invalid;
  CPlayerState::EItemType beamAmmoTypeB = CPlayerState::kIT_Invalid;
  int outBeamAmmoCost = 0;

  bool outOfAmmo = IsOutOfAmmoToShoot(mgr);
  if (outOfAmmo && mChargePhase != kCP_Charged) {
    if (mChargePhase != kCP_NotCharging ||
        !playerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, 0);
    }
    ResetCharge(mgr, false);
    PlayBeamFireSfx(mgr, *player, false);
    PlayAnim(mgr, 0, 0);
  } else if (outOfAmmo ||
             GetBeamAmmoTypeAndCosts(false, mgr, beamAmmoTypeA, beamAmmoTypeB, outBeamAmmoCost)) {
    CPlayerState::EChargeStage chargeState = outOfAmmo ? CPlayerState::kCS_Normal : mChargeState;
    float chargeFactor1 = playerState->GetChargeBeamFactor();
    if (!outOfAmmo && mAbsorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
      outBeamAmmoCost = 0;
    }
    if (beamAmmoTypeA != CPlayerState::kIT_Invalid) {
      playerState->DecrementAmmoAndDisplayAlertIfOut(mgr, beamAmmoTypeA, outBeamAmmoCost);
      if (beamAmmoTypeB != CPlayerState::kIT_Invalid) {
        playerState->DecrementAmmoAndDisplayAlertIfOut(mgr, beamAmmoTypeB, outBeamAmmoCost);
      }
    }

    mChargeEffectVisible = mChargePhase == kCP_NotCharging;
    ++mRapidFireShots;
    const bool targetHoming = mCurrentBeam->GetVelocityInfo().GetTargetHoming(int(chargeState));
    CTransform4f xf(mPointBlankWorldSurface ? mElbowWorldXf : mGunWorldXf * mBeamLocalXf);
    if (!mPointBlankWorldSurface && mGunStrikeCooldownTimer <= 0.f) {
      const CVector3f position = xf.GetTranslation();
      xf = mAssistAimXf;
      xf.SetTranslation(position);
    }
    xf.AddTranslation(player->GetCameraManager()->GetGlobalCameraTranslation(mgr, true));
    switch (mCurrentBeamId) {
    case CPlayerState::kBI_Dark:
    case CPlayerState::kBI_Light:
      mMuzzleEffectVisTimer = 0.5f;
      break;
    default:
      mMuzzleEffectVisTimer = 0.0625f;
      break;
    }
    if (outOfAmmo) {
      chargeFactor1 *= 0.5f;
    }
    const uint attributes = outOfAmmo ? (1 << 3) : 0;

    if (mAbsorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
      TCachedToken< CWeaponDescription > phazonBallToken(gpSimplePool->GetObj("PhazonBall"), true);

      TUniqueId homingTarget = targetHoming ? GetTargetId(mgr) : kInvalidUniqueId;
      mCurrentBeam->CGunWeapon::Fire(phazonBallToken, mUnderwater, dt, chargeState, xf, mgr,
                                     homingTarget, attributes, 0x1c4, nullptr, nullptr,
                                     chargeFactor1, chargeFactor1);

    } else {
      const TUniqueId homingTarget = targetHoming ? GetTargetId(mgr) : kInvalidUniqueId;
      mCurrentBeam->Fire(mCurrentBeam->GetProjectileToken(chargeState), mUnderwater, dt,
                         chargeState, xf, mgr, homingTarget, attributes,
                         CSfxManager::kInternalInvalidSfxId, nullptr, nullptr, chargeFactor1,
                         chargeFactor1);
    }

    mgr.InformListeners(mGunWorldXf.GetTranslation(), kLNT_PlayerFire);

    bool resetCharge = false;
    mCooldown = mCurrentBeam->GetWeaponInfo().mCoolDown;
    if (mChargePhase == kCP_ChargeFx || mChargePhase == kCP_Charged) {
      resetCharge = true;
    }
    if (!resetCharge && mgr.IsMultiplayer()) {
      GetPlayerFromAll(mgr)->EmitMultiplayerBeamParticles(mgr);
    }
    if (resetCharge) {
      ResetCharge(mgr, false);
    }

    if (playerState->GetItemAmount(CPlayerState::kIT_DoubleDamage, true)) {
      PlaySfxForPlayer(GetPlayer(mgr), 0x2612, mSoundVolume, mgr.GetNextAreaId().Value(),
                       mUnderwater, 0);
    }
    mFiredWeaponFlags |= resetCharge ? 4 : 1;
  } else {
    PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                     mgr.GetNextAreaId().Value(), mUnderwater, 0);
  }
}
void CPlayerGun::UpdateChargeState(float dt, CStateManager& mgr) {

  CPlayerState* playerState = GetPlayerFromAll(mgr)->GetPlayerState();

  switch (mChargePhase) {
  case kCP_Charged:
    mChargeRumbleTimer += dt;
    if (mChargeRumbleTimer >= 5.0f) {
      mChargeRumbleTimer = 0.0f;
      CRumbleManager* rumbleMgr = mgr.RumbleManager(mgr.MaskUIdNumPlayers(GetPlayerUniqueId()));
      if (mChargeRumbleHandle == -1) {
        rumbleMgr->StopRumble(mChargeRumbleHandle);
        mChargeRumbleHandle = -1;
      }
      mChargeRumbleHandle = rumbleMgr->Rumble(mgr, kRFX_PlayerGunCharge, 1.f, kRP_Three);
    }
    break;
  default:
    mChargeRumbleTimer = 0.0f;
  }

  if (mChargePhase != kCP_NotCharging) {
    switch (mChargePhase) {
    case kCP_ChargeRequested:
      if (playerState->GetChargeBeamFactor() > playerState->GetChargeAnimStart()) {
        mChargePhase = kCP_Charging;
      }
      break;
    }
    if (mChargeSfx && mSeekerChargeState != kSCS_FullyCharged) {
      CSfxManager::PitchBend(mChargeSfx, mUnderwater ? 0 : 0x2000);
    }
    if (kCP_NotCharging < mChargePhase && mChargePhase < kCP_Charged) {
      playerState->IncrementChargeBeamFactor(kChargeDtFactor * dt);
    }
  } else {
    if (playerState->GetChargeBeamFactor() > 0.0f) {
      playerState->IncrementChargeBeamFactor(-dt);
    }
  }
}

void CPlayerGun::Charging(CStateManager& mgr, int param, float dt) {
  switch (param) {
  case 0:
    PlayAnim(mgr, 1, 0);
    StopChargeSound(mgr, true);
    break;

  case 1: {
    CPlayerState* playerState = GetPlayer(mgr)->GetPlayerState();
    float factor = IsOutOfAmmoToShoot(mgr) ? 0.5f : 1.f;
    switch (mChargePhase) {
    case kCP_Charging:
      if (playerState->GetChargeBeamFactor() >= kFactorMultiplierForBeamCombo * factor) {
        mChargeEffectVisible = true;
        mChargePhase = kCP_ChargeFx;
        mChargeState = CPlayerState::kCS_Charged;
        EnableChargeFx(mgr, true);
        PlayAnim(mgr, 2, 1);
      }
      break;
    case kCP_ChargeFx:
      if (playerState->GetChargeBeamFactor() >= factor) {
        mChargePhase = kCP_Charged;
        break;
      }
    }
    break;
  }

  case 2:
    if (mInterruptEvent) {
      ResetCharge(mgr, false);
      if (mBeamChangeState == kBCS_Idle) {
        PlayAnim(mgr, 0, 0);
      } else {
        mRequestReturnToDefault = true;
      }
    }
    break;
  }
}

bool CPlayerGun::ShouldHolster(CStateManager& mgr, const float& argument) {
  CPlayer* player = GetPlayer(mgr);
  const CPlayer::EPlayerMorphBallState state = player->GetMorphballTransitionState();
  return player->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
         state == CPlayer::kMS_Morphing || state == CPlayer::kMS_Morphed ||
         !player->GetPlayerState()->IsPlayerAlive() ||
         player->GetCameraManager()->IsInCinematicCamera();
}

bool CPlayerGun::IsHolstered(CStateManager& mgr, const float& argument) {
  return mGunHolsterState == kGHS_Holstered;
}

bool CPlayerGun::IsNotHolstered(CStateManager& mgr, const float& argument) {
  return mGunHolsterState == kGHS_Drawn;
}

bool CPlayerGun::StartCharge(CStateManager& mgr, const float& argument) {
  CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
  if (!mInBigStrike && mGunHolsterState == kGHS_Drawn && mChargePhase == kCP_Charging &&
      state->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    int cost = 0;
    CPlayerState::EItemType ammoA = CPlayerState::kIT_Invalid;
    CPlayerState::EItemType ammoB = CPlayerState::kIT_Invalid;
    if (GetBeamAmmoTypeAndCosts(false, mgr, ammoA, ammoB, cost)) {
      return true;
    }
    if (IsOutOfAmmoToShoot(mgr)) {
      return true;
    }
  }
  return false;
}

bool CPlayerGun::InitiateCombo(CStateManager& mgr, const float& argument) {
  CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
  if (state->HasPowerUp(CPlayerState::kIT_Missile) && (mPressedInputFlags & 2) != 0 &&
      mChargePhase == kCP_Charged) {
    const bool canCombo = mAbsorbedPhazonShots != gpTweakPlayerGun->GetMaxAbsorbedPhazonShots();
    if (mAuxWeapon->HasChargeCombo(mCurrentBeamId, mgr) &&
        state->GetItemAmount(CPlayerState::kIT_Missile, true) >=
            state->GetMissileCostForAltAttack() &&
        canCombo) {
      CPlayerState::EItemType ammoA = CPlayerState::EItemType(-1);
      CPlayerState::EItemType ammoB = CPlayerState::EItemType(-1);
      int cost = 0;
      if (GetBeamAmmoTypeAndCosts(true, mgr, ammoA, ammoB, cost)) {
        return true;
      }
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
      return false;
    }
    PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                     mgr.GetNextAreaId().Value(), mUnderwater, false);
  }
  return false;
}

bool CPlayerGun::Discharge(CStateManager& mgr, const float& argument) { return false; }

bool CPlayerGun::TransitionToMorphball(CStateManager& mgr, const float& argument) {
  const CPlayer::EPlayerMorphBallState state = GetPlayer(mgr)->GetMorphballTransitionState();
  return state == CPlayer::kMS_Morphed || state == CPlayer::kMS_Morphing;
}

bool CPlayerGun::TransitionToPlayer(CStateManager& mgr, const float& argument) {
  const CPlayer::EPlayerMorphBallState state = GetPlayer(mgr)->GetMorphballTransitionState();
  return state == CPlayer::kMS_Unmorphing || state == CPlayer::kMS_Unmorphed;
}

bool CPlayerGun::AnimOver(CStateManager& mgr, const float& argument) {
  if (mMissileCloseAnimDone) {
    mMissileCloseAnimDone = false;
    return true;
  }
  return !mCurrentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(
      0.001f, rstl::string("Whole Body"));
}

bool CPlayerGun::ActivateMissile(CStateManager& mgr, const float& argument) {
  bool pressed;
  if ((mSecondaryCooldown > 0.f) == false &&
      ((pressed = (mPressedInputFlags & 2) != 0) || (mInputFlags & 2) != 0)) {
    CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
    if (mgr.IsMultiplayer() && state->HasPowerUp(CPlayerState::kIT_SuperMissile)) {
      return true;
    }
    if (state->HasPowerUp(CPlayerState::kIT_Missile) &&
        state->GetItemAmount(CPlayerState::kIT_Missile, true) > 0) {
      return true;
    }
    if (pressed) {
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
    }
  }
  return false;
}

bool CPlayerGun::CloseMissile(CStateManager& mgr, const float& argument) {
  return (mPressedInputFlags & 0xd) != 0 || !(mMissileExitTimer > 0.f);
}

bool CPlayerGun::ChargeDone(CStateManager& mgr, const float& argument) {
  return mChargePhase == kCP_ChargeDone || mCurrentBeam->IsChargeAnimOver();
}

bool CPlayerGun::ButtonRelease(CStateManager& mgr, const float& argument) {
  return (mReleasedInputFlags & 4) != 0;
}

bool CPlayerGun::ComboOver(CStateManager& mgr, const float& argument) {
  return mChargePhase == kCP_ComboFired && AnimOver(mgr, argument);
}

bool CPlayerGun::InterruptEvent(CStateManager& mgr, const float& argument) {
  if (!mInterruptEvent) {
    mInterruptEvent = ShouldHolster(mgr, argument);
  }
  return mInterruptEvent;
}

bool CPlayerGun::GunLoaded(CStateManager& mgr, const float& argument) {
  return mBeamChangeState == kBCS_Idle;
}

bool CPlayerGun::Scanning(CStateManager& mgr, const float& argument) {
  return GetPlayer(mgr)->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan;
}

bool CPlayerGun::InCinematic(CStateManager& mgr, const float& argument) {
  return GetPlayer(mgr)->GetCameraManager()->IsInCinematicCamera();
}

bool CPlayerGun::StartFidget(CStateManager& mgr, const float& argument) {
  if (mgr.IsMultiplayer()) {
    return false;
  }
  return mFidget.GetState() != CFidget::kS_NoFidget;
}

bool CPlayerGun::FidgetOver(CStateManager& mgr, const float& argument) {
  CPlayer* player = GetPlayer(mgr);
  mCurrentBeam->SetSpeedUpAnimation(mInputFlags != 0 || mDamageAmount > 0.f);
  return mInputFlags != 0 || player->IsInFreeLook() ||
         player->GetVelocityWR().Magnitude() > 0.01f ||
         player->GetAngularVelocityOR().GetVector().GetZ() != 0.f ||
         mMotionState.mMotionState != CMotionState::kMS_Zero || mDamageAmount > 0.f ||
         mFidget.GetState() == CFidget::kS_NoFidget;
}

bool CPlayerGun::Grappling(CStateManager& mgr, const float& argument) {
  return mGrappleArm->IsGrappling();
}

bool CPlayerGun::IsAlive(CStateManager& mgr, const float& argument) {
  return GetPlayer(mgr)->GetPlayerState()->IsPlayerAlive();
}

bool CPlayerGun::InPhazon(CStateManager& mgr, const float& argument) { return false; }

void CPlayerGun::Start(CStateManager& mgr, int message, float dt) {}

void CPlayerGun::Main(CStateManager& mgr, int message, float dt) {
  CPlayerState* state = GetPlayerFromAll(mgr)->GetPlayerState();
  switch (message) {
  case kSM_Enter:
    break;
  case kSM_Update: {
    CPlayer* player = GetPlayer(mgr);
    if ((mReleasedInputFlags & 0xd) != 0 && (mReleasedInputFlags & 4) != 0 &&
        state->ItemEnabled(CPlayerState::kIT_ChargeBeam) && mChargePhase != kCP_NotCharging) {
      ResetCharge(mgr, false);
      if (mMuzzleEffectVisTimer <= 0.f && IsOutOfAmmoToShoot(mgr)) {
        // The shared denial sound is also used when releasing an empty charged shot.
        PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                         mgr.GetNextAreaId().Value(), mUnderwater, 0);
      }
    }
    if ((mCooldown > 0.f) == false && mChargePhase == kCP_NotCharging && !mInBigStrike) {
      const bool canCharge = state->ItemEnabled(CPlayerState::kIT_ChargeBeam);
      const uint firePressed = mPressedInputFlags & 1;
      if (!mRequestReturnToDefault && (firePressed != 0 || (mInputFlags & 8) != 0)) {
        UpdateNormalShotCycle(dt, mgr);
        mFiring = (mChargePhase == kCP_NotCharging || mChargePhase == kCP_ChargeRequested) &&
                  !player->IsInFreeLook();
      }
      if (canCharge && (mInputFlags & 4) != 0) {
        mChargePhase = kCP_ChargeRequested;
        mFiring = false;
        state->SetChargeBeamFactor(0.f);
      }
    }
    if (!mgr.IsMultiplayer()) {
      UpdateGunIdle(dt, mgr);
    }
    break;
  }
  case kSM_Exit:
    break;
  }
}

void CPlayerGun::InMorphball(CStateManager& mgr, int message, float dt) {
  switch (message) {
  case kSM_Enter:
    mBombDependencies.Lock();
    break;
  case kSM_Update: {
    CPlayer* player = GetPlayer(mgr);
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
        !player->GetMorphBall()->InScrewAttackMode()) {
      FireBombs(mgr);
    }
    break;
  }
  case kSM_Exit:
    mBombDependencies.Unlock();
    break;
  }
}

void CPlayerGun::Recoil(CStateManager& mgr, int message, float dt) {
  switch (message) {
  case kSM_Enter:
    EnableChargeFx(mgr, false);
    switch (mChargePhase) {
    case kCP_Charging:
      mChargePhase = kCP_ChargeDone;
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, 0);
      break;
    case kCP_ChargeFx:
    case kCP_Charged:
      UpdateNormalShotCycle(dt, mgr);
      StopChargeSound(mgr, false);
      break;
    }
    break;
  case kSM_Exit:
    if (mChargePhase == kCP_ChargeDone) {
      PlayAnim(mgr, NWeaponTypes::kGAT_BasePosition, false);
    }
    if (mSeekerChargeState == kSCS_Done) {
      PlayAnim(mgr, NWeaponTypes::kGAT_BasePosition, false);
    }
    ResetCharge(mgr, false);
    break;
  }
}

void CPlayerGun::ComboActive(CStateManager& mgr, int message, float dt) {
  static const ushort sounds[2][4] = {{0xbe, 0xbe, 0xbe, 0x1fde}, {0x25a1, 0x25a1, 0x25a1, 0x25a1}};
  switch (message) {
  case kSM_Enter: {
    mComboFiring = true;
    TCachedToken< CGenDescription >& transfer = mCurrentBeam->GetTransferEffect();
    if (transfer.TryCache()) {
      mComboTransferGenerator = rstl::auto_ptr< CElementGen >(rs_new CElementGen(transfer));
      mComboTransferGenerator->SetGlobalOrientation(CTransform4f::Identity());
    }
    mCurrentBeam->SetEnableCharge(true);
    StopChargeSound(mgr, false);
    PlaySfxForPlayer(GetPlayer(mgr), sounds[mSoundSetIndex][mCurrentBeamId], mSoundVolume,
                     mgr.GetNextAreaId().Value(), mUnderwater, false);
    mChargePhase = kCP_ComboTransfer;
    break;
  }
  case kSM_Update:
    if (mComboTransferGenerator.get() != nullptr) {
      mComboTransferGenerator->SetGlobalTranslation(mBeamLocalXf.GetTranslation());
      mComboTransferGenerator->SetOrientation(mBeamLocalXf.GetRotation());
      mComboTransferGenerator->Update(dt);
    }
    switch (mChargePhase) {
    case kCP_ComboTransfer:
      if (mComboTransferFactor < 1.f) {
        mComboTransferFactor += 4.f * dt;
      } else {
        mComboTransferFactor = 1.f;
        mChargePhase = kCP_ComboTransferDone;
        mChargeEffectVisible = false;
      }
      break;
    case kCP_ComboTransferDone:
      mChargePhase = kCP_ComboRequested;
      break;
    case kCP_ComboRequested:
      if (!mGrappleArm->IsGrappleBeamActive()) {
        mGrappleArm->SetStateFlags(CGrappleArm::kSF_ComboFire);
      }
      mGunMotion->PlayPasAnim(SamusGun::kAS_ComboFire, mgr, 0.f, false);
      mCurrentBeam->EnterComboFire(mgr);
      mChargePhase = kCP_ComboAnimating;
      break;
    }
    break;
  case kSM_Exit:
    if (mInterruptEvent &&
        (mChargePhase == kCP_ComboTransfer || mChargePhase == kCP_ComboTransferDone)) {
      PlayAnim(mgr, NWeaponTypes::kGAT_BasePosition, false);
    }
    mComboTransferFactor = 0.f;
    mComboFiring = false;
    StopContinuousBeam(mgr, mBeamChangeState != kBCS_Idle);
    ResetCharge(mgr, false);
    mRequestReturnToDefault = mBeamChangeState != kBCS_Idle;
    break;
  }
}

void CPlayerGun::Holstered(CStateManager& mgr, int message, float dt) {}

void CPlayerGun::Fidgeting(CStateManager& mgr, int message, float dt) {
  const CFidget::EState state = mFidget.GetState();
  switch (message) {
  case kSM_Enter:
    mGunMotion->BasePosition(false);
    mGunMotionState = SamusGun::kAS_BasePosition;
    AsyncLoadFidget(mgr);
    break;
  case kSM_Update:
    if (state == CFidget::kS_Loading) {
      if (IsFidgetLoaded()) {
        EnterFidget(mgr);
      }
    } else if (state == CFidget::kS_MinorFidget || state == CFidget::kS_MajorFidget) {
      mAnimPlaying =
          mGunMotionFidgeting
              ? mGunMotion->IsAnimPlaying()
              : mCurrentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(
                    0.001f, rstl::string("Whole Body"));
      if (!mAnimPlaying) {
        mFidget.ResetState();
      }
    }
    break;
  case kSM_Exit:
    switch (state) {
    case CFidget::kS_NoFidget:
      mFidget.ResetState();
      break;
    case CFidget::kS_Loading:
      UnLoadFidget();
      // Fall through to reset the completed or interrupted animation.
    case CFidget::kS_MinorFidget:
    case CFidget::kS_MajorFidget:
    case CFidget::kS_HolsterBeam:
      mFidget.ResetAll();
      mCurrentBeam->SetSpecialAnimationPlaying(true);
      break;
    }
    mAnimPlaying = false;
    if (mBeamChangeState == kBCS_Idle) {
      ReturnArmAndGunToDefault(mgr, true);
      PlayAnim(mgr, NWeaponTypes::kGAT_BasePosition, false);
    }
    if (state != CFidget::kS_NoFidget) {
      GetPlayerFromAll(mgr)->CameraBobObject()->SetState(CPlayerCameraBob::kCBS_Walk, mgr);
    }
    break;
  }
}

void CPlayerGun::MissileActive(CStateManager& mgr, int message, float dt) {
  switch (message) {
  case kSM_Enter:
    if (mChargePhase != kCP_NotCharging) {
      ResetCharge(mgr, false);
    }
    mMissileState = kMS_Ready;
    mMissileMode = true;
  case kSM_Update:
    UpdateSeeker(dt, mgr);
    break;
  case kSM_Exit:
    mMissileAnimActive = false;
    ResetSeeker(mgr);
    mMissileExitTimer = 7.f;
    if (mInterruptEvent) {
      if (mBeamChangeState == kBCS_Idle) {
        PlayAnim(mgr, NWeaponTypes::kGAT_BasePosition, false);
      }
      mMissileMode = false;
    }
    break;
  }
}

void CPlayerGun::MissileClosing(CStateManager& mgr, int message, float dt) {
  switch (message) {
  case kSM_Enter:
    PlayAnim(mgr, NWeaponTypes::kGAT_FromMissile, false);
    break;
  case kSM_Update:
    if (mCurrentBeam->GetSolidModelData().GetAnimationData()->GetAnimTimeRemaining(
            rstl::string("Whole Body")) < 0.001) {
      mMissileCloseAnimDone = true;
    }
    break;
  case kSM_Exit:
    mMissileCloseAnimDone = false;
    mMissileState = kMS_Inactive;
    mMissileMode = false;
    if (!mInterruptEvent &&
        GetPlayer(mgr)->GetPlayerState()->ItemEnabled(CPlayerState::kIT_ChargeBeam) &&
        (mInputFlags & 4) != 0) {
      mChargePhase = kCP_ChargeRequested;
    }
    break;
  }
}

void CPlayerGun::EventHandler(CStateManager& mgr, int message, float dt) {
  switch (message) {
  case kSM_Enter:
    mInterruptEvent = false;
    if (mChargePhase != kCP_NotCharging) {
      ResetCharge(mgr, false);
    }
    break;
  case kSM_Update:
  case kSM_Exit:
    break;
  }
}

void CPlayerGun::DamageRumble(const CVector3f& position, float damage, const CStateManager& mgr) {
  mDamageAmount = damage;
  mDamageLocation = position;
}

bool CPlayerGun::IsOutOfAmmoToShoot(CStateManager& mgr) const {
  const CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
  switch (mCurrentBeamId) {
  case CPlayerState::kBI_Dark:
    return state->GetItemAmount(CPlayerState::kIT_DarkAmmo, true) < 1;
  case CPlayerState::kBI_Light:
    return state->GetItemAmount(CPlayerState::kIT_LightAmmo, true) < 1;
  case CPlayerState::kBI_Annihilator:
    return state->GetItemAmount(CPlayerState::kIT_LightAmmo, true) < 1 ||
           state->GetItemAmount(CPlayerState::kIT_DarkAmmo, true) < 1;
  default:
    return false;
  }
}

bool CPlayerGun::GetBeamAmmoTypeAndCosts(bool combo, CStateManager& mgr,
                                         CPlayerState::EItemType& ammoA,
                                         CPlayerState::EItemType& ammoB, int& cost) const {
  static const int normalCosts[] = {0, 1, 1, 1};
  static const int chargedCosts[] = {0, 5, 5, 5};
  static const int comboCosts[] = {0, 30, 30, 30};
  const CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
  ammoA = CPlayerState::kIT_Invalid;
  ammoB = CPlayerState::kIT_Invalid;
  cost = normalCosts[mCurrentBeamId];
  switch (mCurrentBeamId) {
  case CPlayerState::kBI_Dark:
    ammoA = CPlayerState::kIT_DarkAmmo;
    break;
  case CPlayerState::kBI_Light:
    ammoA = CPlayerState::kIT_LightAmmo;
    break;
  case CPlayerState::kBI_Annihilator:
    ammoA = CPlayerState::kIT_LightAmmo;
    ammoB = CPlayerState::kIT_DarkAmmo;
    break;
  default:
    return true;
  }
  if (combo) {
    cost = comboCosts[mCurrentBeamId];
  } else if (mChargePhase >= kCP_Charging && mChargePhase <= kCP_Charged) {
    cost = chargedCosts[mCurrentBeamId];
  }
  if (ammoA != CPlayerState::kIT_Invalid && state->GetItemAmount(ammoA, true) < cost) {
    return false;
  }
  if (ammoB != CPlayerState::kIT_Invalid && state->GetItemAmount(ammoB, true) < cost) {
    return false;
  }
  return true;
}

CStateMachine* CPlayerGun::GetStateMachine() {
  return mStateMachineToken.TryCache() ? *mStateMachineToken : nullptr;
}

void CPlayerGun::PollStateMachine(CStateManager& mgr) {
  if (mStateMachine.GetCurrentState() == nullptr && GetStateMachine() != nullptr) {
    InitializeStateMachine(mgr);
  }
}

void CPlayerGun::ResetStateMachine(CStateManager& mgr) {
  if (mStateMachine.GetCurrentState() == nullptr || strcmp("Start", mStateMachine.GetName()) != 0) {
    mStateMachine.SetState(mgr, *this, rstl::string_l("Start"));
  }
}

void CPlayerGun::InitializeStateMachine(CStateManager& mgr) {
  mStateMachine.Setup(GetStateMachine());
  mStateMachine.SetTriggerFunctions(skGunTriggerFunctions, ARRAY_SIZE(skGunTriggerFunctions));
  mStateMachine.SetStateFunctions(skGunStateFunctions, ARRAY_SIZE(skGunStateFunctions));
  ResetStateMachine(mgr);
  mStateMachineInitialized = true;
}

TUniqueId CPlayerGun::CreatePowerBomb(CStateManager& mgr) { return DropPowerBomb(mgr); }

void CPlayerGun::PlayAnim(CStateManager& mgr, int animation, bool loop) {
  static const ushort toMissile[2][4] = {{0x2f9, 0x34c, 0x34e, 0x350},
                                         {0x25d0, 0x25d3, 0x25d5, 0x25d7}};
  static const ushort fromMissile[2][4] = {{0x2fa, 0x34d, 0x34f, 0x351},
                                           {0x25d1, 0x25d4, 0x25d6, 0x25d8}};
  static const ushort fromBeam[2][4] = {{0, 0x1fd3, 0x1fd7, 0x1fd5}, {0, 0x2597, 0x25dd, 0x25da}};
  static const ushort toBeam[2] = {0xc5, 0x25b3};

  mCurrentBeam->PlayAnim(NWeaponTypes::EGunAnimType(animation), loop);
  ushort sound = CSfxManager::kInternalInvalidSfxId;
  switch (animation) {
  case NWeaponTypes::kGAT_ToMissile:
    sound = toMissile[mSoundSetIndex][mCurrentBeamId];
    break;
  case NWeaponTypes::kGAT_FromMissile:
    sound = fromMissile[mSoundSetIndex][mCurrentBeamId];
    break;
  case NWeaponTypes::kGAT_FromBeam:
    sound = toBeam[mSoundSetIndex];
    break;
  case NWeaponTypes::kGAT_ToBeam:
    sound = fromBeam[mSoundSetIndex][mCurrentBeamId];
    break;
  }
  if (sound != CSfxManager::kInternalInvalidSfxId) {
    PlaySfxForPlayer(GetPlayer(mgr), sound, mSoundVolume, mgr.GetNextAreaId().Value(), mUnderwater,
                     0);
  }
}

void CPlayerGun::ResetSeeker(CStateManager& mgr) {
  StopChargeSound(mgr, false);
  EnableSeekerFx(mgr, false);
  mSeekerChargeState = kSCS_NotCharging;
  mSeekerChargeFactor = 0.f;
  mCurrentSeekerTarget = kInvalidUniqueId;
  mSeekerLockTimer = 0.f;
  mAllSeekersLockedTime = 0.f;
  mSeekerTargets.clear();
}

void CPlayerGun::HandleWeaponChange(const CFinalInput& input, CStateManager& mgr) {
  if (mBeamChangeState == kBCS_Idle) {
    HandleBeamChange(input, mgr);
  }
}

CPlayerGun::CGunMorph::CGunMorph(float transformTime, float holdTime)
: mYLerp(1.f)
, mGunTransformTime(transformTime > 0.f ? transformTime : 1.f)
, mRemTime(0.f)
, mSpeed(0.1f)
, mHoloHoldTime(fabsf(holdTime))
, mRemHoldTime(2.f)
, mTransitionFactor(1.f)
, mMorphDirection(kMD_Done)
, mGunState(kGS_OutWipeDone)
, mMorphing(false)
, mWeaponChanged(false) {}

void CPlayerGun::CGunMorph::StartWipe(EMorphDir direction) {
  mRemHoldTime = mHoloHoldTime;
  if (direction == kMD_In && mGunState == kGS_InWipeDone) {
    return;
  }
  if (mMorphDirection != direction && mGunState != kGS_OutWipe) {
    mRemTime = mGunTransformTime;
    mSpeed = 1.f / mGunTransformTime;
  } else if (mGunState != kGS_InWipe) {
    mRemTime = mGunTransformTime - mRemTime;
  }
  mMorphDirection = direction;
  mGunState = mMorphDirection == kMD_In ? kGS_InWipe : kGS_OutWipe;
  mMorphing = true;
}

CPlayerGun::CGunMorph::EWipeEvent CPlayerGun::CGunMorph::Update(float inY, float outY, float dt,
                                                                const CPlayer& player) {
  const bool cinematic = player.GetCameraManager()->IsInCinematicCamera();
  EWipeEvent event = kWE_None;
  if (mGunState == kGS_InWipeDone) {
    mRemHoldTime -= dt;
    if ((mRemHoldTime <= 0.f || cinematic) && mWeaponChanged) {
      StartWipe(kMD_Out);
      mWeaponChanged = false;
      mRemHoldTime = 0.f;
      event = kWE_OutWipeStarted;
    }
  }
  if (mMorphing) {
    const float t = mRemTime * mSpeed;
    if (mMorphDirection == kMD_In) {
      mYLerp = inY * (1.f - t) + outY * t;
      mTransitionFactor = t;
    } else {
      mYLerp = outY * (1.f - t) + inY * t;
      mTransitionFactor = 1.f - t;
    }
    if (mRemTime > 0.f) {
      mRemTime -= dt;
      if (cinematic) {
        mRemTime = 0.f;
      }
    } else {
      mMorphing = false;
      mRemTime = 0.f;
      if (mMorphDirection == kMD_In) {
        mGunState = kGS_InWipeDone;
        mTransitionFactor = 0.f;
      } else {
        mGunState = kGS_OutWipeDone;
        mTransitionFactor = 1.f;
        mMorphDirection = kMD_Done;
        event = kWE_OutWipeFinished;
      }
    }
  }
  return event;
}

CPlayerGun::CMotionState::CMotionState(float extendDistance)
: mExtendParabolaDelayTimer(0.f)
, mFireTime(0.f)
, mCurrentExtendDistance(0.f)
, mCurrentRotation(0.f)
, mRotationT(0.f)
, mStartRotation(0.f)
, mEndRotation(0.f)
, mExtendDistance(extendDistance)
, mMotionState(kMS_Zero)
, mFireState(kFS_NotFiring)
, mExtendParabola(true) {}

void CPlayerGun::CMotionState::Update(bool firing, float dt, CTransform4f& transform,
                                      CStateManager& mgr) {
  if (firing) {
    mFireState = kFS_StartFire;
    mFireTime = 0.f;
  } else if (mFireState != kFS_NotFiring) {
    if (mFireTime > dt) {
      mFireState = kFS_Firing;
    }
    mFireTime += dt;
  }

  if (mExtendParabola && mMotionState == kMS_LockOn) {
    const float extendT = mCurrentExtendDistance * (1.f / mExtendDistance);
    CTransform4f other =
        CTransform4f::RotateZ(CRelAngle::FromDegrees(15.f * -4.f * extendT * (extendT - 1.f)));
    other.SetTranslation(CVector3f(0.f, mCurrentExtendDistance, 0.f));
    transform = transform * other;
  } else if (mFireState == kFS_StartFire || mFireState == kFS_Firing) {
    if (fabs(mRotationT - 1.f) < 0.1f) {
      mStartRotation = mEndRotation;
      mRotationT = 0.f;
      if (mFireState == kFS_StartFire) {
        mEndRotation = CCast::StoF(mgr.Random()->Next() % 15);
        mEndRotation *= (mgr.Random()->Next() % 100) > 45 ? 1.f : -1.f;
      } else {
        mEndRotation = 0.f;
        if (mStartRotation == mEndRotation) {
          mCurrentRotation = mEndRotation;
          mFireState = kFS_NotFiring;
        }
      }
    } else {
      mCurrentRotation = (mEndRotation - mStartRotation) * mRotationT + mStartRotation;
    }

    mRotationT += (10.f * dt) * 0.8f * (1.f - mRotationT);
    const CRelAngle angle = CRelAngle::FromDegrees(mCurrentRotation);
    CQuaternion quat = CQuaternion::AxisAngle(CUnitVector3f(transform.GetForward()), angle);
    CTransform4f rotated = quat.BuildTransform4f() * transform.GetRotation();
    rotated.SetTranslation(transform.GetTranslation());
    transform = rotated * CTransform4f::Translate(0.f, mCurrentExtendDistance, 0.f);
  } else {
    transform = transform * CTransform4f::Translate(0.f, mCurrentExtendDistance, 0.f);
  }

  switch (mMotionState) {
  case kMS_LockOn:
    mCurrentExtendDistance += 3.f * dt;
    if (mCurrentExtendDistance > mExtendDistance) {
      mCurrentExtendDistance = mExtendDistance;
      mMotionState = kMS_One;
      mExtendParabola = false;
    }
    break;
  case kMS_CancelLockOn:
    mCurrentExtendDistance -= 3.f * dt;
    if (mCurrentExtendDistance < 0.f) {
      mCurrentExtendDistance = 0.f;
      mMotionState = kMS_Zero;
    }
    break;
  default:
    break;
  }

  if (!mExtendParabola) {
    if (mExtendParabolaDelayTimer < 30.f) {
      mExtendParabolaDelayTimer += dt;
    } else {
      mExtendParabola = true;
      mExtendParabolaDelayTimer = 0.f;
    }
  }
}

void CPlayerGun::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CPlayerGunBase::AcceptScriptMsg(mgr, msg);
  const TUniqueId sender = msg.GetSenderId();
  CPlayer& player = *GetPlayerFromAll(mgr);
  const CPlayerState& playerState = *player.GetPlayerState();

  switch (msg.GetMessage()) {
  case kSM_Create:
    SetBeam(playerState.GetCurrentBeam(), mgr);
    for (rstl::reserved_vector< CGunWeapon*, 4 >::iterator it = mSelectableBeams.begin();
         it != mSelectableBeams.end(); ++it) {
      (*it)->SetSoundVolume(mSoundVolume);
    }
    mAuxWeapon->SetSoundVolume(mSoundVolume);
    InitMuzzleData(mgr);
    if (mgr.IsMultiplayer()) {
      for (int i = 0; i < mSelectableBeams.size(); ++i) {
        mSelectableBeams[i]->Load(mgr, false);
        mSelectableBeams[i]->Unload(mgr);
      }
    }
    break;
  case kSM_XINF:
    if (mUnderwater && mAuxWeapon->IsComboFxActive(mgr)) {
      StopContinuousBeam(mgr, false);
    }
    break;
  case kSM_Damage: {
    bool bigStrike = false;
    bool metroidAttached = false;
    const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(sender));
    if (weapon && weapon->HasAttrib(CWeapon::kPA_BigStrike)) {
      bigStrike = true;
      mBigStrikeTimer = weapon->GetDamageDuration();
    } else if (const CPatterned* ai = TCastToConstPtr< CPatterned >(mgr.GetObjectById(sender))) {
      if (ai->IsMakingBigStrike()) {
        bigStrike = true;
        mBigStrikeTimer = ai->GetDamageDuration();
        const TUniqueId attachedActor = player.GetAttachedActorId();
        if (attachedActor != kInvalidUniqueId) {
          metroidAttached =
              TCastToConstPtr< CMetroid >(mgr.GetObjectById(attachedActor)) != nullptr;
        }
      }
    }
    if (!mInBigStrike) {
      if (bigStrike) {
        mGunMotionReturningFromStrike = false;
      }
      TakeDamage(bigStrike, !metroidAttached, mgr);
    }
    break;
  }
  case kSM_Landed:
    if (player.IsLandingStrikePending() && !mInBigStrike) {
      player.SetLandingStrikePending(false);
      TakeDamage(true, false, mgr);
      mBigStrikeTimer = 0.75f;
    }
    break;
  case kSM_Delete:
    break;
  default:
    break;
  }

  mGrappleArm->AcceptScriptMsg(mgr, msg);
  mAuxWeapon->AcceptScriptMsg(mgr, msg);
}

void CPlayerGun::TouchModel(const CStateManager& mgr) const {
  if (mgr.IsMultiplayer()) {
    mGunMotion->GetModelData().Touch();
    mGrappleArm->TouchModel(mgr);
    for (rstl::reserved_vector< CGunWeapon*, 4 >::const_iterator it = mSelectableBeams.begin();
         it != mSelectableBeams.end(); ++it) {
      (*it)->Touch(mgr);
    }
  } else {
    if (GetPlayer(const_cast< CStateManager& >(mgr))->GetMorphballTransitionState() !=
        CPlayer::kMS_Morphed) {
      mGunMotion->GetModelData().Touch(mgr, 0);
      mCurrentBeam->Touch(mgr);
      mGrappleArm->TouchModel(mgr);
    }
    if (mLoadingBeam != nullptr) {
      mLoadingBeam->Touch(mgr);
      mLoadingBeam->TouchHolo(mgr);
    }
  }
}

void CPlayerGun::PreRender(CStateManager& mgr, const CVector3f& cameraPosition) {
  const CPlayerState* state = mgr.GetPlayerState();
  const CPlayer* player = GetPlayer(mgr);
  if (state->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    return;
  }

  const CPlayerState::EPlayerVisor visor = state->GetActiveVisor(mgr);
  if (mgr.IsMultiplayer() && mgr.GetNumPlayers() >= 3u) {
    const uint frame = mgr.GetRenderFrameIndex();
    if ((frame & 1) != 0 && visor == CPlayerState::kPV_Combat &&
        ((frame >> 1) & 3) == mgr.MaskUIdNumPlayers(mPlayerUniqueId) &&
        mCurrentBeam->SolidModelData()) {
      const CTransform4f gunTransform = CTransform4f::Translate(cameraPosition) * mGunWorldXf;
      const CAABox bounds = mCurrentBeam->GetBounds(gunTransform);
      mLights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()), bounds);
    }
  } else {
    if (visor == CPlayerState::kPV_Combat && mCurrentBeam->SolidModelData()) {
      const CTransform4f gunTransform = CTransform4f::Translate(cameraPosition) * mGunWorldXf;
      CWorldShadow* shadow = GetWorldShadow();
      const CAABox bounds = mCurrentBeam->GetBounds(gunTransform);
      mLights.SetAmbientColor(mAmbientColor);
      mLights.SetFindShadowLight(CWorldShadow::CanRender(mgr));
      if (mgr.GetNextAreaId() != kInvalidAreaId) {
        mLights.SetWorldLightingLevel(0.25f);
        mLights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()), bounds);
      }
      mLights.BuildDynamicLightList(mgr, bounds);
      mAmbientColor = mLights.GetAmbientColor();
      if (mLights.HasShadowLight() && mCurrentBeam->IsLoaded()) {
        shadow->BuildLightShadowTexture(mgr, mgr.GetNextAreaId(), mLights.GetShadowLightIndex(),
                                        bounds, true, false);
      } else {
        shadow->ResetBlur();
      }
    }
    const CColor damageColor = player->GetDarkAetherDamageColor(mgr, 0);
    if (!(damageColor == CColor::Black())) {
      mLights.SetAmbientColor(CColor::Add(mAmbientColor, damageColor));
    }
  }

  if (mGrappleArm->GetStateFlags() != 0) {
    mGrappleArm->PreRender(mgr, cameraPosition);
  }
}

void CPlayerGun::AddToRenderer(const CStateManager& mgr) const {
  const rstl::optional_object< CModelData >& model = mCurrentBeam->SolidModelData();
  if (model) {
    model->RenderParticles(mgr.GetFrustumPlanes());
  }
}

void CPlayerGun::Render(const CStateManager& mgr, const CVector3f& cameraTranslation,
                        const CModelFlags& flags) const {
  const CGraphics::CProjectionState backupProjection(CGraphics::GetProjectionState());
  CGraphics::SetDepthRange(1.f / 32.f, 1.f / 8.f);
  const CTransform4f gunTransform = CTransform4f::Translate(cameraTranslation) * mGunWorldXf;
  const CTransform4f elbowTransform = gunTransform * mElbowLocalXf;
  const CGunMorph::EGunState gunState = mGunMorph.mGunState;
  const CTransform4f backupView(CGraphics::GetViewMatrix());
  CGraphics::SetViewPointMatrix(gunTransform.GetInverse() * backupView);
  gpRender->SetModelMatrix(CTransform4f::Identity());

  if (mChargeEffectVisible) {
    if (mAuxMuzzleGenerators[mCurrentBeamId].get() != nullptr && mChargePhase == kCP_ChargeFx) {
      mAuxMuzzleGenerators[mCurrentBeamId]->Render();
    }
    if (mMuzzleEffectVisTimer > 0.f ||
        (mChargePhase != kCP_NotCharging && mChargePhase != kCP_ChargeRequested &&
         mChargePhase != kCP_Charging)) {
      mCurrentBeam->DrawMuzzleFx(mgr);
    }
    if (mMissileAuxMuzzleGenerator.get() != nullptr && mSeekerChargeState == kSCS_FullyCharged) {
      mMissileAuxMuzzleGenerator->Render();
    }
    if (!mSeekerMuzzleGenerators.empty() &&
        (mMuzzleEffectVisTimer > 0.f ||
         (mSeekerChargeState != kSCS_NotCharging && mSeekerChargeState != kSCS_Requested &&
          mSeekerChargeState != kSCS_Opening))) {
      for (int i = 0; i < mSeekerMuzzleGenerators.size(); ++i) {
        if (mSeekerMuzzleGenerators[i].get() != nullptr) {
          mSeekerMuzzleGenerators[i]->Render();
        }
      }
    }
  }
  if (mHoloTransitionGenerator.get() != nullptr &&
      mgr.GetRenderVisorMode() != CStateManager::kRVM_Echo &&
      (gunState == CGunMorph::kGS_InWipe || gunState == CGunMorph::kGS_OutWipe)) {
    mHoloTransitionGenerator->Render();
  }
  CGraphics::SetViewPointMatrix(backupView);
  if (mComboFiring) {
    mAuxWeapon->RenderMuzzleFx();
  }
  mCurrentBeam->PreRenderGunFx(mgr, gunTransform);
  if (!mgr.IsMultiplayer()) {
    mGunMotion->Draw(mgr, gunTransform);
  }
  const bool drawSuitArm = !mGrappleArm->IsGrappling() && mGunHolsterState == kGHS_Drawn;
  (this->*mRender)(mgr, cameraTranslation, drawSuitArm, elbowTransform, gunTransform, flags, flags);
  CElementGen* underwaterParticles =
      GetPlayer(const_cast< CStateManager& >(mgr))->GetUnderwaterParticles();
  if (underwaterParticles != nullptr && underwaterParticles->GetParticleCount() > 0) {
    underwaterParticles->Render();
  }

  const CTransform4f effectView(CGraphics::GetViewMatrix());
  CGraphics::SetViewPointMatrix(gunTransform.GetInverse() * effectView);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (mComboFiring && mComboTransferGenerator.get() != nullptr) {
    mComboTransferGenerator->Render();
  }
  if (mPhazonAbsorbFlashGenerator.get() != nullptr) {
    mPhazonAbsorbFlashGenerator->Render();
  }
  if (mPhazonChargeGenerator.get() != nullptr) {
    mPhazonChargeGenerator->Render();
  }
  mCurrentBeam->PostRenderGunFx(mgr, gunTransform);
  if (mSeekerSecondaryFx != CGunWeapon::kSFT_None && mMissileSecondaryGenerator.get() != nullptr) {
    mMissileSecondaryGenerator->Render();
  }
  CGraphics::SetViewPointMatrix(effectView);
  CGraphics::SetDepthRange(1.f / 8.f, 1.f);
  CGraphics::SetProjectionState(backupProjection);
}

void CPlayerGun::Reset(CStateManager& mgr) {
  CPlayerGunBase::Reset(mgr);
  ResetCharge(mgr, false);
  ResetSeeker(mgr);
  PlayAnim(mgr, 0, false);
  if (mStateMachineInitialized) {
    ResetStateMachine(mgr);
  }
}

void CPlayerGun::Update(float dt, CStateManager& mgr) {
  if (gpMain->IsMaxSpeed()) {
    return;
  }
  CPlayerGunBase::Update(dt, mgr);
  if (!mStateMachineInitialized) {
    PollStateMachine(mgr);
    return;
  }

  CPlayer* player = GetPlayer(mgr);
  CPlayerState* playerState = player->GetPlayerState();
  const bool isUnmorphed = player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  const bool justFroze = isUnmorphed ? just_froze(mFrozen, player->GetFrozenState()) : false;
  const bool justThawed = isUnmorphed ? just_thawed(mFrozen, player->GetFrozenState()) : false;
  mFrozen = isUnmorphed ? player->GetFrozenState() : false;
  mFiring = false;
  const float advDt = mFrozen ? 0.f : dt;
  if (mCurrentBeam->IsSpecialAnimationPlaying() && !mCurrentBeam->GetSpeedUpAnimation()) {
    mCurrentBeam->SetSpeedUpAnimation(mInputFlags != 0);
  }
  mCurrentBeam->Update(advDt, mgr);
  mGunMotion->Update(advDt, mgr);
  mGrappleArm->Update(advDt, mgr);
  if (!mAuxWeapon->IsLoaded()) {
    mAuxWeapon->LoadIdle();
  }
  if (!mCommonDependenciesLoaded && NWeaponTypes::are_tokens_ready(mCommonDependencies)) {
    mCommonDependenciesLoaded = true;
  }
  mStateMachine.Update(mgr, *this, advDt);
  UpdateTimers(dt);
  if (!mCurrentBeam->IsLoaded()) {
    return;
  }

  if (mMissileShotInterval == 0.f) {
    mMissileShotInterval = mCurrentBeam->GetAnimDuration(NWeaponTypes::kGAT_MissileReload) +
                           mCurrentBeam->GetAnimDuration(NWeaponTypes::kGAT_FromBeam);
  }
  const CTransform4f gunLocalXf =
      GetLocatorTransform(mGunMotion->GetModelData(), rstl::string_l("GBSE_SDK"), true);
  CModelData& beamModel = *mCurrentBeam->SolidModelData();
  mBeamLocalXf =
      GetLocatorTransform(beamModel, rstl::string_l(CGunWeapon::GetMuzzleLocatorName()), true);
  mElbowLocalXf = GetLocatorTransform(beamModel, rstl::string_l("elbow"), false);
  mGunWorldXf = mTransform * gunLocalXf;
  CTransform4f beamWorldXf = mGunWorldXf * mBeamLocalXf;
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      !player->GetCameraManager()->IsInCinematicCamera()) {
    TUniqueId bestId = kInvalidUniqueId;
    const CVector3f direction = mGunWorldXf.GetForward().AsNormalized();
    const CVector3f offset = -(0.5f * direction);
    const CVector3f position = mGunWorldXf.GetTranslation() + offset;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    bounds.AccumulateBounds(position);
    bounds.AccumulateBounds(position + 3.5f * direction);
    mgr.BuildNearList(nearList, bounds, skWeaponCollisionFilter, player);
    const CRayCastResult result = mgr.RayWorldIntersection(bestId, position, direction, 3.5f,
                                                           skWeaponCollisionFilter, nearList);
    mPointBlankWorldSurface = result.IsValid();
    if (result.IsValid()) {
      mElbowWorldXf = mGunWorldXf * mElbowLocalXf;
      mElbowWorldXf.AddTranslation(offset);
      beamWorldXf.SetTranslation(result.GetPoint());
    }
  } else {
    mPointBlankWorldSurface = false;
  }
  CTransform4f beamTargetXf = mPointBlankWorldSurface ? mElbowWorldXf : beamWorldXf;
  beamModel.AdvanceParticles(mGunWorldXf, advDt, mgr);
  mCurrentBeam->UpdateGunFx(mShotSmokeTimer > 2.f && mTimeSinceFire > 0.15f, dt, mgr,
                            mElbowLocalXf);
  if (mSeekerSecondaryFx != CGunWeapon::kSFT_None && mMissileSecondaryGenerator.get()) {
    mMissileSecondaryGenerator->SetGlobalOrientAndTrans(mElbowLocalXf);
    mMissileSecondaryGenerator->Update(advDt);
  }
  if (justFroze) {
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
    mCurrentBeam->EnableFrozenEffect(kFFT_Frozen);
    if (mSeekerSecondaryFx != CGunWeapon::kSFT_None) {
      mSeekerSecondaryFx = CGunWeapon::kSFT_None;
    }
  } else if (justThawed) {
    mCurrentBeam->EnableFrozenEffect(kFFT_Thawed);
  }
  if (mBeamChangeState != kBCS_Idle) {
    UpdateBeamChange(advDt, mgr);
  }
  UpdateFreeLook(advDt, mgr);
  if (mGrappleArm->GetStateFlags() != 0 && !mGrappleArm->IsGrappling()) {
    UpdateLeftArmTransform();
  }
  UpdateGunMotion(advDt, mgr);

  const CVector3f cameraTranslation =
      player->GetCameraManager()->GetGlobalCameraTranslation(mgr, true);
  beamWorldXf.AddTranslation(cameraTranslation);
  beamTargetXf.AddTranslation(cameraTranslation);
  if (mChargeEffectVisible) {
    const bool emitting = mComboTransferFactor < 1.f;
    const float scaleFactor = mComboFiring ? 2.f * (1.f - mComboTransferFactor) : 2.f;
    const CVector3f scale(scaleFactor, scaleFactor, scaleFactor);
    mCurrentBeam->UpdateMuzzleFx(advDt, scale, mBeamLocalXf.GetTranslation(), emitting);
    if (mSeekerSecondaryFx != CGunWeapon::kSFT_None) {
      if (mMissileAuxMuzzleGenerator.get()) {
        mMissileAuxMuzzleGenerator->SetGlobalOrientAndTrans(mBeamLocalXf);
        mMissileAuxMuzzleGenerator->SetGlobalScale(scale);
        mMissileAuxMuzzleGenerator->SetParticleEmission(
            mMissileAuxMuzzleGenerator->GetParticleEmission() && emitting);
        mMissileAuxMuzzleGenerator->Update(advDt);
      }
      for (int i = 0; i < mSeekerMuzzleGenerators.size(); ++i) {
        if (mSeekerMuzzleGenerators[i].get()) {
          mSeekerMuzzleGenerators[i]->SetGlobalTranslation(mBeamLocalXf.GetTranslation());
          mSeekerMuzzleGenerators[i]->SetGlobalScale(scale);
          mSeekerMuzzleGenerators[i]->SetParticleEmission(emitting);
          mSeekerMuzzleGenerators[i]->Update(advDt);
        }
      }
    }
    if (mAuxMuzzleGenerators[mCurrentBeamId].get() && mChargePhase == kCP_ChargeFx) {
      mAuxMuzzleGenerators[mCurrentBeamId]->SetGlobalOrientAndTrans(mBeamLocalXf);
      mAuxMuzzleGenerators[mCurrentBeamId]->SetGlobalScale(scale);
      mAuxMuzzleGenerators[mCurrentBeamId]->SetParticleEmission(emitting);
      mAuxMuzzleGenerators[mCurrentBeamId]->Update(advDt);
    }

    const CScriptPlayerHint* hint =
        TCastToConstPtr< CScriptPlayerHint >(player->GetPlayerHintManager()->GetCurrentHint(mgr));
    if (hint && (hint->GetOverrideFlags() & 0x40000) && playerState->GetChargeBeamFactor() == 1.f) {
      CTransform4f absorbXf = mPointBlankWorldSurface ? mElbowWorldXf : mGunWorldXf * mBeamLocalXf;
      absorbXf.AddTranslation(player->GetCameraManager()->GetGlobalCameraTranslation(mgr, true));
      if (absorbXf.GetForward().GetZ() > 0.35f) {
        const float radius = gpTweakPlayerGun->GetPhazonShotAbsorbRadius();
        rstl::reserved_vector< TUniqueId, 1024 > nearList;
        CAABox bounds(CVector3f(-radius, 0.f, -radius), CVector3f(radius, 0.f, radius));
        bounds = bounds.GetTransformedAABox(absorbXf);
        mgr.BuildNearList(nearList, bounds,
                          CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)), player);
        const float radiusSquared = radius * radius;
        bool absorbed = false;
        for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
             it != nearList.end(); ++it) {
          CGameProjectile* projectile =
              TCastToPtr< CGameProjectile >(const_cast< CEntity* >(mgr.GetObjectById(*it)));
          if (projectile && projectile->GetOwnerId() != mPlayerUniqueId &&
              projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Phazon &&
              (projectile->GetTranslation() - absorbXf.GetTranslation()).MagSquared() <
                  radiusSquared) {
            mAbsorbedPhazonShots = rstl::min_val(mAbsorbedPhazonShots + 1,
                                                 gpTweakPlayerGun->GetMaxAbsorbedPhazonShots());
            mgr.DeleteObjectRequest(*it);
            absorbed = true;
            if (!mPhazonChargeGenerator.get() &&
                mAbsorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
              const TCachedToken< CGenDescription > description =
                  gpSimplePool->GetObj("PhazonCharge");
              mPhazonChargeGenerator =
                  rstl::auto_ptr< CElementGen >(rs_new CElementGen(description));
              mCurrentBeam->ActivateCharge(false, false);
              if (mChargeSfx) {
                CSfxManager::SfxStop(mChargeSfx);
                mChargeSfx = PlaySfxForPlayer(nullptr, 0x179, mSoundVolume, kInvalidAreaId.Value(),
                                              mUnderwater, true);
              }
            }
          }
        }
        if (absorbed) {
          const TCachedToken< CGenDescription > description =
              gpSimplePool->GetObj("PhazonAbsorbFlash");
          mPhazonAbsorbFlashGenerator =
              rstl::auto_ptr< CElementGen >(rs_new CElementGen(description));
          PlaySfxForPlayer(GetPlayer(mgr), 0x1d1, mSoundVolume, mgr.GetNextAreaId().Value(),
                           mUnderwater, false);
        }
      }
    }
    if (mPhazonAbsorbFlashGenerator.get()) {
      if (!mPhazonAbsorbFlashGenerator->IsSystemDeletable()) {
        mPhazonAbsorbFlashGenerator->SetGlobalTranslation(mBeamLocalXf.GetTranslation());
        mPhazonAbsorbFlashGenerator->Update(advDt);
      } else {
        mPhazonAbsorbFlashGenerator = rstl::auto_ptr< CElementGen >();
      }
    }
    if (mPhazonChargeGenerator.get()) {
      mPhazonChargeGenerator->SetGlobalTranslation(mBeamLocalXf.GetTranslation());
      mPhazonChargeGenerator->Update(advDt);
    }
  }
  if (playerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    UpdateAuxWeapons(advDt, beamTargetXf, mgr);
    UpdateChargeState(advDt, mgr);
  }
  UpdateGunLight(beamWorldXf, mgr);
  DoUserAnimEvents(advDt, mgr);
}

float CPlayerGun::GetBeamVelocity() const {
  return mCurrentBeam->IsLoaded() ? mCurrentBeam->GetVelocityInfo().GetVelocity(mChargeState).GetY()
                                  : 10.f;
}

void CPlayerGun::SetAuxTargetId(TUniqueId target) {
  if (!mAuxWeapon.null()) {
    mAuxWeapon->SetTargetId(target);
  }
}

TUniqueId CPlayerGun::GetAuxTargetId() const {
  if (!mAuxWeapon.null()) {
    return mAuxWeapon->GetTargetId();
  }
  return kInvalidUniqueId;
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr) { mCurrentBeam->AsyncLoadSuitArm(); }

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  CPlayerGunBase::ProcessInput(input, mgr);
  CPlayerState* state = GetPlayerFromAll(mgr)->GetPlayerState();
  if (state->HasPowerUp(CPlayerState::kIT_ChargeBeam)) {
    if (!state->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
      state->EnableItem(CPlayerState::kIT_ChargeBeam);
    }
  } else if (state->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    state->DisableItem(CPlayerState::kIT_ChargeBeam);
    ResetCharge(mgr, false);
  }
  CPlayer* player = GetPlayer(mgr);
  if (state->GetItemAmount(CPlayerState::kIT_SwitchWeaponPower, true) != 0) {
    state->SetCurrentBeam(CPlayerState::kBI_Power);
    state->SetItemAmount(CPlayerState::kIT_SwitchWeaponPower, 0);
  } else if (state->GetItemAmount(CPlayerState::kIT_SwitchWeaponAnnihilator, true) != 0) {
    state->SetCurrentBeam(CPlayerState::kBI_Annihilator);
    state->SetItemAmount(CPlayerState::kIT_SwitchWeaponAnnihilator, 0);
  } else if (state->GetItemAmount(CPlayerState::kIT_SwitchWeaponLight, true) != 0) {
    state->SetCurrentBeam(CPlayerState::kBI_Light);
    state->SetItemAmount(CPlayerState::kIT_SwitchWeaponLight, 0);
  } else if (state->GetItemAmount(CPlayerState::kIT_SwitchWeaponDark, true) != 0) {
    state->SetCurrentBeam(CPlayerState::kBI_Dark);
    state->SetItemAmount(CPlayerState::kIT_SwitchWeaponDark, 0);
  }
  if (mgr.IsMultiplayer() && IsOutOfAmmoToShoot(mgr) &&
      state->GetCurrentBeam() != CPlayerState::kBI_Power) {
    PlaySfxForPlayer(GetPlayer(mgr), skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                     mgr.GetNextAreaId().Value(), mUnderwater, false);
    state->SetCurrentBeam(CPlayerState::kBI_Power);
  }
  switch (player->GetMorphballTransitionState()) {
  case CPlayer::kMS_Unmorphed:
    if (mBeamChangeState == kBCS_Idle) {
      HandleWeaponChange(input, mgr);
    }
    break;
  case CPlayer::kMS_Morphed:
  case CPlayer::kMS_Morphing:
  case CPlayer::kMS_Unmorphing:
    break;
  }
}

CVector3f CPlayerGun::GetRainSplashPosition() const {
  if (mCurrentBeam != nullptr) {
    return mCurrentBeam->GetRainSplashPosition();
  } else {
    return CVector3f::Zero();
  }
}

int CPlayerGun::GetBombsAvailable(CStateManager& mgr) const {
  const TUniqueId playerId = GetPlayerUniqueId();
  return 3 - mgr.GetWeaponMgr()->GetNumActive(playerId, kWT_Bomb);
}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr) const {
  CDamageInfo damage = GetPlayer(mgr)->GetDeathTime() > 0.f
                           ? CDamageInfo(CWeaponMode(kWT_PowerBomb), 0.f, 0.f, 0.f)
                           : gpTweakPlayerGun->GetPowerBombInfo();
  damage = damage.ApplyDoubleDamage(*GetPlayer(mgr)->GetPlayerState());
  const float radius = GetPlayer(mgr)->GetMorphBall()->GetBallRadius();
  const TUniqueId id = mgr.AllocateUniqueId();
  CPowerBomb* bomb = rs_new CPowerBomb(
      mBombEffects[kBW_PowerBomb][0], id, GetPlayer(mgr)->GetCurrentAreaId(), mPlayerUniqueId,
      kWT_PowerBomb, mgr.IsMultiplayer() ? CPowerBomb::kF_NoDamageDelay : 0,
      CTransform4f::Translate(GetPlayer(mgr)->GetTranslation() + CVector3f(0.f, 0.f, radius)),
      damage);
  mgr.AddObject(bomb);
  return id;
}

void CPlayerGun::DropBomb(EBWeapon type, CStateManager& mgr) {
  const float radius = GetPlayer(mgr)->GetMorphBall()->GetBallRadius();
  CPlayer* player = GetPlayerFromAll(mgr);
  switch (type) {
  case kBW_PowerBomb:
    player->GetPlayerState()->DecrementAmmoAndDisplayAlertIfOut(mgr, CPlayerState::kIT_Powerbomb,
                                                                1);
    mPowerBombId = DropPowerBomb(mgr);
    break;
  case kBW_Bomb: {
    if (mBombCount <= 0) {
      break;
    }
    const CMaterialList triggerMaterials(kMT_Player);
    CDamageInfo damage = gpTweakPlayerGun->GetBombInfo();
    damage = damage.ApplyDoubleDamage(*GetPlayer(mgr)->GetPlayerState());
    CBomb* bomb = rs_new CBomb(
        mBombEffects[type][0], mBombEffects[type][1], mgr.AllocateUniqueId(),
        GetPlayer(mgr)->GetCurrentAreaId(), mPlayerUniqueId, triggerMaterials, kWT_Bomb, 0x100, 1.f,
        gpTweakPlayerGun->GetBombTriggerRadius(),
        CTransform4f::Translate(GetPlayer(mgr)->GetTranslation() + CVector3f(0.f, 0.f, radius)),
        damage);
    mgr.AddObject(bomb);
    mBombReloadTimer += gpTweakPlayerGun->GetBombDropDelayTime();
    --mBombCount;
    if (CEntity* entity = mgr.GetObjectByIdFromListAll(GetPlayer(mgr)->GetRidingPlatform())) {
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(entity)) {
        platform->AddSlave(bomb->GetUniqueId(), mgr, rstl::optional_object_null());
      }
    }
    break;
  }
  }
}

void CPlayerGun::FireBombs(CStateManager& mgr) {
  if (AreBombsDisabled()) {
    return;
  }

  CPlayerState* state = GetPlayerFromAll(mgr)->GetPlayerState();
  bool bombReady = true;
  if (mPowerBombId != kInvalidUniqueId &&
      !mgr.CanCreateProjectile(mPlayerUniqueId, kWT_PowerBomb, 1)) {
    const CPowerBomb* powerBomb = static_cast< const CPowerBomb* >(mgr.GetObjectById(mPowerBombId));
    if (powerBomb == nullptr || powerBomb->IsEnding()) {
      mPowerBombId = kInvalidUniqueId;
    } else {
      bombReady = false;
    }
  }

  if ((mPressedInputFlags & 2) == 0) {
    if (state->GetItemAmount(CPlayerState::kIT_MorphBallBombs, true) != 0 && bombReady &&
        (mPressedInputFlags & 1) != 0) {
      DropBomb(kBW_Bomb, mgr);
    }
  } else if (state->GetItemAmount(CPlayerState::kIT_Powerbomb, true) > 0 &&
             mgr.CanCreateProjectile(mPlayerUniqueId, kWT_Bomb, 1) &&
             mgr.CanCreateProjectile(mPlayerUniqueId, kWT_PowerBomb, 1)) {
    DropBomb(kBW_PowerBomb, mgr);
  }
}

void CPlayerGun::TakeDamage(bool bigStrike, bool strikeGrapple, CStateManager& mgr) {
  const CPlayer& player = *GetPlayer(mgr);
  bool hasStrikeAngle = false;
  float angle = 0.f;
  if (mDamageAmount >= 10.f && !bigStrike && !mComboFiring && mGunStrikeDelayTimer <= 0.f) {
    mGunStrikeDelayTimer = 20.f;
    mGunStrikeCooldownTimer = 0.75f;
    if (mGunMorph.mGunState == CGunMorph::kGS_OutWipeDone) {
      const CVector3f localDamage = player.GetTransform().TransposeRotate(mDamageLocation);
      angle = CAbsAngle::FromRadians(atan2(localDamage.GetY(), localDamage.GetX())).AsDegrees();
      hasStrikeAngle = true;
    }
  }

  const bool struck = hasStrikeAngle || bigStrike;
  if (struck && player.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mGunMotion->PlayPasAnim(SamusGun::kAS_Struck, mgr, angle, bigStrike);
    ResetCharge(mgr, false);
    ResetSeeker(mgr);
    PlayAnim(mgr, 0, false);
    if ((bigStrike && strikeGrapple) || mInFreeLook) {
      mGrappleArm->EnterStruck(mgr, angle, bigStrike, !mInFreeLook);
    }
  }

  mInterruptEvent = mInterruptEvent || struck;
  mInBigStrike = bigStrike;
  mDamageAmount = 0.f;
  mDamageLocation = CVector3f::Zero();
}

TUniqueId CPlayerGun::GetTargetId(CStateManager& mgr) {
  TUniqueId target = GetPlayer(mgr)->GetOrbitTargetId();
  if (target != kInvalidUniqueId) {
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(target));
    if (actor != nullptr && !actor->GetMaterialList().HasMaterial(kMT_Target) &&
        !actor->GetMaterialList().HasMaterial(kMT_SeekerTarget)) {
      target = kInvalidUniqueId;
    }
  }
  return target;
}

void CPlayerGun::PlayBeamFireSfx(CStateManager& mgr, CPlayer& player, bool play) {
  static const ushort sounds[2][2] = {{0xca, 0xcb}, {0x25cd, 0x25ce}};
  if (play && !player.GetCameraManager()->IsInCinematicCamera()) {
    PlaySfxForPlayer(GetPlayer(mgr), sounds[mSoundSetIndex][0], mSoundVolume,
                     mgr.GetNextAreaId().Value(), mUnderwater, 0);
  }
}

void CPlayerGun::ResetCharge(CStateManager& mgr, bool playAnimation) {
  if (mChargePhase != kCP_NotCharging && mChargePhase != kCP_ChargeRequested) {
    EnableChargeFx(mgr, false);
    StopChargeSound(mgr, false);
  }
  mPhazonChargeGenerator = rstl::auto_ptr< CElementGen >();
  mPhazonAbsorbFlashGenerator = rstl::auto_ptr< CElementGen >();
  GetPlayerFromAll(mgr)->GetPlayerState()->SetChargeBeamFactor(0.f);
  mRequestReturnToDefault = false;
  mChargePhase = kCP_NotCharging;
  mSeekerChargeState = kSCS_NotCharging;
  mChargeState = CPlayerState::kCS_Normal;
  mChargeRumbleTimer = 0.f;
  mAbsorbedPhazonShots = 0;
  if (mCurrentBeam != nullptr) {
    mCurrentBeam->OnChargeReset();
  }
  if (playAnimation) {
    PlayAnim(mgr, 0, false);
  }
}

void CPlayerGun::StopChargeSound(CStateManager& mgr, bool start) {
  if (mChargeSfx) {
    CSfxManager::SfxStop(mChargeSfx);
    mChargeSfx = CSfxHandle::NullHandle();
  }
  CRumbleManager* rumble = mgr.RumbleManager(mgr.MaskUIdNumPlayers(GetPlayerUniqueId()));
  if (mChargeRumbleHandle != -1) {
    rumble->StopRumble(mChargeRumbleHandle);
    mChargeRumbleHandle = -1;
  }
  if (start) {
    static const ushort sounds[2][4] = {{0xc2, 0x1fc6, 0x1fe0, 0x1fdb},
                                        {0x259a, 0x25a5, 0x25c3, 0x25b9}};
    int sound = sounds[mSoundSetIndex][mCurrentBeamId];
    if (!mgr.IsMultiplayer() && mSeekerChargeState != kSCS_NotCharging) {
      sound = 0x184;
    }
    mChargeSfx =
        PlaySfxForPlayer(nullptr, sound, mSoundVolume, CSfxManager::kAllAreas, mUnderwater, true);
    mChargeRumbleHandle = rumble->Rumble(mgr, kRFX_PlayerGunCharge, 1.f, kRP_Three);
  }
}

void CPlayerGun::EnableChargeFx(CStateManager& mgr, bool enable) {
  mCurrentBeam->ActivateCharge(enable, false);
  SetGunLightActive(enable, mgr);
  mCurrentBeam->EnableSecondaryFx(enable ? CGunWeapon::kSFT_Charge : CGunWeapon::kSFT_CancelCharge);
  mChargeEffectVisible = enable;
  if (enable) {
    mAuxMuzzleGenerators[mCurrentBeamId] =
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(mAuxMuzzleEffects[mCurrentBeamId]));
    mAuxMuzzleGenerators[mCurrentBeamId]->SetParticleEmission(true);
  } else {
    mAuxMuzzleGenerators[mCurrentBeamId] = rstl::auto_ptr< CElementGen >();
  }
  if (mgr.IsMultiplayer()) {
    GetPlayerFromAll(mgr)->SetMultiplayerBeamAuxParticlesEnabled(mgr, enable);
  }
}

void CPlayerGun::UpdateSeeker(float dt, CStateManager& mgr) {
  if (mInBigStrike) {
    mSeekerTargets.clear();
    return;
  }

  bool allowMissileFire = true;
  CPlayer* player = GetPlayer(mgr);
  CPlayerState* playerState = player->GetPlayerState();
  if (playerState->HasPowerUp(CPlayerState::kIT_SeekerLauncher)) {
    if (mSeekerChargeState == kSCS_NotCharging) {
      mSeekerChargeFactor = CMath::Clamp(0.f, mSeekerChargeFactor - kChargeDtFactor * dt, 1.f);
    } else if (mSeekerChargeState > kSCS_NotCharging && mSeekerChargeState < kSCS_FullyCharged) {
      mSeekerChargeFactor = CMath::Clamp(0.f, mSeekerChargeFactor + kChargeDtFactor * dt, 1.f);
    }
    if (mSeekerVisor != playerState->GetCurrentVisor()) {
      mCurrentSeekerTarget = kInvalidUniqueId;
      mSeekerLockTimer = 0.f;
      mAllSeekersLockedTime = 0.f;
      mSeekerTargets.clear();
    }
    for (int i = 0; i < mSeekerTargets.size(); ++i) {
      mSeekerTargets[i].second += dt;
    }
    if (mReleasedInputFlags & 2) {
      mSeekerChargeState = kSCS_Fire;
    }
    UpdateSeekerEffects(dt);

    switch (mSeekerChargeState) {
    case kSCS_NotCharging:
      if (!(mInputFlags & 2) || playerState->GetItemAmount(CPlayerState::kIT_Missile, true) == 0) {
        break;
      }
      mSeekerChargeState = kSCS_Requested;
      mSeekerChargeFactor = 0.f;
    case kSCS_Requested:
      if (!(playerState->GetChargeAnimStart() < mSeekerChargeFactor)) {
        break;
      }
      mSeekerChargeState = kSCS_Opening;
      if (!mMissileAnimActive) {
        PlayAnim(mgr, NWeaponTypes::kGAT_ToMissile, false);
        mMissileAnimActive = true;
      }
    case kSCS_Opening:
      if (mSeekerChargeFactor < kFactorMultiplierForBeamCombo) {
        break;
      }
      StopChargeSound(mgr, true);
      EnableSeekerFx(mgr, true);
      mSeekerChargeState = kSCS_Charging;
    case kSCS_Charging:
      if (mSeekerChargeFactor < 1.f) {
        break;
      }
      mSeekerChargeState = kSCS_FullyCharged;
    case kSCS_FullyCharged: {
      typedef rstl::reserved_vector< rstl::pair< TUniqueId, float >, 5 > TargetList;
      for (TargetList::iterator it = mSeekerTargets.begin(); it != mSeekerTargets.end();) {
        CActor* target = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(it->first)));
        bool remove = true;
        if (target && target->GetActive() == true &&
            target->GetMaterialList().HasMaterial(kMT_SeekerTarget) == true &&
            IsSeekerTargetInRange(*target, *player, mgr, 100.f) == true) {
          remove = false;
        }
        if (remove) {
          it = mSeekerTargets.erase(it);
        } else {
          ++it;
        }
      }
      if (mSeekerTargets.size() == mMaxSeekerTargets) {
        mAllSeekersLockedTime += dt;
      } else {
        mAllSeekersLockedTime = 0.f;
      }
      if (mSeekerTargets.size() >= playerState->GetItemAmount(CPlayerState::kIT_Missile, true) ||
          mSeekerTargets.size() >= mMaxSeekerTargets) {
        break;
      }

      bool allowDuplicate = false;
      TUniqueId targetId = GetTargetId(mgr);
      if (CSwarmBasics* swarm =
              TCastToPtr< CSwarmBasics >(const_cast< CEntity* >(mgr.GetObjectById(targetId)))) {
        const TUniqueId swarmTarget = swarm->GetSeekerTargetLockedOn();
        if (swarmTarget != kInvalidUniqueId) {
          targetId = swarmTarget;
        }
      }
      const CVector3f eyePosition = player->GetEyePosition();
      if (targetId != kInvalidUniqueId) {
        if (CActor* target =
                TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(targetId)))) {
          if (!target->GetMaterialList().HasMaterial(kMT_SeekerTarget) ||
              !IsSeekerTargetInRange(*target, *player, mgr, 100.f)) {
            break;
          }
          const CVector3f delta = target->GetAimPosition(mgr, 0.f) - eyePosition;
          const float distance = delta.Magnitude();
          if (mgr.RayStaticIntersection(eyePosition, delta.AsNormalized(), distance,
                                        skWeaponCollisionFilter)
                  .IsValid()) {
            break;
          }
        }
      }
      if (targetId == mCurrentSeekerTarget && targetId != kInvalidUniqueId) {
        mSeekerLockTimer += dt;
        if (mSeekerLockTimer > 0.5f) {
          allowDuplicate = true;
          mSeekerLockTimer = 0.f;
        }
      } else {
        mCurrentSeekerTarget = targetId;
        mSeekerLockTimer = 0.f;
      }

      if (targetId == kInvalidUniqueId) {
        const CTransform4f xf =
            mPointBlankWorldSurface ? mElbowWorldXf : mGunWorldXf * mBeamLocalXf;
        rstl::reserved_vector< TUniqueId, 1024 > nearList;
        CAABox bounds(CVector3f(-1.f, 0.f, -1.f), CVector3f(1.f, 100.f, 1.f));
        bounds = bounds.GetTransformedAABox(xf);
        mgr.BuildNearList(nearList, bounds, skSeekerTargetFilter, player);
        if (!nearList.empty()) {
          const CGameCamera* camera = player->GetCameraManager()->GetFirstPersonCamera();
          for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
               it != nearList.end(); ++it) {
            CActor* target = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(*it)));
            if (!target) {
              continue;
            }
            const uint visorFlags = target->GetTargetableVisorFlags();
            bool visible = true;
            switch (player->GetPlayerState()->GetCurrentVisor()) {
            case CPlayerState::kPV_Combat:
              visible = (visorFlags & 1) != 0;
              break;
            case CPlayerState::kPV_Echo:
              visible = (visorFlags & 8) != 0;
              break;
            case CPlayerState::kPV_Dark:
              visible = (visorFlags & 2) != 0;
              break;
            case CPlayerState::kPV_Scan:
              visible = (visorFlags & 4) != 0;
              break;
            }
            if (!visible) {
              continue;
            }
            if (CBouncyGrenade* grenade = TCastToPtr< CBouncyGrenade >(target)) {
              if (grenade->GetFlags() & 8) {
                continue;
              }
            }
            bool alreadyLocked = false;
            for (int i = 0; i < mSeekerTargets.size(); ++i) {
              if (*it == mSeekerTargets[i].first) {
                alreadyLocked = true;
                break;
              }
            }
            if (alreadyLocked) {
              continue;
            }
            const CVector3f orbitPosition = target->GetOrbitPosition(mgr);
            const CVector3f screenPosition = camera->ConvertToScreenSpace(orbitPosition);
            const float screenY = screenPosition.GetY() * CGraphics::GetViewport().mHeight * 0.5f;
            const float screenX = screenPosition.GetX() * CGraphics::GetViewport().mWidth * 0.5f;
            if (screenX * screenX + screenY * screenY < 2500.f &&
                mgr.RayCollideWorld(eyePosition, orbitPosition, nearList, skWeaponCollisionFilter,
                                    target)) {
              targetId = *it;
              break;
            }
          }
        }
      }
      if (targetId != kInvalidUniqueId) {
        if (!allowDuplicate) {
          for (int i = 0; i < mSeekerTargets.size(); ++i) {
            if (targetId == mSeekerTargets[i].first) {
              targetId = kInvalidUniqueId;
              break;
            }
          }
        }
        if (targetId != kInvalidUniqueId) {
          mSeekerVisor = playerState->GetCurrentVisor();
          mSeekerTargets.push_back(rstl::pair< TUniqueId, float >(targetId, 0.f));
          PlaySfxForPlayer(GetPlayer(mgr), 0x1db, player->GetSoundPan(CPlayer::kMSP_4),
                           mgr.GetNextAreaId().Value(), mUnderwater, false);
        }
      }
      break;
    }
    case kSCS_Fire:
      if (!mSeekerTargets.empty()) {
        const float pitch = mSeekerTargets.size() > 1 ? 45.f : 0.f;
        const float yaw = 180.f + 180.f / mSeekerTargets.size();
        const float yawStep = 360.f / mSeekerTargets.size();
        for (int i = 0; i < mSeekerTargets.size(); ++i) {
          uint attributes = 0x400000;
          ushort sound = CSfxManager::kInternalInvalidSfxId;
          if (i == 0) {
            if (!mgr.IsMultiplayer()) {
              if (mSeekerTargets.size() == 1) {
                sound = 0x17f;
              } else if (mSeekerTargets.size() > 1 && mSeekerTargets.size() < 4) {
                sound = 0x181;
              } else if (mSeekerTargets.size() >= 4 && mSeekerTargets.size() < 6) {
                sound = 0x182;
              }
            }
          } else {
            attributes = 0x1c00000;
          }
          const CTransform4f spread =
              CTransform4f::RotateY(CRelAngle::FromDegrees(yawStep * i + yaw)) *
              CTransform4f::RotateX(CRelAngle::FromDegrees(pitch));
          FireSecondary(dt, mgr, mSeekerTargets[i].first, attributes, &spread, sound);
        }
        allowMissileFire = false;
      }
      ResetSeeker(mgr);
      break;
    default:
      break;
    }
  }

  if (mMissileExitTimer > 0.f && mSeekerChargeState == kSCS_NotCharging) {
    mMissileExitTimer -= dt;
  }
  const CAnimData& animData = *mCurrentBeam->GetSolidModelData().GetAnimationData();
  if (mMissileState == kMS_Ready) {
    if (allowMissileFire && (mPressedInputFlags & 2)) {
      FireSecondary(dt, mgr, kInvalidUniqueId, 0, nullptr, CSfxManager::kInternalInvalidSfxId);
    }
  } else if (!animData.IsAnimTimeRemaining(0.001f, rstl::string_l("Whole Body"))) {
    switch (mMissileState) {
    case kMS_Reloading:
      mMissileState = kMS_Ready;
      break;
    case kMS_Shot:
      if (playerState->GetItemAmount(CPlayerState::kIT_Missile, true) < 1) {
        mMissileState = kMS_Ready;
      } else {
        PlayAnim(mgr, NWeaponTypes::kGAT_FromBeam, false);
        mMissileState = kMS_Reloading;
      }
      break;
    default:
      break;
    }
  }
}

void CPlayerGun::UpdateSeekerEffects(float dt) {
  if (!mChargeEffectVisible) {
    return;
  }
  if (mMissileAuxMuzzleGenerator.get() != nullptr) {
    mMissileAuxMuzzleGenerator->SetParticleEmission(mSeekerTargets.size() == mMaxSeekerTargets);
  }
  const int oldCount = mSeekerMuzzleGenerators.size() - 1;
  if (oldCount < mSeekerTargets.size()) {
    for (int i = 0; i < mSeekerMuzzleGenerators.size(); ++i) {
      if (mSeekerFadeRates[i] < 0.f) {
        mSeekerFadeRates[i] = 2.f * dt;
      }
    }
    for (int i = mSeekerMuzzleGenerators.size(); i < mSeekerTargets.size(); ++i) {
      mSeekerMuzzleGenerators.push_back(
          rstl::auto_ptr< CElementGen >(rs_new CElementGen(mSeekerMuzzleEffects[i])));
      mSeekerMuzzleGenerators.back()->SetParticleEmission(true);
    }
  } else if (mSeekerTargets.size() < oldCount) {
    for (int i = mSeekerTargets.size() + 1; i < mSeekerMuzzleGenerators.size(); ++i) {
      if (mSeekerFadeRates[i] >= 0.f) {
        mSeekerFadeRates[i] = 3.f * -dt;
      }
    }
  }

  for (int i = mSeekerMuzzleGenerators.size() - 1; i >= 0; --i) {
    if (mSeekerFadeRates[i] != 0.f) {
      CColor color = mSeekerMuzzleGenerators[i]->GetModulationColor();
      float alpha = color.GetAlpha() + mSeekerFadeRates[i];
      if (alpha < 0.f) {
        alpha = 0.f;
      } else if (alpha > 1.f) {
        alpha = 1.f;
      }
      color.SetAlpha(alpha);
      mSeekerMuzzleGenerators[i]->SetModulationColor(color);
      if (alpha == 0.f || alpha == 1.f) {
        mSeekerFadeRates[i] = 0.f;
      }
      if (alpha == 0.f) {
        mSeekerMuzzleGenerators.erase(mSeekerMuzzleGenerators.begin() + i);
      }
    }
  }
  const int newCount = mSeekerMuzzleGenerators.size() - 1;
  if (oldCount != newCount) {
    CSfxManager::PitchBend(mChargeSfx, newCount * 0x7a0 + 0x2000);
  }
}

void CPlayerGun::EnableSeekerFx(CStateManager& mgr, bool enable) {
  mChargeEffectVisible = enable;
  SetGunLightActive(enable, mgr);
  mSeekerSecondaryFx = enable ? CGunWeapon::kSFT_Charge : CGunWeapon::kSFT_CancelCharge;
  if (mSeekerSecondaryFx == CGunWeapon::kSFT_Charge) {
    mMissileSecondaryGenerator =
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(*mMissileSecondaryEffect));
    mMissileSecondaryGenerator->SetGlobalScale(mScale);
  } else {
    if (mSeekerSecondaryFx != CGunWeapon::kSFT_None &&
        mMissileSecondaryGenerator.get() != nullptr) {
      mMissileSecondaryGenerator->SetParticleEmission(false);
    }
    mSeekerSecondaryFx = CGunWeapon::kSFT_None;
  }

  if (enable) {
    mMissileAuxMuzzleGenerator =
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(*mMissileAuxMuzzleEffect));
    mMissileAuxMuzzleGenerator->SetParticleEmission(false);
    mSeekerMuzzleGenerators.push_back(
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(mSeekerMuzzleEffects[0])));
    mSeekerMuzzleGenerators[0]->SetParticleEmission(true);
  } else {
    mMissileAuxMuzzleGenerator = rstl::auto_ptr< CElementGen >();
    mSeekerMuzzleGenerators.clear();
  }
}

void CPlayerGun::FireSecondary(float dt, CStateManager& mgr, TUniqueId target, uint attributes,
                               const CTransform4f* transform, ushort sound) {
  mFidget.ResetAll();
  CPlayer& player = *GetPlayerFromAll(mgr);
  CPlayerState& playerState = *player.GetPlayerState();
  const int missiles = playerState.GetItemAmount(CPlayerState::kIT_Missile, true);
  CPlayerState::EItemType ammoTypeA = CPlayerState::kIT_Invalid;
  CPlayerState::EItemType ammoTypeB = CPlayerState::kIT_Invalid;
  int ammoCost = 0;
  CPlayerState::EBeamId beam = mCurrentBeamId;
  EWeaponType weaponType = mCurrentBeam->GetType();
  CPlayerState::EChargeStage chargeState = mChargeState;

  if (mgr.IsMultiplayer() && playerState.HasPowerUp(CPlayerState::kIT_SuperMissile)) {
    beam = CPlayerState::kBI_Power;
    weaponType = kWT_Power;
    chargeState = CPlayerState::kCS_Charged;
    if (missiles < 1) {
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyMissileSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
      CPlayerState::CPowerUp& superMissile = playerState.PowerUp(CPlayerState::kIT_SuperMissile);
      superMissile.mTimeLeft = 0.f;
      superMissile.mAmount = 0;
      superMissile.mCapacity = 0;
      mgr.DisplayAlertAboutOutOfAmmo(player, CPlayerState::kIT_SuperMissile);
      return;
    }
    playerState.DecrementAmmoAndDisplayAlertIfOut(mgr, CPlayerState::kIT_Missile, 1);
  } else {
    bool canFire = true;
    if (missiles < 1) {
      canFire = false;
    } else if (mComboFiring &&
               !GetBeamAmmoTypeAndCosts(mComboFiring, mgr, ammoTypeA, ammoTypeB, ammoCost)) {
      canFire = false;
    }
    if (!canFire) {
      mMissileExitTimer = 7.f;
      PlaySfxForPlayer(GetPlayer(mgr), skEmptyMissileSfx[mSoundSetIndex], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
      return;
    }
    const int missileCost = mComboFiring ? playerState.GetMissileCostForAltAttack() : 1;
    if (missiles < missileCost || mgr.GetWeaponIdCount(mPlayerUniqueId, kWT_Missile) > 5) {
      return;
    }
    playerState.DecrementAmmoAndDisplayAlertIfOut(mgr, CPlayerState::kIT_Missile, missileCost);
    if (ammoTypeA != CPlayerState::kIT_Invalid) {
      playerState.DecrementAmmoAndDisplayAlertIfOut(mgr, ammoTypeA, ammoCost);
      if (ammoTypeB != CPlayerState::kIT_Invalid) {
        playerState.DecrementAmmoAndDisplayAlertIfOut(mgr, ammoTypeB, ammoCost);
      }
    }
  }

  const TUniqueId targetId = target == kInvalidUniqueId ? GetTargetId(mgr) : target;
  CTransform4f xf(mPointBlankWorldSurface ? mElbowWorldXf : mGunWorldXf * mBeamLocalXf);
  if (!mPointBlankWorldSurface && mGunStrikeCooldownTimer <= 0.f) {
    const CVector3f position = xf.GetTranslation();
    xf = mAssistAimXf;
    xf.SetTranslation(position);
  }
  xf.AddTranslation(GetPlayer(mgr)->GetCameraManager()->GetGlobalCameraTranslation(mgr, true));
  if (transform != nullptr) {
    xf = xf * *transform;
  }
  mAuxWeapon->Fire(dt, mUnderwater, beam, chargeState, xf, mgr, weaponType, targetId, attributes,
                   sound);
  mSecondaryCooldown = mMissileShotInterval;
  if ((attributes & 0x01000000) == 0) {
    if (playerState.GetItemAmount(CPlayerState::kIT_DoubleDamage, true)) {
      PlaySfxForPlayer(GetPlayer(mgr), 0x2612, mSoundVolume, mgr.GetNextAreaId().Value(),
                       mUnderwater, false);
    }
    mgr.InformListeners(mGunWorldXf.GetTranslation(), kLNT_PlayerFire);
  }
  if (!mComboFiring && (attributes & 0x01000000) == 0) {
    PlayAnim(mgr,
             mMissileAnimActive ? NWeaponTypes::kGAT_MissileReload
                                : NWeaponTypes::kGAT_MissileShoot,
             false);
    mMissileAnimActive = true;
    mMissileState = kMS_Shot;
    mMissileExitTimer = 7.f;
  }
  mFiredWeaponFlags |= 2;
}

void CPlayerGun::UpdateAuxWeapons(float dt, const CTransform4f& transform, CStateManager& mgr) {
  const CVector3f firePosition = mGunWorldXf * mBeamLocalXf.GetTranslation();
  const CVector3f cameraTranslation =
      GetPlayer(mgr)->GetCameraManager()->GetGlobalCameraTranslation(mgr, true);
  const bool active =
      mAuxWeapon->UpdateComboFx(dt, mScale, firePosition + cameraTranslation, transform, mgr);
  if (mComboFiring && mChargePhase == kCP_ComboFired && !active) {
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_CancelCharge);
    if (AnimOver(mgr, 0.f)) {
      mComboFiring = false;
    }
  }
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool deactivate) {
  ReturnArmAndGunToDefault(mgr, false);
  mAuxWeapon->StopComboFx(mgr, deactivate);
  mCurrentBeam->EnableSecondaryFx(deactivate ? CGunWeapon::kSFT_None
                                             : CGunWeapon::kSFT_CancelCharge);
}

void CPlayerGun::DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode& node,
                                 EUserEventType type) {
  switch (type) {
  case kUE_Projectile:
    if (mChargePhase == kCP_ComboAnimating) {
      FireSecondary(dt, mgr, kInvalidUniqueId, 0, nullptr, CSfxManager::kInternalInvalidSfxId);
      mComboTransferGenerator = rstl::auto_ptr< CElementGen >();
      EnableChargeFx(mgr, false);
      mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_ToCombo);
      mChargePhase = kCP_ComboFired;
    }
    break;
  case kUE_Delete:
  case kUE_DamageOn:
    break;
  }
}

void CPlayerGun::DoUserAnimEvents(float dt, CStateManager& mgr) {
  const CPlayer& player = *GetPlayer(mgr);
  const int area = player.GetCurrentAreaId().Value();
  const CAnimData& animData = *mCurrentBeam->GetSolidModelData().GetAnimationData();
  const CGameCamera& camera = *player.GetCameraManager()->GetCurrentCamera(mgr, true);
  const CVector3f origin = mTransform.GetTranslation();
  const CVector3f posToCamera = camera.GetTranslation() - origin;

  int soundCount = 0;
  const CSoundPOINode* sounds = animData.GetSoundPOIList(soundCount);
  for (int i = 0; i < soundCount; ++i) {
    const CSoundPOINode& sound = sounds[i];
    if (sound.GetPoiType() == kPT_Sound &&
        (sound.GetCharacterIndex() == -1 ||
         sound.GetCharacterIndex() == animData.GetCharacterIndex())) {
      NWeaponTypes::do_sound_event(mAnimSfx, mAnimSfxPitch, false, sound.GetSoundId(),
                                   sound.GetWeight(), sound.GetFlags(), sound.GetFallOff(),
                                   sound.GetMaxDistance(), 0x14, CAudioSys::kMaxVolume, posToCamera,
                                   origin, area, mSoundVolume, mgr);
    }
  }

  int intCount = 0;
  const CInt32POINode* nodes = animData.GetInt32POIList(intCount);
  for (int i = 0; i < intCount; ++i) {
    const CInt32POINode& node = nodes[i];
    switch (node.GetPoiType()) {
    case kPT_SoundInt32:
      if (node.GetCharacterIndex() == -1 ||
          node.GetCharacterIndex() == animData.GetCharacterIndex()) {
        NWeaponTypes::do_sound_event(
            mAnimSfx, mAnimSfxPitch, false, node.GetValue(), node.GetWeight(), node.GetFlags(),
            0.1f, 150.f, 0x14, CAudioSys::kMaxVolume, posToCamera, origin, area, mSoundVolume, mgr);
      }
      break;
    case kPT_UserEvent:
      DoUserAnimEvent(dt, mgr, node, static_cast< EUserEventType >(node.GetValue()));
      break;
    default:
      break;
    }
  }
}

void CPlayerGun::SetGunLightActive(bool active, CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }
  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (light != nullptr) {
    light->SetActive(active);
    if (active) {
      CElementGen* generator =
          mMissileAnimActive
              ? (mSeekerMuzzleGenerators.empty() ? nullptr : mSeekerMuzzleGenerators[0].get())
              : mCurrentBeam->GetMuzzleFx(1);
      if (generator != nullptr && generator->SystemHasLight()) {
        CLight muzzleLight = generator->GetLight();
        muzzleLight.SetColor(CColor::Black());
        light->SetLight(muzzleLight);
      }
    }
  }
}

void CPlayerGun::UpdateGunLight(const CTransform4f& transform, CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    return;
  }
  if (mChargePhase == kCP_NotCharging && mSeekerChargeState == kSCS_NotCharging) {
    return;
  }
  CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  if (light != nullptr && light->GetActive()) {
    light->SetTransform(transform);
    light->SetTranslation(transform.GetTranslation());
    CElementGen* generator =
        mMissileAnimActive
            ? (mSeekerMuzzleGenerators.empty() ? nullptr : mSeekerMuzzleGenerators[0].get())
            : mCurrentBeam->GetMuzzleFx(1);
    if (generator != nullptr && generator->SystemHasLight()) {
      CLight muzzleLight = generator->GetLight();
      muzzleLight.SetColor(
          CColor(CColor::Lerp(0u, muzzleLight.GetColor().GetColor_u32(),
                              GetPlayer(mgr)->GetPlayerState()->GetChargeBeamFactor())));
      light->SetLight(muzzleLight);
    }
  }
}

void CPlayerGun::SetBeam(CPlayerState::EBeamId beam, CStateManager& mgr) {
  for (int i = 0; i < 4; ++i) {
    mSelectableBeams[i]->InitializeResources(mgr);
  }
  mNextBeamId = beam;
  mCurrentBeamId = beam;
  mCurrentBeam = mSelectableBeams[beam];
  mCurrentBeam->Load(mgr, true);
  mCurrentBeam->SetRainSplashGenerator(mRainSplashGenerator.get());
  mAuxWeapon->Load(beam, mgr);
}

void CPlayerGun::InitMuzzleData(CStateManager& mgr) {
  static const char* const auxNames[] = {"EmptyMuzzle", "IceAuxMuzzle", "WaveAuxMuzzle",
                                         "PlasmaAuxMuzzle"};
  static const char* const seekerNames[] = {"MissileMuzzle", "MissileMuzzle1", "MissileMuzzle2",
                                            "MissileMuzzle3", "MissileMuzzle4"};
  if (!mgr.IsMultiplayer()) {
    mHoloTransitionGenerator =
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(gpSimplePool->GetObj("holoTransition")));
    mHoloTransitionGenerator->SetParticleEmission(true);
    mRender = &CPlayerGun::RenderGunWithHologram;
    mSoundSetIndex = 0;
    mMaxSeekerTargets = 5;
  } else {
    mRender = &CPlayerGun::RenderGun;
    mGunMorph = CGunMorph(0.0625f, 0.0625f);
    mSoundSetIndex = 1;
    mMaxSeekerTargets = 3;
  }
  mSeekerFadeRates.resize(mMaxSeekerTargets, 0.f);
  for (int i = 0; i < 4; ++i) {
    mAuxMuzzleEffects.push_back(TLockedToken< CGenDescription >(gpSimplePool->GetObj(auxNames[i])));
    rstl::auto_ptr< CElementGen > generator(rs_new CElementGen(mAuxMuzzleEffects[i]));
    generator->SetParticleEmission(false);
    mAuxMuzzleGenerators.push_back(generator);
  }
  for (int i = 0; i < mMaxSeekerTargets; ++i) {
    mSeekerMuzzleEffects.push_back(
        TCachedToken< CGenDescription >(gpSimplePool->GetObj(seekerNames[i])));
  }
  mMissileAuxMuzzleEffect =
      TCachedToken< CGenDescription >(gpSimplePool->GetObj("MissileAuxMuzzle"));
  mMissileSecondaryEffect = TCachedToken< CGenDescription >(gpSimplePool->GetObj("Missile2nd"));
}

void CPlayerGun::InitBombData() {
  for (int i = 0; i < 2; ++i) {
    mBombEffects.push_back(rstl::reserved_vector< TToken< CGenDescription >, 2 >());
  }
  TToken< CGenDescription > bombSet = gpSimplePool->GetObj("BombSet");
  TToken< CGenDescription > bombExplosion = gpSimplePool->GetObj("BombExplo");
  mBombEffects[0].push_back(bombSet);
  mBombEffects[0].push_back(bombExplosion);
  TToken< CGenDescription > powerBombExplosion = gpSimplePool->GetObj("PowerBombExplo");
  mBombEffects[1].push_back(powerBombExplosion);
  mBombEffects[1].push_back(powerBombExplosion);
}

void CPlayerGun::InitBeamData() {
  mSelectableBeams[0] = mPowerBeam.get();
  mSelectableBeams[1] = mDarkBeam.get();
  mSelectableBeams[2] = mLightBeam.get();
  mSelectableBeams[3] = mAnnihilatorBeam.get();
  mCurrentBeam = mSelectableBeams[0];
}

void CPlayerGun::ChangeWeapon(CStateManager& mgr) {
  if (mOutgoingBeam != nullptr && mOutgoingBeam != mCurrentBeam) {
    mOutgoingBeam->Unload(mgr);
  }
  mLoadingBeam = mSelectableBeams[mNextBeamId];
  mCurrentBeam->EnableFx(false);
  mCurrentBeam->ReleaseResources(mgr);
  mMuzzleEffectVisTimer = 0.f;
  mBeamLoadDelayFrames = mgr.IsMultiplayer() ? 0 : 2;
  PlayBeamFireSfx(mgr, *GetPlayerFromAll(mgr), true);
  mGunMorph.StartWipe(CGunMorph::kMD_In);
}

bool CPlayerGun::ProcessGunMorph(float dt, CStateManager& mgr) {
  static const ushort beamSounds[2][4] = {{0, 0x1fd2, 0x1fd6, 0x1fd4}, {0, 0x2596, 0x25dc, 0x25d9}};
  static const ushort wipeSounds[2][2] = {{0xca, 0xcb}, {0x25cd, 0x25ce}};
  const CGunMorph::EGunState gunState = mGunMorph.mGunState;
  CPlayer* player = GetPlayer(mgr);
  const bool unmorphed = player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
  CPlayerState* playerState = GetPlayerFromAll(mgr)->GetPlayerState();
  const bool cinematic = player->GetCameraManager()->IsInCinematicCamera();
  switch (gunState) {
  case CGunMorph::kGS_InWipeDone:
    if (mCurrentBeamId != mNextBeamId && mLoadingBeam != nullptr) {
      mCurrentBeam->SetModelTouchEnabled(false);
      if (!mBeamLoadRequested && --mBeamLoadDelayFrames <= 0 && mLoadingBeam != mCurrentBeam) {
        mBeamLoadRequested = true;
        mLoadingBeam->Load(mgr, false);
        mAuxWeapon->Load(mNextBeamId, mgr);
      }
      if (!unmorphed) {
        mLoadingBeam->Touch(mgr);
      }
      gpMain->SetFrameTimeMinimum(1000);
      mAuxWeapon->LoadIdle();
      mLoadingBeam->Update(dt, mgr);
      if (mLoadingBeam->IsLoaded() && mAuxWeapon->IsLoaded()) {
        mBeamLoadDelayFrames = 2;
        mBeamLoadRequested = false;
        mOutgoingBeam = mLoadingBeam == mCurrentBeam ? nullptr : mCurrentBeam;
        mLoadingBeam = nullptr;
        mCurrentBeamId = mNextBeamId;
        mCurrentBeam = mSelectableBeams[mNextBeamId];
        mGunMorph.mWeaponChanged = true;
        playerState->SetCurrentBeam(mNextBeamId);
      }
    }
    break;
  case CGunMorph::kGS_InWipe:
  case CGunMorph::kGS_OutWipe:
    if (mHoloTransitionGenerator.get() != nullptr) {
      mHoloTransitionGenerator->SetGlobalOrientation(CTransform4f::Identity());
      mHoloTransitionGenerator->SetGlobalTranslation(CVector3f(0.f, mGunMorph.mYLerp, 0.f));
      mHoloTransitionGenerator->Update(dt);
    }
    break;
  }

  switch (mGunMorph.Update(0.2f, 1.292392f, dt, *player)) {
  case CGunMorph::kWE_OutWipeStarted:
    if (!cinematic) {
      PlaySfxForPlayer(GetPlayer(mgr), wipeSounds[mSoundSetIndex][1], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
    }
    break;
  case CGunMorph::kWE_OutWipeFinished:
    if (mOutgoingBeam != nullptr && mOutgoingBeam != mCurrentBeam) {
      mOutgoingBeam->Unload(mgr);
      mOutgoingBeam = nullptr;
    }
    if (unmorphed && !cinematic) {
      PlaySfxForPlayer(GetPlayer(mgr), beamSounds[mSoundSetIndex][mCurrentBeamId], mSoundVolume,
                       mgr.GetNextAreaId().Value(), mUnderwater, false);
    }
    mCurrentBeam->SetRainSplashGenerator(mRainSplashGenerator.get());
    mCurrentBeam->EnableFx(true);
    PlayAnim(mgr, 11, false);
    if (mInFreeLook) {
      mGrappleArm->SetStateFlags(CGrappleArm::kSF_FreeLook);
    } else if (mGrappleArm->GetStateFlags() != 0) {
      ReturnToDefault(mgr, false);
    }
    return true;
  }
  return false;
}

void CPlayerGun::UpdateBeamChange(float dt, CStateManager& mgr) {
  switch (mBeamChangeState) {
  case kBCS_Close:
    if (AnimOver(mgr, 0.f)) {
      ChangeWeapon(mgr);
      mBeamChangeState = kBCS_Morph;
    }
    break;
  case kBCS_Morph:
    if (ProcessGunMorph(dt, mgr)) {
      mBeamChangeState = kBCS_Open;
    }
    break;
  case kBCS_Open:
    if (AnimOver(mgr, 0.f)) {
      mBeamChangeState = kBCS_Idle;
    }
    break;
  default:
    break;
  }
}

void CPlayerGun::HandleBeamChange(const CFinalInput& input, CStateManager& mgr) {
  static const CPlayerState::EItemType beamItems[] = {
      CPlayerState::kIT_PowerBeam, CPlayerState::kIT_DarkBeam, CPlayerState::kIT_LightBeam,
      CPlayerState::kIT_AnnihilatorBeam};
  static const CControlMapper::ECommands beamCommands[] = {
      CControlMapper::kC_PowerBeam, CControlMapper::kC_IceBeam, CControlMapper::kC_WaveBeam,
      CControlMapper::kC_PlasmaBeam};
  static const ushort sounds[2] = {0xbf, 0x25b0};
  CPlayer* player = GetPlayerFromAll(mgr);
  CPlayerState* state = player->GetPlayerState();
  float maxInput = 0.f;
  int beam = -1;
  for (int i = 0; i < 4; ++i) {
    if (state->HasPowerUp(beamItems[i])) {
      const float value = player->GetControlMapper().GetAnalogInput(beamCommands[i], input);
      if (value > 0.65f && value > maxInput) {
        maxInput = value;
        beam = i;
      }
    }
  }
  if (mNextBeamId != state->GetCurrentBeam()) {
    beam = state->GetCurrentBeam();
  }
  if (beam <= -1) {
    return;
  }
  if (mCurrentBeamId != beam && state->HasPowerUp(beamItems[beam])) {
    mNextBeamId = CPlayerState::EBeamId(beam);
    PlayAnim(mgr, NWeaponTypes::kGAT_ToBeam, false);
    if ((mInFreeLook || mAuxWeapon->IsComboFxActive(mgr)) &&
        (mGrappleArm->GetStateFlags() & CGrappleArm::kSF_GunChanging) == 0) {
      mGrappleArm->SetStateFlags(CGrappleArm::kSF_GunChanging);
    }
    mCurrentBeam->EnableSecondaryFx(CGunWeapon::kSFT_None);
    mInvalidSfx = CSfxHandle::NullHandle();
    mBeamChangeState = kBCS_Close;
    mInterruptEvent = true;
  } else if (state->HasPowerUp(beamItems[beam])) {
    if (!mMissileMode) {
      if (!CSfxManager::IsPlaying(mInvalidSfx)) {
        mInvalidSfx = PlaySfxForPlayer(GetPlayer(mgr), sounds[mSoundSetIndex], mSoundVolume,
                                       mgr.GetNextAreaId().Value(), mUnderwater, false);
      }
    } else {
      mMissileExitTimer = 0.f;
      mInvalidSfx = CSfxHandle::NullHandle();
    }
  }
}

void CPlayerGun::EnterFreeLook(CStateManager& mgr) {
  mGunMotion->PlayPasAnim(SamusGun::kAS_FreeLook, mgr, 0.f, false);
  mGrappleArm->SetStateFlags(CGrappleArm::kSF_FreeLook);
}

void CPlayerGun::ReturnArmAndGunToDefault(CStateManager& mgr, bool force) {
  if (force || !mInFreeLook) {
    ReturnToDefault(mgr, false);
  }
  if (!mGunMotionFidgeting) {
    mCurrentBeam->ReturnToDefault(mgr, mBeamChangeState != kBCS_Idle);
  }
  mGunMotionFidgeting = false;
}

void CPlayerGun::ReturnToDefault(CStateManager& mgr, bool bigStrikeReset) {
  mGunMotion->ReturnToDefault(mgr, bigStrikeReset);
  mGrappleArm->ReturnToDefault(mgr, 0.f, false);
}

void CPlayerGun::SetFidgetAnimBits(int animSet, bool holster) {
  mFidgetAnimBits = 0;
  if (holster) {
    mFidgetAnimBits = 2;
    return;
  }
  switch (mFidget.GetType()) {
  case SamusGun::kFT_Minor:
    mFidgetAnimBits = 1;
    if (animSet > 0 && animSet < 2) {
      mFidgetAnimBits |= 4;
    }
    break;
  case SamusGun::kFT_Major:
    switch (animSet) {
    case 4:
    case 5:
      mFidgetAnimBits = 1;
      break;
    default:
      mFidgetAnimBits = 2;
      break;
    }
    mFidgetAnimBits |= 4;
    break;
  }
}

bool CPlayerGun::IsFidgetLoaded() {
  int loaded = 0;
  if ((mFidgetAnimBits & 1) == 1 && mGunMotion->GunController().IsFidgetLoaded()) {
    loaded |= 1;
  }
  if ((mFidgetAnimBits & 2) == 2 && mCurrentBeam->IsFidgetLoaded()) {
    loaded |= 2;
  }
  if ((mFidgetAnimBits & 4) == 4) {
    CGunController* controller = mGrappleArm->GunController();
    if (controller != nullptr && controller->IsFidgetLoaded()) {
      loaded |= 4;
    }
  }
  return loaded == mFidgetAnimBits;
}

void CPlayerGun::UnLoadFidget() {
  if ((mFidgetAnimBits & 1) == 1) {
    mGunMotion->GunController().UnLoadFidget();
  }
  if ((mFidgetAnimBits & 2) == 2) {
    mCurrentBeam->UnLoadFidget();
  }
  if ((mFidgetAnimBits & 4) == 4) {
    CGunController* controller = mGrappleArm->GunController();
    if (controller != nullptr) {
      controller->UnLoadFidget();
    }
  }
  mFidgetAnimBits = 0;
}

void CPlayerGun::AsyncLoadFidget(CStateManager& mgr) {
  const SamusGun::EFidgetType type = mFidget.GetType();
  const int animSet = mFidget.GetAnimSet();
  bool holster = mFidget.GetState() == CFidget::kS_HolsterBeam;
  SetFidgetAnimBits(animSet, holster);
  if ((mFidgetAnimBits & 1) == 1) {
    mGunMotion->GunController().LoadFidgetAnimAsync(mgr, type, mCurrentBeamId, animSet);
  }
  if ((mFidgetAnimBits & 2) == 2) {
    mCurrentBeam->AsyncLoadFidget(mgr, holster ? SamusGun::kFT_Minor : type, animSet);
  }
  if ((mFidgetAnimBits & 4) == 4) {
    CGunController* controller = mGrappleArm->GunController();
    if (controller != nullptr) {
      controller->LoadFidgetAnimAsync(
          mgr, type, type != SamusGun::kFT_Minor ? mCurrentBeamId : CPlayerState::kBI_Power,
          animSet);
    }
  }
  mFidget.SetLoading();
}

void CPlayerGun::EnterFidget(CStateManager& mgr) {
  const SamusGun::EFidgetType type = mFidget.GetType();
  const int animSet = mFidget.GetAnimSet();
  if ((mFidgetAnimBits & 1) == 1) {
    mGunMotion->EnterFidget(mgr, type, animSet);
    mGunMotionFidgeting = true;
  } else {
    mGunMotionFidgeting = false;
  }
  if ((mFidgetAnimBits & 2) == 2) {
    mCurrentBeam->EnterFidget(mgr, type, animSet);
  }
  if ((mFidgetAnimBits & 4) == 4) {
    mGrappleArm->EnterFidget(
        mgr, type, type != SamusGun::kFT_Minor ? mCurrentBeamId : CPlayerState::kBI_Power, animSet);
  }
  UnLoadFidget();
  mFidget.DoneLoading();
}

void CPlayerGun::UpdateGunIdle(float dt, CStateManager& mgr) {
  CPlayer* player = GetPlayer(mgr);
  if (!player->IsInFreeLook() && player->GetOrbitState() == CPlayer::kOS_NoOrbit &&
      mBeamChangeState == kBCS_Idle && !mInBigStrike) {
    const bool moving = player->GetVelocityWR().Magnitude() > 0.01f ||
                        player->GetAngularVelocityOR().GetVector().GetZ() != 0.f;
    mFidget.Update(mInputFlags, moving, mGunStrikeCooldownTimer > 0.f, dt, mgr, *player);
    if (mFidget.GetState() == CFidget::kS_NoFidget) {
      if (!moving || mInputFlags != 0) {
        if (mGunMotionState != SamusGun::kAS_Idle) {
          mGunMotion->PlayPasAnim(SamusGun::kAS_Idle, mgr, 0.f, false);
          mGunMotionState = SamusGun::kAS_Idle;
        }
      } else {
        if (mGunStrikeCooldownTimer <= 0.f && mIdleWanderDelayTimer <= 0.f) {
          mIdleWanderDelayTimer = 8.f;
          mGunMotion->PlayPasAnim(SamusGun::kAS_Wander, mgr, 0.f, false);
          mGunMotionState = SamusGun::kAS_Wander;
        }
        mIdleWanderDelayTimer -= dt;
      }
    }
  } else {
    mFidget.ResetAll();
  }
}

void CPlayerGun::UpdateGunMotion(float dt, CStateManager& mgr) {
  mMotionState.Update(mFiring, dt, mGunWorldXf, mgr);
  CPlayer* player = GetPlayer(mgr);
  if (player->GetOrbitState() == CPlayer::kOS_OrbitObject && GetTargetId(mgr) != kInvalidUniqueId &&
      !mComboFiring) {
    if (mMotionState.mMotionState == CMotionState::kMS_Zero && !mComboFiring) {
      mMotionState.mMotionState = CMotionState::kMS_LockOn;
      ReturnArmAndGunToDefault(mgr, true);
    }
  } else if (mMotionState.mMotionState != CMotionState::kMS_CancelLockOn &&
             mMotionState.mMotionState != CMotionState::kMS_Zero) {
    mMotionState.mMotionState = CMotionState::kMS_CancelLockOn;
  }
}

void CPlayerGun::UpdateTimers(float dt) {
  if (mBombReloadTimer > 0.f) {
    mBombReloadTimer -= dt;
    if (mBombReloadTimer <= 0.f) {
      mBombCount = 3;
      mBombReloadTimer = 0.f;
    }
  }
  if (mMuzzleEffectVisTimer > 0.f) {
    mMuzzleEffectVisTimer -= dt;
  }
  if (mRapidFireDecayTimer >= 0.2f) {
    mRapidFireDecayTimer = 0.f;
    if (mRapidFireShots > 0) {
      --mRapidFireShots;
    }
  } else {
    mRapidFireDecayTimer += dt;
  }
  if ((mInputFlags & 0xd) == 0) {
    if (mTimeSinceFire < 2.f) {
      mTimeSinceFire += dt;
      if (mTimeSinceFire > 1.f) {
        mRapidFireShots = 0;
        mShotSmokeTimer = 0.f;
      }
    }
  } else {
    mTimeSinceFire = 0.f;
  }
  if (mInBigStrike) {
    if (mBigStrikeTimer <= 0.f) {
      if (mGunMotionReturningFromStrike) {
        if (!mGunMotion->GetModelData().GetAnimationData()->IsAnimTimeRemaining(
                0.001f, rstl::string("Whole Body"))) {
          mInBigStrike = false;
          mGunMotionReturningFromStrike = false;
        }
      } else {
        mBigStrikeTimer = 0.f;
        mGunMotionReturningFromStrike = true;
        mGunMotion->BasePosition(true);
      }
    } else {
      mBigStrikeTimer -= dt;
    }
  }
  if (mRapidFireShots > 5 && mShotSmokeTimer < 2.f) {
    mShotSmokeTimer += dt;
  }
  if (mGunStrikeDelayTimer > 0.f) {
    mGunStrikeDelayTimer -= dt;
  }
  if (mGunStrikeCooldownTimer > 0.f) {
    mGunStrikeCooldownTimer -= dt;
  }
}

void CPlayerGun::UpdateFreeLook(float dt, CStateManager& mgr) {
  if (mFidget.GetState() == CFidget::kS_NoFidget) {
    if (GetPlayer(mgr)->IsInFreeLook() && mMotionState.mMotionState == CMotionState::kMS_Zero &&
        !mInBigStrike) {
      if (mInFreeLook != true && mBeamChangeState == kBCS_Idle) {
        if (mEnterFreeLookDelayTimer < 0.25f) {
          mEnterFreeLookDelayTimer += dt;
        }
        if (mEnterFreeLookDelayTimer >= 0.25f && !mGrappleArm->IsLoadingDependencies()) {
          EnterFreeLook(mgr);
          mInFreeLook = true;
        }
      }
    } else {
      if (mInFreeLook) {
        if (mBeamChangeState == kBCS_Idle && !mComboFiring) {
          ReturnToDefault(mgr, mInBigStrike);
        }
        mInFreeLook = false;
      }
      mEnterFreeLookDelayTimer = 0.f;
    }
  }
}

void CPlayerGun::UpdateLeftArmTransform() {
  CTransform4f elbow = mAnimPlaying ? CTransform4f::Identity() : mElbowLocalXf;
  elbow.SetTranslation(elbow * CVector3f(-0.9f, -0.4f, 0.4f));
  mGrappleArm->SetAuxTransform(elbow);
  mGrappleArm->SetTransform(mTransform);
}

CTransform4f CPlayerGun::GetLocatorTransform(const CModelData& model, const rstl::string& name,
                                             bool dynamic) const {
  return dynamic ? model.GetScaledLocatorTransformDynamic(name, nullptr)
                 : model.GetScaledLocatorTransform(name);
}

void CPlayerGun::DrawArm(const CStateManager& mgr, const CVector3f& cameraTranslation,
                         const CModelFlags& flags) const {
  if (mGrappleArm->GetStateFlags() == 0) {
    return;
  }

  const CPlayer* player = GetPlayer(const_cast< CStateManager& >(mgr));
  const CTransform4f armTransform = mGrappleArm->GetTransform();
  if (player->GetGrappleState() != CPlayer::kGS_None ||
      CVector3f::Dot(armTransform.GetForward(), player->GetTransform().GetForward()) > 0.1f) {
    mGrappleArm->Render(mgr, cameraTranslation, flags, &mLights);
  }
}

void CPlayerGun::RenderGunWithHologram(const CStateManager& mgr, const CVector3f& cameraTranslation,
                                       bool drawSuitArm, const CTransform4f& elbowTransform,
                                       const CTransform4f& gunTransform,
                                       const CModelFlags& armFlags,
                                       const CModelFlags& gunFlags) const {
  const CWorldShadow* shadow = GetWorldShadow();
  switch (mGunMorph.mGunState) {
  case CGunMorph::kGS_OutWipeDone:
    RenderGun(mgr, cameraTranslation, drawSuitArm, elbowTransform, gunTransform, armFlags,
              gunFlags);
    break;
  case CGunMorph::kGS_InWipeDone:
    mCurrentBeam->DrawHologram(mgr, gunTransform, CModelFlags(CModelFlags::kT_Opaque, 1.f));
    if (mLights.HasShadowLight()) {
      shadow->EnableModelProjectedShadow(gunTransform, mLights.GetShadowLightArrayIndex(), 2.15f);
    }
    BeginDarkVisorRender(mgr);
    DrawArm(mgr, cameraTranslation, armFlags);
    EndDarkVisorRender(mgr);
    shadow->DisableModelProjectedShadow();
    break;
  case CGunMorph::kGS_InWipe:
  case CGunMorph::kGS_OutWipe: {
    const bool echoVisor = mgr.GetRenderVisorMode() == CStateManager::kRVM_Echo;
    const CTransform4f morphTransform =
        elbowTransform * CTransform4f::Translate(0.f, mGunMorph.mYLerp, 0.f);
    if (echoVisor) {
      gpRender->DisableDestinationAlpha();
    }
    CopyScreenTex();
    if (echoVisor) {
      gpRender->SetDestinationAlpha(0);
    }
    mCurrentBeam->DrawHologram(mgr, gunTransform, CModelFlags(CModelFlags::kT_Opaque, 1.f));
    const CGameCamera* camera = mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true);
    const float depth = ConvertToScreenSpace(morphTransform.GetTranslation(), *camera).GetZ();
    if (echoVisor) {
      gpRender->DisableDestinationAlpha();
    }
    DrawScreenTex(depth);
    if (mLights.HasShadowLight()) {
      shadow->EnableModelProjectedShadow(gunTransform, mLights.GetShadowLightArrayIndex(), 2.15f);
    }
    gpRender->SetModelMatrix(morphTransform);
    DrawClipCube(mHologramClipCube);
    const int playerIndex = mgr.MaskUIdNumPlayers(mPlayerUniqueId);
    if (echoVisor) {
      gpRender->SetDestinationAlpha(0);
    }
    mCurrentBeam->Draw(drawSuitArm, playerIndex, mgr, gunTransform, gunFlags, &mLights);
    BeginDarkVisorRender(mgr);
    DrawArm(mgr, cameraTranslation, armFlags);
    EndDarkVisorRender(mgr);
    shadow->DisableModelProjectedShadow();
    break;
  }
  }
}

void CPlayerGun::RenderGun(const CStateManager& mgr, const CVector3f& cameraTranslation,
                           bool drawSuitArm, const CTransform4f& elbowTransform,
                           const CTransform4f& gunTransform, const CModelFlags& armFlags,
                           const CModelFlags& gunFlags) const {
  const CWorldShadow* shadow = GetWorldShadow();
  if (mLights.HasShadowLight()) {
    shadow->EnableModelProjectedShadow(gunTransform, mLights.GetShadowLightArrayIndex(), 2.15f);
  }

  BeginDarkVisorRender(mgr);
  DrawArm(mgr, cameraTranslation, armFlags);
  mCurrentBeam->Draw(drawSuitArm, mgr.MaskUIdNumPlayers(mPlayerUniqueId), mgr, gunTransform,
                     gunFlags, &mLights);
  EndDarkVisorRender(mgr);

  if (!mgr.IsMultiplayer()) {
    shadow->DisableModelProjectedShadow();
  }
}

CVector3f CPlayerGun::ConvertToScreenSpace(const CVector3f& position,
                                           const CGameCamera& camera) const {
  CVector3f viewPos = camera.GetTransform().TransposeRotate(
      CVector3f(position.GetX() - camera.GetTransform().Get03(),
                position.GetY() - camera.GetTransform().Get13(),
                position.GetZ() - camera.GetTransform().Get23()));
  CVector3f screenPos(viewPos);

  if (screenPos.IsNonZero()) {
    return CGraphics::GetPerspectiveProjectionMatrix().MultiplyOneOverW(screenPos);
  }

  return CVector3f(-1.f, -1.f, 1.f);
}

void CPlayerGun::BeginDarkVisorRender(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    gpRender->SetDestinationAlpha(0);
  }
}

void CPlayerGun::EndDarkVisorRender(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    gpRender->DisableDestinationAlpha();
  }
}

void CPlayerGun::DrawScreenTex(float depth) {
  const CTransform4f backupView(CGraphics::GetViewMatrix());
  const CGraphics::CProjectionState backupProjection(CGraphics::GetProjectionState());
  const CViewport& viewport = CGraphics::GetViewport();
  CGraphics::SetOrtho(float(viewport.mLeft), float(viewport.mLeft + viewport.mWidth),
                      float(viewport.mTop + viewport.mHeight), float(viewport.mTop), -1.f, 1.f);
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetDepthWriteMode(true, kE_GEqual, true);
  CGraphics::LoadDolphinSpareTexture(320, 224, GX_TF_RGBA8, nullptr,
                                     CGraphics::kSpareBufferTexMapID);

  const GXVtxDescList vertexDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vertexDesc);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(320.f, depth, 0.f);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(640.f, depth, 0.f);
  GXTexCoord2f32(1.f, 1.f);
  GXPosition3f32(320.f, depth, 224.f);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(640.f, depth, 224.f);
  GXTexCoord2f32(1.f, 0.f);
  CGX::End();

  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetViewPointMatrix(backupView);
  CGraphics::SetProjectionState(backupProjection);
}

void CPlayerGun::CopyScreenTex() {
  GXSetTexCopySrc(320, 224, 320, 224);
  GXSetTexCopyDst(320, 224, GX_TF_RGBA8, false);
  GXCopyTex(CGraphics::GetDolphinSpareBuffer(), false);
  GXPixModeSync();
}

CPlayerGun::CPlayerGun(TUniqueId playerId, int characterIndex)
: CPlayerGunBase(rstl::string("SamusGun"), playerId, CVector3f(2.f, 2.f, 2.f), 20)
, mGunWorldXf(CTransform4f::Identity())
, mBeamLocalXf(CTransform4f::Identity())
, mElbowLocalXf(CTransform4f::Identity())
, mElbowWorldXf(CTransform4f::Identity())
, mDamageLocation(CVector3f::Zero())
, mStateMachineToken(gpSimplePool->GetObj("SamusGunFSM"))
, mGunMorph(gpTweakPlayerGun->GetGunTransformTime(), gpTweakPlayerGun->GetHoloHoldTime())
, mMotionState(gpTweakPlayerGun->GetGunExtendDistance())
, mHologramClipCube(CVector3f(-0.293292f, 0.f, -0.2481945f),
                    CVector3f(0.293292f, 1.292392f, 0.2481945f))
, mRender(&CPlayerGun::RenderGunWithHologram)
, mGunMotion(rs_new CGunMotion(NWeaponTypes::get_asset_id_from_name("GunMotion"), mScale))
, mGrappleArm(rs_new CGrappleArm(mScale, playerId, bool(uchar(characterIndex))))
, mAuxWeapon(rs_new CAuxWeapon(playerId))
, mPowerBeam(rs_new CPowerBeam(playerId, mScale, characterIndex))
, mDarkBeam(rs_new CDarkBeam(playerId, mScale, characterIndex))
, mLightBeam(rs_new CLightBeam(playerId, mScale, characterIndex))
, mAnnihilatorBeam(rs_new CAnnihilatorBeam(playerId, mScale, characterIndex))
, mSelectableBeams(4, static_cast< CGunWeapon* >(nullptr))
, mBombDependencies(TToken< CDependencyGroup >(gpSimplePool->GetObj("Bomb_DGRP")), *gpSimplePool)
, mCurrentBeam(nullptr)
, mOutgoingBeam(nullptr)
, mLoadingBeam(nullptr)
, mMissileExitTimer(7.f)
, mComboTransferFactor(0.f)
, mBombReloadTimer(0.f)
, mTimeSinceFire(0.f)
, mRapidFireDecayTimer(0.f)
, mShotSmokeTimer(0.f)
, mMuzzleEffectVisTimer(0.f)
, mEnterFreeLookDelayTimer(0.f)
, mGunStrikeCooldownTimer(0.f)
, mIdleWanderDelayTimer(0.f)
, mDamageAmount(0.f)
, mBigStrikeTimer(0.f)
, mGunStrikeDelayTimer(0.f)
, mChargePhase(kCP_NotCharging)
, mSeekerChargeState(kSCS_NotCharging)
, mSeekerChargeFactor(0.f)
, mMissileState(kMS_Inactive)
, mSeekerSecondaryFx(CGunWeapon::kSFT_None)
, mMissileShotInterval(0.f)
, mBeamChangeState(kBCS_Idle)
, mCurrentBeamId(CPlayerState::kBI_Power)
, mNextBeamId(mCurrentBeamId)
, mSoundSetIndex(0)
, mFidgetAnimBits(0)
, mAnimSfxPitch(0x2000)
, mBombCount(3)
, mRapidFireShots(0)
, mGunMotionState(SamusGun::kAS_BasePosition)
, mBeamLoadDelayFrames(0)
, mAnimSfx(ushort(-1), CSfxHandle::NullHandle())
, mChargeSfx(CSfxHandle::NullHandle())
, mInvalidSfx(CSfxHandle::NullHandle())
, mChargeRumbleHandle(-1)
, mChargeRumbleTimer(0.f)
, mPowerBombId(kInvalidUniqueId)
, x7b4_(0)
, mMaxSeekerTargets(0)
, mSeekerVisor(CPlayerState::EPlayerVisor(-1))
, mCurrentSeekerTarget(kInvalidUniqueId)
, mSeekerLockTimer(0.f)
, mAllSeekersLockedTime(0.f)
, mAmbientColor(CColor::Black())
, mAbsorbedPhazonShots(0)
, mStateMachineInitialized(false)
, mComboFiring(false)
, mRequestReturnToDefault(false)
, mInterruptEvent(false)
, mFrozen(false)
, mChargeEffectVisible(false)
, mInFreeLook(false)
, mGunMotionFidgeting(false)
, mAnimPlaying(false)
, mFiring(false)
, mPointBlankWorldSurface(false)
, mGunMotionReturningFromStrike(false)
, mMissileAnimActive(false)
, mMissileCloseAnimDone(false)
, mCommonDependenciesLoaded(false)
, mBeamLoadRequested(false) {
  mStateMachineToken.Lock();
  InitBeamData();
  InitBombData();
  TLockedToken< CDependencyGroup > dependencies(gpSimplePool->GetObj("Common_DGRP"));
  mCommonDependencies.reserve(dependencies->GetObjectTagVector().size());
  CGunWeapon::FillTokenVector(dependencies->GetObjectTagVector(), mCommonDependencies, true);
  NWeaponTypes::lock_tokens(mCommonDependencies);
  if (uchar(characterIndex) == 0) {
    mGunMotion->GetModelData().LockTextures();
  }
}

CPlayerGun::~CPlayerGun() { NWeaponTypes::unlock_tokens(mCommonDependencies); }
