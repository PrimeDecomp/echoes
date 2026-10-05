#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CControlHintManager.hpp"
#include "MetroidPrime/CRezbitEffect.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"

#include <float.h>

void CPlayer::UpdatePlayerControlDirection(float dt, CStateManager& mgr) {
  const CVector3f oldDirection = mControlDir;
  const CVector3f oldFlatDirection = mControlDirFlat;
  CalculatePlayerControlDirection(mgr);
  if (mInterpolatingControlDir && mMorphBallState == kMS_Morphed) {
    mControlDirInterpTime += dt;
    if (mControlDirInterpTime > mControlDirInterpDuration) {
      mControlDirInterpTime = mControlDirInterpDuration;
      ResetControlDirectionInterpolation();
    }
    const float blend = CMath::Limit(mControlDirInterpTime / mControlDirInterpDuration, 1.f);
    mControlDir = CVector3f::Lerp(oldDirection, mControlDir, blend);
    mControlDirFlat = CVector3f::Lerp(oldFlatDirection, mControlDir, blend);
  }
}

void CPlayer::CalculatePlayerControlDirection(CStateManager& mgr) {
  if (mControlDirectionOverridden) {
    if (mControlDirOverride.CanBeNormalized()) {
      mControlDir = mControlDirOverride.AsNormalized();
      mControlDirFlat = mControlDirOverride;
      mControlDirFlat.SetZ(0.f);
      if (mControlDirFlat.CanBeNormalized()) {
        mControlDirFlat.Normalize();
      } else {
        mControlDir = CVector3f(0.f, 1.f, 0.f);
        mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
      }
    } else {
      mControlDir = CVector3f(0.f, 1.f, 0.f);
      mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
    }
  } else {
    const CVector3f cameraToPlayer =
        GetTranslation() - mCameraManager->GetCurrentCamera(mgr, true)->GetTranslation();
    if (!cameraToPlayer.CanBeNormalized()) {
      mControlDir = CVector3f(0.f, 1.f, 0.f);
      mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
    } else {
      CVector3f flatDirection = cameraToPlayer;
      flatDirection.SetZ(0.f);
      if (flatDirection.CanBeNormalized()) {
        if (flatDirection.Magnitude() > gpTweakBall->GetBallCameraControlDistance()) {
          mControlDir = cameraToPlayer.AsNormalized();
          if (flatDirection.CanBeNormalized()) {
            flatDirection.Normalize();
            switch (mMorphBallState) {
            case kMS_Morphed:
              mControlDirFlat = flatDirection;
              break;
            case kMS_Unmorphed:
            case kMS_Morphing:
            case kMS_Unmorphing:
              mControlDir = GetTransform().GetForward();
              mControlDirFlat = mControlDir;
              mControlDirFlat.SetZ(0.f);
              if (mControlDirFlat.CanBeNormalized()) {
                mControlDirFlat.Normalize();
              }
              break;
            }
          } else if (mMorphBallState != kMS_Morphed) {
            mControlDir = GetTransform().GetForward();
            mControlDirFlat = mControlDir;
            mControlDirFlat.SetZ(0.f);
            if (mControlDirFlat.CanBeNormalized()) {
              mControlDirFlat.Normalize();
            }
          }
        } else {
          if (mFlatMoveSpeed < 0.25f) {
            mControlDir = cameraToPlayer;
            mControlDirFlat = flatDirection;
          } else if (mMorphBallState != kMS_Morphed) {
            mControlDir = GetTransform().GetForward();
            mControlDirFlat = mControlDir;
            mControlDirFlat.SetZ(0.f);
            if (mControlDirFlat.CanBeNormalized()) {
              mControlDirFlat.Normalize();
            }
          }
        }
      }
    }
  }
}

void CPlayer::ResetPlayerHintState(CStateManager& mgr) {
  // The action controlled by this Prime-shared flag remains unresolved.
  x1268_26_ = true;
  mCanEnterMorphBall = true;
  mCanLeaveMorphBall = true;
  mControlDirectionOverridden = false;
  mExtendTargetDistance = false;
  mOutOfBallLookAtHint = false;
  mSpiderBallControlXY = false;
  x126a_ &= ~1;
  x1269_31_ = false;
  x126a_ &= ~0x80;
  x126a_ &= ~0x40;
  mLandingStrikePending = false;
  mMorphBall->SetBoostEnabled(true);
  ResetControlDirectionInterpolation();
  RemoveMaterial(kMT_Immovable, mgr);
  if (mControlHintManager && mPlayerHintControlHintId != kInvalidUniqueId) {
    mControlHintManager->RemoveHint(mPlayerHintControlHintId, GetUniqueId(), mgr);
  }
}

bool CPlayer::SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr) {
  x1268_26_ = (hint.GetOverrideFlags() & 1) != 0;
  mCanEnterMorphBall = !(hint.GetOverrideFlags() & 0x40);
  mCanLeaveMorphBall = !(hint.GetOverrideFlags() & 0x20);
  mDampBoostEntryVelocity = (hint.GetOverrideFlags() & 0x800000) != 0;
  mControlDirectionOverridden = (hint.GetOverrideFlags() & 2) != 0;
  if (mControlDirectionOverridden) {
    mControlDirOverride = hint.GetTransform().GetForward();
    SetControlDirectionInterpolation(hint.GetControlInterpDur());
  }
  mExtendTargetDistance = (hint.GetOverrideFlags() & 4) != 0;
  mOutOfBallLookAtHint = (hint.GetOverrideFlags() & 8) != 0;
  mSpiderBallControlXY = (hint.GetOverrideFlags() & 0x10) != 0;
  x126a_ = (x126a_ & ~1) | ((hint.GetOverrideFlags() & 0x4000) ? 1 : 0);
  x1269_31_ = (hint.GetOverrideFlags() & 0x8000) != 0;
  x126a_ = (x126a_ & ~0x80) | ((hint.GetOverrideFlags() & 0x10000) ? 0x80 : 0);
  x126a_ = (x126a_ & ~0x40) | ((hint.GetOverrideFlags() & 0x20000) ? 0x40 : 0);
  mMorphBall->SetBoostEnabled(!(hint.GetOverrideFlags() & 0x100));

  bool switchedVisor = false;
  if ((hint.GetOverrideFlags() & 0x200) != 0) {
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_CombatVisor)) {
      mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x400) != 0) {
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_ScanVisor)) {
      mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Scan);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x800) != 0) {
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_DarkVisor)) {
      mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Dark);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x1000) != 0) {
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_EchoVisor)) {
      mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Echo);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x2000) != 0) {
    AddMaterial(kMT_Immovable, mgr);
  }
  if ((hint.GetOverrideFlags() & 0x80) != 0) {
    if (mPlayerHintControlHintId != kInvalidUniqueId && mControlHintManager) {
      mControlHintManager->RemoveHint(mPlayerHintControlHintId, GetUniqueId(), mgr);
    }
    mPlayerHintControlHintId =
        DisableControls(mgr, 1, hint.GetUniqueId(), 0.f, CGameHint::kBHT_Unknown5);
  }
  if ((hint.GetOverrideFlags() & 0x200000) != 0 && mMorphBallState != kMS_Unmorphed) {
    PrepareToLeaveMorphBallState(0.f, mgr, kMS_Unmorphed);
    LeaveMorphBallState(mgr);
    return switchedVisor;
  }
  if ((hint.GetOverrideFlags() & 0x400000) != 0 && mMorphBallState != kMS_Morphed) {
    SetOrbitRequest(kOR_EnterMorphBall, mgr);
    mGun->Holster(mgr);
    mGravityBoostUsed = false;
    EnterMorphBallState(mgr, kMS_Unmorphed);
    PrepareToEnterMorphBallState(0.f, mgr);
    ActivateMorphBallCamera(mgr);
    mCameraManager->HintManager()->RefreshHint(mgr);
    mCameraManager->BallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
  }
  return switchedVisor;
}

bool CPlayer::FireBeamHeld(const CFinalInput& input) const {
  bool held = false;
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_FireOrBomb, input) ||
      mControlMapper.GetDigitalInput(CControlMapper::kC_FireOrBomb2, input)) {
    held = true;
  }
  return held;
}

bool CPlayer::FireBeamPressed(const CFinalInput& input) const {
  bool pressed = false;
  if (mControlMapper.GetPressInput(CControlMapper::kC_FireOrBomb, input) ||
      mControlMapper.GetPressInput(CControlMapper::kC_FireOrBomb2, input)) {
    pressed = true;
  }
  return pressed;
}

bool CPlayer::AutoFireHeld(const CFinalInput& input) const {
  bool held = false;
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_AutoFireBeam, input)) {
    held = true;
  }
  return held;
}

bool CPlayer::JumpHeld(const CFinalInput& input) const {
  bool held = false;
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost, input) ||
      mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost2, input)) {
    held = true;
  }
  return held;
}

bool CPlayer::JumpPressed(const CFinalInput& input) const {
  bool pressed = false;
  if (mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input) ||
      mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost2, input)) {
    pressed = true;
  }
  return pressed;
}

bool CPlayer::ChargeBeamHeld(const CFinalInput& input) const {
  bool held = false;
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_ChargeBeam, input) ||
      mControlMapper.GetDigitalInput(CControlMapper::kC_ChargeBeam2, input)) {
    held = true;
  }
  return held;
}

bool CPlayer::BoostHeld(const CFinalInput& input) const {
  bool held = false;
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_BoostBall, input)) {
    held = true;
  }
  return held;
}

void CPlayer::UpdateRezbitRecoveryInput(const CFinalInput& input) {
  const bool left = mControlMapper.GetPressInput(CControlMapper::kC_TurnLeft, input);
  const bool right = mControlMapper.GetPressInput(CControlMapper::kC_TurnRight, input);
  if (right && (mRezbitRecoveryDirection == 1 || mRezbitRecoveryDirection == 0)) {
    ++mRezbitRecoveryInputCount;
    mRezbitRecoveryDirection = 2;
  }
  if (left && (mRezbitRecoveryDirection == 2 || mRezbitRecoveryDirection == 0)) {
    ++mRezbitRecoveryInputCount;
    mRezbitRecoveryDirection = 1;
  }
}

void CPlayer::ResetRezbitRecoveryInput() {
  mRezbitRecoveryDirection = 0;
  mRezbitRecoveryInputCount = 0;
}

CPlayer::ERezbitState CPlayer::GetRezbitState() const { return mRezbitState; }

void CPlayer::SetRezbitState(ERezbitState state) {
  if (state == kRS_None || state == kRS_Recovered) {
    mRezbitState = state;
  }
}

void CPlayer::StartRezbitState(CStateManager& mgr, const CRezbitEffectOptions& options) {
  if (GetRezbitState() == kRS_None) {
    mRezbitGunDrawBlocks.AddPlayer(mgr, mPlayerIndex, true);
    SetHudDisable(2.f, 0.5f, 0.5f);
    gpGameState->HintOptions().SetInRezbitState(true);
    if (mPlayerState->HasPowerUp(CPlayerState::kIT_CombatVisor)) {
      mPlayerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
    }

    CScriptControlHint::TCommandStates commandStates;
    mRezbitControlHintId =
        static_cast< CControlHintManager* >(mControlHintManager)
            ->CreateHint(mgr, rstl::string("Rezbit Control Hint"), 0, 0.f, 0xfc, commandStates,
                         GetUniqueId(), CGameHint::kBHT_TriggersAndB, 0, 0.f,
                         CGameHint::SCallback(this, &CPlayer::ResetRezbitState),
                         CGameHint::SCallback(this, &CPlayer::BeginRezbitRecovery), 2.f, 1);
    mRezbitState = kRS_Infected;
    CRezbitEffect* effect = rs_new CRezbitEffect(
        mgr.AllocateUniqueId(),
        CEntityInfo(GetAreaIdForPersistence(), CEntity::NullConnectionList, true, kUnkId), options);
    mgr.AddObject(*effect);
    mRezbitVirusMemoTimer = 1.25f;
    mGun->ResetCharge(mgr, true);
  }
}

void CPlayer::UpdateRezbitState(float dt) {
  if (mRezbitVirusMemoTimer > 0.f && mRezbitState == kRS_Infected) {
    mRezbitVirusMemoTimer -= dt;
    if (mRezbitVirusMemoTimer <= 0.f) {
      const int playerIndex = GetPlayerIndex();
      CSamusHud::DisplayHudMemo(
          rstl::wstring_l(gpStringTable->GetString("RezbitSuitSoftwareVirus")),
          CHUDMemoParms(FLT_MAX, true, false, false, 1 << playerIndex, false));
    }
  }
  if (mStaticTimer < 0.5f) {
    SetHudDisable(0.5f, 0.5f, 0.5f);
  }
}

void CPlayer::BeginRezbitRecovery() {
  mRezbitState = kRS_Recovering;
  const int playerIndex = GetPlayerIndex();
  CSamusHud::DisplayHudMemo(rstl::wstring_l(L""),
                            CHUDMemoParms(0.f, false, true, false, 1 << playerIndex, true));
}

void CPlayer::ResetRezbitState(CStateManager& mgr) {
  mRezbitGunDrawBlocks.RemovePlayer(mgr, mPlayerIndex);
  mRezbitState = kRS_None;
  mRezbitControlHintId = kInvalidUniqueId;
  gpGameState->HintOptions().SetInRezbitState(false);
}

void CPlayer::StopRezbitState(CStateManager& mgr) {
  if (mRezbitState != kRS_None && mRezbitControlHintId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mRezbitControlHintId);
    mRezbitGunDrawBlocks.RemovePlayer(mgr, mPlayerIndex);
    mRezbitState = kRS_None;
    mRezbitControlHintId = kInvalidUniqueId;
    gpGameState->HintOptions().SetInRezbitState(false);
  }
}

TUniqueId CPlayer::DisableControls(CStateManager& mgr, uint controls, TUniqueId source,
                                   float duration, CGameHint::EBreakHintType breakType) {
  if (mControlHintManager) {
    CScriptControlHint::TCommandStates commandStates;
    return static_cast< CControlHintManager* >(mControlHintManager)
        ->CreateHint(mgr, rstl::string("Player Hint disabled controls"), 0, duration, controls,
                     commandStates, source, breakType, 0, 0.f, CGameHint::SCallback(),
                     CGameHint::SCallback(), 0.f, 0);
  }
  return kInvalidUniqueId;
}
