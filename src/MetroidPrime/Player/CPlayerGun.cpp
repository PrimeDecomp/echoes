#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CAuxWeapon.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"
#include "dolphin/types.h"

static const float kFactorMultiplierForBeamCombo =
    1.0f / CPlayerState::GetMissileComboChargeFactor();
static const float kChargeDtFactor = 1.0f / CPlayerState::GetMissileComboChargeFactor();
static const short skEmptyBeamSfx[] = {
    0x524,
    0x25A4,
};

static bool IsSeekerTargetInRange(const CActor& target, const CPlayer& player, float radius) {
  // TODO: Compare target aim position to player eye position. The actor virtual's signature
  // differs from the current shared declaration and must be recovered before using it here.
  return false;
}

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr) {
  CPlayer* player = GetPlayerFromAll(mgr);
  CPlayerState* playerState = player->GetPlayerState();

  if (playerState->GetItemAmount(CPlayerState::kIT_BeamWeaponsDisabled, true) > 0) {
    return;
  }

  CPlayerState::EItemType beamAmmoTypeA = CPlayerState::kIT_Invalid;
  CPlayerState::EItemType beamAmmoTypeB = CPlayerState::kIT_Invalid;
  int outBeamAmmoCost = 0;

  bool outOfAmmo = IsOutOfAmmoToShoot(mgr);
  if (outOfAmmo && mChargePhase != kCP_Charged) {
    if (mChargePhase != kCP_NotCharging ||
        !playerState->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
      GetPlayer(mgr)->PlaySfxForPlayer(skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                                       mgr.GetNextAreaId(), mUnderwater, 0);
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

    const bool targetHoming = mCurrentBeam->GetVelocityInfo().GetTargetHoming(int(chargeState));

    // TODO: Select the point-blank or assisted aim transform and camera translation.
    CTransform4f xf = mGunWorldXf;

    if (mAbsorbedPhazonShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
      CToken phazonBallToken = gpSimplePool->GetObj("PhazonBall");

      TUniqueId homingTarget = targetHoming ? GetTargetId(mgr) : kInvalidUniqueId;
      mCurrentBeam->Fire(phazonBallToken, mUnderwater, dt, chargeState, xf, mgr, homingTarget, 0,
                         0x1c4, kInvalidUniqueId, CSfxHandle::NullHandle(), chargeFactor1,
                         chargeFactor1);

    } else {
      // TODO: Fire the selected beam using its normal projectile token.
    }

    mgr.fn_8003C4B8(mGunWorldXf.GetTranslation(), 0); // something with object lists

    // TODO: Muzzle/Phazon effects, recoil, and per-frame firing flags.
    bool resetCharge = false;
    mCooldown = mCurrentBeam->GetWeaponInfo().m_coolDown;
    if (mChargePhase == kCP_ChargeFx || mChargePhase == kCP_Charged) {
      resetCharge = true;
    }
    if (!resetCharge && mgr.fn_80036F10()) {
      GetPlayerFromAll(mgr)->fn_8000BC44(mgr);
    }
    if (resetCharge) {
      ResetCharge(mgr, false);
    }

    if (playerState->GetItemAmount(CPlayerState::kIT_DoubleDamage, true)) {
      GetPlayer(mgr)->PlaySfxForPlayer(0x2612, mSoundVolume, mgr.GetNextAreaId(), mUnderwater, 0);
    }
  } else {
    GetPlayer(mgr)->PlaySfxForPlayer(skEmptyBeamSfx[mSoundSetIndex], mSoundVolume,
                                     mgr.GetNextAreaId(), mUnderwater, 0);
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
    if (mChargePhase == kCP_ChargeRequested &&
        playerState->GetChargeAnimStart() < playerState->GetChargeBeamFactor()) {
      mChargePhase = kCP_Charging;
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

// Structure-first pass: TODO bodies below are not equivalent implementations.

bool CPlayerGun::ShouldHolster(CStateManager& mgr, const float& argument) {
  CPlayer* player = GetPlayer(mgr);
  const CPlayer::EPlayerMorphBallState state = player->GetMorphballTransitionState();
  return player->GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
         state == CPlayer::kMS_Morphing || state == CPlayer::kMS_Morphed ||
         !player->GetPlayerState()->IsPlayerAlive() ||
         player->GetCameraManager()->IsInCinematicCamera();
}

bool CPlayerGun::IsHolstered(CStateManager& mgr, const float& argument) {
  // The original table name is surprising: the target tests Drawing, not Holstered.
  return mGunHolsterState == kGHS_Drawing;
}

bool CPlayerGun::IsNotHolstered(CStateManager& mgr, const float& argument) {
  return mGunHolsterState == kGHS_Drawn;
}

bool CPlayerGun::StartCharge(CStateManager& mgr, const float& argument) {
  if (mInBigStrike || mGunHolsterState != kGHS_Drawn || mChargePhase != kCP_Charging ||
      !GetPlayer(mgr)->GetPlayerState()->ItemEnabled(CPlayerState::kIT_ChargeBeam)) {
    return false;
  }
  CPlayerState::EItemType ammoA, ammoB;
  int cost;
  return GetBeamAmmoTypeAndCosts(false, mgr, ammoA, ammoB, cost) || IsOutOfAmmoToShoot(mgr);
}

bool CPlayerGun::InitiateCombo(CStateManager& mgr, const float& argument) {
  // TODO: Check the selected auxiliary combo, missile/beam ammo, and Phazon override.
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
  // TODO: Check missile/seeker availability and secondary cooldown; play denial sound.
  return false;
}

bool CPlayerGun::CloseMissile(CStateManager& mgr, const float& argument) {
  return (mPressedInputFlags & 0xd) != 0 || mMissileExitTimer <= 0.f;
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
  mInterruptEvent = mInterruptEvent || ShouldHolster(mgr, argument);
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
  return !mgr.fn_80036F10() && mFidget.GetState() != CFidget::kS_NoFidget;
}

bool CPlayerGun::FidgetOver(CStateManager& mgr, const float& argument) {
  // TODO: Test input, freelook, player motion, damage, and fidget animation completion.
  return false;
}

bool CPlayerGun::Grappling(CStateManager& mgr, const float& argument) {
  // TODO: Query the grapple arm's grappling state.
  return false;
}

bool CPlayerGun::IsAlive(CStateManager& mgr, const float& argument) {
  return GetPlayer(mgr)->GetPlayerState()->IsPlayerAlive();
}

bool CPlayerGun::InPhazon(CStateManager& mgr, const float& argument) { return false; }

void CPlayerGun::Start(CStateManager& mgr, int message, float dt) {}

void CPlayerGun::Main(CStateManager& mgr, int message, float dt) {
  // TODO: Drive normal/charged firing, weapon changes, and idle animations on update.
}

void CPlayerGun::InMorphball(CStateManager& mgr, int message, float dt) {
  // TODO: Lock/unlock bomb dependencies on entry/exit; fire bombs unless Screw Attacking.
}

void CPlayerGun::Recoil(CStateManager& mgr, int message, float dt) {
  // TODO: Fire or cancel the charged shot/seeker volley and finish the recoil animation.
}

void CPlayerGun::ComboActive(CStateManager& mgr, int message, float dt) {
  // TODO: Transfer charge particles, consume combo ammunition, and handle combo fire events.
}

void CPlayerGun::Holstered(CStateManager& mgr, int message, float dt) {}

void CPlayerGun::Fidgeting(CStateManager& mgr, int message, float dt) {
  // TODO: Load, play, and unload the selected gun/grapple fidget animation.
}

void CPlayerGun::MissileActive(CStateManager& mgr, int message, float dt) {
  // TODO: Open the missile chamber, fire/reload missiles, and update seeker charging.
}

void CPlayerGun::MissileClosing(CStateManager& mgr, int message, float dt) {
  // TODO: Close the chamber and latch completion for AnimOver.
}

void CPlayerGun::EventHandler(CStateManager& mgr, int message, float dt) {
  // TODO: Clear the interrupt latch and reset any active charge on entry.
}

void CPlayerGun::DamageRumble(const CVector3f& position, float damage) {
  mDamageLocation = position;
  mDamageAmount = damage;
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
  } else if (mChargePhase > kCP_ChargeRequested && mChargePhase <= kCP_Charged) {
    cost = chargedCosts[mCurrentBeamId];
  }
  const CPlayerState* state = GetPlayer(mgr)->GetPlayerState();
  return (ammoA == CPlayerState::kIT_Invalid || state->GetItemAmount(ammoA, true) >= cost) &&
         (ammoB == CPlayerState::kIT_Invalid || state->GetItemAmount(ammoB, true) >= cost);
}

CStateMachine* CPlayerGun::GetStateMachine() {
  return mStateMachineToken.IsLoaded() ? *mStateMachineToken : nullptr;
}

void CPlayerGun::PollStateMachine(CStateManager& mgr) {
  if (!mStateMachine.HasState() && GetStateMachine() != nullptr) {
    InitializeStateMachine(mgr);
  }
}

void CPlayerGun::ResetStateMachine(CStateManager& mgr) {
  if (!mStateMachine.HasState() || rstl::string(mStateMachine.GetName()) != rstl::string("Start")) {
    mStateMachine.SetState(mgr, *this, rstl::string("Start"));
  }
}

void CPlayerGun::InitializeStateMachine(CStateManager& mgr) {
  static const TStateMachineState< CPlayerGun >::STriggerFunction triggers[] = {
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
  static const TStateMachineState< CPlayerGun >::SStateFunction states[] = {
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
  mStateMachine.Setup(*GetStateMachine());
  mStateMachine.SetTriggerFunctions(triggers, ARRAY_SIZE(triggers));
  mStateMachine.SetStateFunctions(states, ARRAY_SIZE(states));
  ResetStateMachine(mgr);
  mStateMachineInitialized = true;
}

TUniqueId CPlayerGun::CreatePowerBomb(CStateManager& mgr) { return DropPowerBomb(mgr); }

void CPlayerGun::PlayAnim(CStateManager& mgr, int animation, bool loop) {
  mCurrentBeam->PlayAnim(NWeaponTypes::EGunAnimType(animation), loop);
  // TODO: Play the beam/multiplayer-specific open, close, and weapon-switch sound.
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
, mHoloHoldTime(holdTime < 0.f ? -holdTime : holdTime)
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
  if (mMorphDirection == direction || mGunState == kGS_OutWipe) {
    if (mGunState != kGS_InWipe) {
      mRemTime = mGunTransformTime - mRemTime;
    }
  } else {
    mRemTime = mGunTransformTime;
    mSpeed = 1.f / mGunTransformTime;
  }
  mMorphDirection = direction;
  mGunState = direction == kMD_In ? kGS_InWipe : kGS_OutWipe;
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
  // TODO: Recoil extension/parabola and lock-on rotation interpolation.
}

void CPlayerGun::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Forward messages to gun/auxiliary/grapple resources and initialize registered assets.
}

void CPlayerGun::TouchModel(const CStateManager& mgr) const {
  // TODO: Touch current gun-motion, grapple, beam, and hologram models.
}

void CPlayerGun::PreRender(CStateManager& mgr, const CVector3f& cameraPosition) {
  // TODO: Prepare arm/beam models, lights, rain splashes, and world shadow.
}

void CPlayerGun::AddToRenderer(const CStateManager& mgr) const {
  // TODO: Queue visible muzzle and Phazon particle generators.
}

void CPlayerGun::Render(const CStateManager& mgr, const CVector3f& cameraTranslation,
                        const CModelFlags& flags) const {
  // TODO: Dispatch the selected gun renderer and draw grapple/muzzle/Phazon effects.
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
  // TODO: Coordinate loading, state machine, weapon effects, pose, aiming, lights, and timers.
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
  return mAuxWeapon.null() ? kInvalidUniqueId : mAuxWeapon->GetTargetId();
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr) {
  // TODO: Request suit resources on the grapple arm and current beam.
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Update base input state, beam changes, charge requests, and weapon state machine.
}

CVector3f CPlayerGun::fn_801c6df8() const {
  // TODO: Return the current beam's unidentified vector, or zero when absent.
  return CVector3f::Zero();
}

int CPlayerGun::GetBombsAvailable(CStateManager& mgr) const {
  // TODO: Subtract the weapon manager's active bomb count from the three-bomb limit.
  return 0;
}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr) const {
  // TODO: Create/register CPowerBomb with player damage, ownership, and bomb effect tokens.
  return kInvalidUniqueId;
}

void CPlayerGun::DropBomb(EBWeapon type, CStateManager& mgr) {
  // TODO: Create/register CBomb, apply double damage, update reload/count, and attach to platforms.
}

void CPlayerGun::FireBombs(CStateManager& mgr) {
  // TODO: Check morph state, bomb/power-bomb input, ammo, cooldown, and active power bomb.
}

void CPlayerGun::TakeDamage(bool bigStrike, bool strikeGrapple, CStateManager& mgr) {
  // TODO: Start struck gun/grapple animations, select SFX, and cancel active charge/fidget.
}

TUniqueId CPlayerGun::GetTargetId(CStateManager& mgr) {
  // TODO: Validate the player's orbit target against projectile-target material flags.
  return kInvalidUniqueId;
}

void CPlayerGun::PlayBeamFireSfx(CStateManager& mgr, CPlayer& player, bool play) {
  // TODO: Play the beam fire sound when requested and outside cinematics.
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
    mCurrentBeam->Unk8();
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
    // TODO: Start the SP/MP beam or seeker charge sound and charge rumble.
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
  // TODO: Toggle the player's remote charge effect in multiplayer.
}

void CPlayerGun::UpdateSeeker(float dt, CStateManager& mgr) {
  // TODO: Acquire/revalidate targets, advance charge/lock timers, and launch the seeker volley.
}

void CPlayerGun::UpdateSeekerEffects(float dt) {
  // TODO: Update seeker/missile particles, opacity, scale, and muzzle positions.
}

void CPlayerGun::EnableSeekerFx(CStateManager& mgr, bool enable) {
  // TODO: Create/clear the seeker muzzle and missile secondary generators.
}

void CPlayerGun::FireSecondary(float dt, CStateManager& mgr, TUniqueId target, uint attributes,
                               const CTransform4f* transform, ushort sound) {
  // TODO: Consume missiles/combo ammunition and dispatch the auxiliary projectile.
}

void CPlayerGun::UpdateAuxWeapons(const CTransform4f& transform, CStateManager& mgr) {
  // TODO: Update combo completion and cancel the beam's secondary effect when finished.
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool deactivate) {
  // TODO: Stop auxiliary combo effects and cancel the beam's charge particles.
}

void CPlayerGun::DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode& node,
                                 EUserEventType type) {
  // TODO: Handle projectile/combo animation events using their locator transforms.
}

void CPlayerGun::DoUserAnimEvents(float dt, CStateManager& mgr) {
  // TODO: Dispatch user and sound POIs with player pitch and underwater settings.
}

void CPlayerGun::SetGunLightActive(bool active, CStateManager& mgr) {
  // TODO: Activate the script light associated with this gun.
}

void CPlayerGun::UpdateGunLight(const CTransform4f& transform, CStateManager& mgr) {
  // TODO: Copy the active muzzle generator's light/color and transform to the gun light.
}

void CPlayerGun::SetBeam(CPlayerState::EBeamId beam, CStateManager& mgr) {
  // TODO: Unload/select/load the four beam weapons and update auxiliary beam data.
}

void CPlayerGun::InitMuzzleData(CStateManager& mgr) {
  // TODO: Choose SP/MP renderer/sounds, initialize beam/seeker/missile particle resources.
}

void CPlayerGun::InitBombData() {
  // TODO: Populate normal/power bomb effect pairs from BombSet/BombExplo/PowerBombExplo.
}

void CPlayerGun::InitBeamData() {
  mSelectableBeams[0] = mPowerBeam.get();
  mSelectableBeams[1] = mDarkBeam.get();
  mSelectableBeams[2] = mLightBeam.get();
  mSelectableBeams[3] = mAnnihilatorBeam.get();
  mCurrentBeam = mSelectableBeams[0];
}

void CPlayerGun::ChangeWeapon(CStateManager& mgr) {
  // TODO: Complete the pending beam swap, initialize effects, and update player beam selection.
}

bool CPlayerGun::ProcessGunMorph(float dt, CStateManager& mgr) {
  // TODO: Advance the hologram wipe, update the loading beam and complete weapon changes.
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
  // TODO: Read beam selections, validate ownership, and request the selected beam.
}

void CPlayerGun::EnterFreeLook(CStateManager& mgr) {
  // TODO: Select the matching beam/gun/grapple freelook animation set.
}

void CPlayerGun::ReturnArmAndGunToDefault(CStateManager& mgr, bool force) {
  // TODO: Restore arm/gun poses when fidgeting, in freelook, or forced.
}

void CPlayerGun::ReturnToDefault(CStateManager& mgr, bool bigStrikeReset) {
  // TODO: Reset gun-motion and grapple-controller animations.
}

void CPlayerGun::SetFidgetAnimBits(int animSet, bool holster) {
  // TODO: Select participating gun-motion, beam, and grapple fidget animations.
}

bool CPlayerGun::IsFidgetLoaded() {
  // TODO: Poll every animation selected by mFidgetAnimBits.
  return false;
}

void CPlayerGun::UnLoadFidget() {
  // TODO: Unload selected gun-motion, beam, and grapple fidget animations.
}

void CPlayerGun::AsyncLoadFidget(CStateManager& mgr) {
  // TODO: Request selected fidget resources for the player and animation set.
}

void CPlayerGun::EnterFidget(CStateManager& mgr) {
  // TODO: Start the selected gun-motion, beam, and grapple fidget animations.
}

void CPlayerGun::UpdateGunIdle(float dt, CStateManager& mgr) {
  // TODO: Advance fidget delays and choose idle/wander/holster actions.
}

void CPlayerGun::UpdateGunMotion(float dt, CStateManager& mgr) {
  // TODO: Update the gun-motion animation controller and user events.
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
  // TODO: Enter/exit freelook according to input, movement, and strike cooldown.
}

void CPlayerGun::UpdateLeftArmTransform() {
  // TODO: Compose the elbow locator and gun transform for the grapple arm.
}

CTransform4f CPlayerGun::GetLocatorTransform(const CModelData& model, const rstl::string& name,
                                             bool dynamic) const {
  // TODO: Resolve the static/dynamic model locator and apply model scale.
  return CTransform4f::Identity();
}

void CPlayerGun::DrawArm(const CStateManager& mgr, const CVector3f& cameraTranslation,
                         const CModelFlags& flags) const {
  // TODO: Render the grapple arm with the gun's actor lights and camera translation.
}

void CPlayerGun::RenderGunWithHologram(const CStateManager& mgr, const CVector3f& cameraTranslation,
                                       bool drawSuitArm, const CTransform4f& elbowTransform,
                                       const CTransform4f& gunTransform,
                                       const CModelFlags& armFlags,
                                       const CModelFlags& gunFlags) const {
  // TODO: Draw the gun, transition clip planes/hologram, and suit arm.
}

void CPlayerGun::RenderGun(const CStateManager& mgr, const CVector3f& cameraTranslation,
                           bool drawSuitArm, const CTransform4f& elbowTransform,
                           const CTransform4f& gunTransform, const CModelFlags& armFlags,
                           const CModelFlags& gunFlags) const {
  // TODO: Draw the non-hologram beam and suit arm using their supplied flags/transforms.
}

CVector3f CPlayerGun::ConvertToScreenSpace(const CVector3f& position,
                                           const CGameCamera& camera) const {
  // TODO: Transform/project the world point and convert to viewport coordinates.
  return CVector3f::Zero();
}

void CPlayerGun::BeginDarkVisorRender(const CStateManager& mgr) const {
  // TODO: Begin the renderer's dark-visor ordering hook.
}

void CPlayerGun::EndDarkVisorRender(const CStateManager& mgr) const {
  // TODO: End the renderer's dark-visor ordering hook.
}

void CPlayerGun::DrawScreenTex() {
  // TODO: Draw the captured screen texture with the gun's transition material.
}

void CPlayerGun::CopyScreenTex() {
  // TODO: Copy the gun transition screen rectangle into the scratch texture.
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
, mGrappleArm(rs_new CGrappleArm(mScale, playerId, bool(uchar(characterIndex))))
, mAuxWeapon(rs_new CAuxWeapon(playerId))
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
  // TODO: Construct CGunMotion and the four beam subclasses once their layouts are recovered.
  // The current CGunMotion/CPowerBeam scaffolds cannot safely supply the target allocation sizes.
  mStateMachineToken.Lock();
  InitBeamData();
  InitBombData();
  // TODO: Fill/lock Common_DGRP tokens and prepare the single-player gun-motion materials.
}

CPlayerGun::~CPlayerGun() {
  for (rstl::vector< CToken >::iterator it = mCommonDependencies.begin();
       it != mCommonDependencies.end(); ++it) {
    it->Unlock();
  }
}
