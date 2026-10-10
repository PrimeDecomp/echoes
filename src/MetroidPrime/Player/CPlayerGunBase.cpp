#include "MetroidPrime/Player/CPlayerGunBase.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CControlHintManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"

static uint ReleasedFlags(uint last, uint current);
static uint PressedFlags(uint last, uint current);

CPlayerGunBase::CPlayerGunBase(const rstl::string& name, TUniqueId playerId, const CVector3f& scale,
                               int maxSplashes)
: CEntity(kInvalidUniqueId, NullEntityInfo, name, 0)
, mTransform(CTransform4f::Identity())
, mAssistAimXf(CTransform4f::Identity())
, mScale(scale)
, mRainSplashGenerator(rs_new CRainSplashGenerator(scale, maxSplashes, 2, 0.f, 0.125f))
, mLights(8, CVector3f::Zero(), 4, 4, 0.1f, false, false, false, false)
, mPlayerUniqueId(playerId)
, mLightId(kInvalidUniqueId)
, mWorldShadow(rs_new CWorldShadow(32, 32, true))
, mCooldown(0.f)
, mSecondaryCooldown(0.f)
, mGunHolsterRemTime(0.f)
, mInputFlags(0)
, mLastInputFlags(0)
, mReleasedInputFlags(0)
, mPressedInputFlags(0)
, mFiredWeaponFlags(0)
, mGunDrawBlockCount(0)
, mChargeState(CPlayerState::kCS_Normal)
, mGunHolsterState(kGHS_Drawn)
, mSoundVolume(0x4a)
, mUnderwater(false)
, mBombsDisabled(false)
, mInBigStrike(false)
, mMissileMode(false)
, mInPhazonPool(false) {}

CPlayerGunBase::~CPlayerGunBase() {}

void CPlayerGunBase::Reset(CStateManager& mgr) {
  const bool wasInBigStrike = mInBigStrike;
  mInBigStrike = true;
  ProcessInput(CFinalInput(), mgr);
  mInBigStrike = wasInBigStrike;
  mGunDrawBlockCount = 0;
}

void CPlayerGunBase::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  const CPlayer* player = GetPlayer(mgr);
  const bool morphed = player->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
  const bool bigStrike = mInBigStrike && !morphed;
  const bool frozen = player->GetFrozenState() && !morphed;
  if (bigStrike || frozen ||
      static_cast< const CControlHintManager* >(player->GetControlHintManager())
          ->HasDisableFlags(1, mgr)) {
    mPressedInputFlags = 0;
    mReleasedInputFlags = 0;
    mLastInputFlags = 0;
    mInputFlags = 0;
    return;
  }
  mInputFlags = player->FireBeamHeld(input) ? 1 : 0;
  const bool charge = player->ChargeBeamHeld(input);
  int chargeFlag = 0;
  if (charge) {
    chargeFlag = 4;
  }
  mInputFlags |= chargeFlag;
  const bool missile =
      player->GetControlMapper().GetDigitalInput(CControlMapper::kC_MissileOrPowerBomb, input);
  int missileFlag = 0;
  if (missile) {
    missileFlag = 2;
  }
  mInputFlags |= missileFlag;
  const bool autoFire = player->AutoFireHeld(input);
  int autoFireFlag = 0;
  if (autoFire) {
    autoFireFlag = 8;
  }
  mInputFlags |= autoFireFlag;
  mReleasedInputFlags = ReleasedFlags(mLastInputFlags, mInputFlags);
  mPressedInputFlags = PressedFlags(mLastInputFlags, mInputFlags);
  mLastInputFlags = mInputFlags;
}

void CPlayerGunBase::Update(float dt, CStateManager& mgr) {
  mUnderwater = GetPlayer(mgr)->GetCameraManager()->GetFirstPersonCamera()->GetFluidCount() != 0;
  mFiredWeaponFlags = 0;
  if (mCooldown > 0.f) {
    mCooldown -= dt;
  }
  if (mSecondaryCooldown > 0.f) {
    mSecondaryCooldown -= dt;
  }
  if (mRainSplashGenerator.get() != nullptr) {
    mRainSplashGenerator->Update(dt, mgr);
  }
}

void CPlayerGunBase::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  EScriptObjectMessage message = msg.GetMessage();
  CPlayer* player = GetPlayer(mgr);
  switch (message) {
  case kSM_Create:
    mSoundVolume = player->GetSoundPan(CPlayer::kMSP_3);
    CreateGunLight(mgr);
    break;
  case kSM_Delete:
    DeleteGunLight(mgr);
    break;
  case kSM_EnteredPhazonPool:
  case kSM_InsidePhazonPool:
    mInPhazonPool = true;
    break;
  case kSM_ExitedPhazonPool:
    mInPhazonPool = false;
    break;
  case kSM_EnteredFluid:
  case kSM_InsideFluid:
  case kSM_ExitedFluid:
    break;
  default:
    break;
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

void CPlayerGunBase::CreateGunLight(CStateManager& mgr) {
  if (mLightId == kInvalidUniqueId) {
    mLightId = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CGameLight(mLightId, kInvalidAreaId, false, rstl::string_l(""), mTransform,
                                    mPlayerUniqueId,
                                    CLight::BuildDirectional(CVector3f::Forward(), CColor::Black()),
                                    mLightId.Value() & 0x3ff, 0, 0.f));
  }
}

void CPlayerGunBase::DeleteGunLight(CStateManager& mgr) {
  if (mLightId != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mLightId);
    mLightId = kInvalidUniqueId;
  }
}

CWorldShadow* CPlayerGunBase::GetWorldShadow() { return mWorldShadow.get(); }

const CWorldShadow* CPlayerGunBase::GetWorldShadow() const { return mWorldShadow.get(); }

static uint ReleasedFlags(uint last, uint current) { return last & (last ^ current); }

static uint PressedFlags(uint last, uint current) { return current & (last ^ current); }

void CPlayerGunBase::Holster(CStateManager& mgr) {
  mGunHolsterState = kGHS_Holstered;
  mGunHolsterRemTime = 0.f;
  GetPlayerFromAll(mgr)->SetAimTarget(kInvalidUniqueId);
}

void CPlayerGunBase::HolsterGun(CStateManager& mgr) {
  if (mGunHolsterState == kGHS_Holstered || mGunHolsterState == kGHS_Holstering) {
    return;
  }
  CPlayer* player = GetPlayerFromAll(mgr);
  float holsterTime = gpTweakPlayerGun->GetGunHolsterTime();
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphing) {
    holsterTime = 0.1f;
  }
  if (mGunHolsterState == kGHS_Drawing) {
    mGunHolsterRemTime = holsterTime * (1.f - mGunHolsterRemTime / 0.45f);
  } else {
    mGunHolsterRemTime = holsterTime;
  }
  mGunHolsterState = kGHS_Holstering;
  player->SetAimTarget(kInvalidUniqueId);
}

void CPlayerGunBase::DrawGun(CStateManager& mgr) {
  if (mGunHolsterState != kGHS_Holstered || GetPlayer(mgr)->InGrappleJumpCooldown()) {
    return;
  }

  mGunHolsterState = kGHS_Drawing;
  mGunHolsterRemTime = 0.45f;
}

void CPlayerGunBase::UpdateGunHolster(const CFinalInput& input, CStateManager& mgr) {
  float dt = input.DeltaTime();
  CPlayer* player = GetPlayer(mgr);
  switch (mGunHolsterState) {
  case kGHS_Drawn: {
    bool shouldHolster = false;
    if (player->GetTweakPlayerControls()->GetGunButtonTogglesHolster()) {
      if (player->GetControlMapper().GetPressInput(CControlMapper::kC_ToggleHolster, input)) {
        shouldHolster = true;
      }
      if (!player->FireBeamHeld(input) &&
          !player->GetControlMapper().GetDigitalInput(CControlMapper::kC_MissileOrPowerBomb,
                                                      input) &&
          player->GetTweakPlayerControls()->GetGunNotFiringHolstersGun()) {
        mGunHolsterRemTime -= dt;
        if (mGunHolsterRemTime <= 0.f) {
          shouldHolster = true;
        }
      }
    } else {
      if (!player->FireBeamHeld(input) && !player->GetControlMapper().GetDigitalInput(
                                              CControlMapper::kC_MissileOrPowerBomb, input)) {
        if (player->GetTweakPlayerControls()->GetGunNotFiringHolstersGun()) {
          mGunHolsterRemTime -= dt;
        }
      } else {
        mGunHolsterRemTime = gpTweakPlayerGun->GetGunNotFiringTime();
      }
    }
    if (shouldHolster) {
      HolsterGun(mgr);
    }
    break;
  }
  case kGHS_Drawing:
    if (mGunHolsterRemTime > 0.f) {
      mGunHolsterRemTime -= dt;
    } else {
      mGunHolsterState = kGHS_Drawn;
      mGunHolsterRemTime = gpTweakPlayerGun->GetGunNotFiringTime();
    }
    break;
  case kGHS_Holstered: {
    if (GetPlayer(mgr)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      mPressedInputFlags = 0;
      mReleasedInputFlags = 0;
      mLastInputFlags = 0;
      mInputFlags = 0;
    }
    bool draw = false;
    if (player->FireBeamHeld(input) ||
        player->GetControlMapper().GetDigitalInput(CControlMapper::kC_MissileOrPowerBomb, input) ||
        player->GetGrappleState() == CPlayer::kGS_None) {
      draw = true;
    } else if (player->GetTweakPlayerControls()->GetGunButtonTogglesHolster()) {
      if (player->GetControlMapper().GetPressInput(CControlMapper::kC_ToggleHolster, input)) {
        draw = true;
      }
    }
    CPlayerState* playerState = player->GetPlayerState();
    if (playerState->GetCurrentVisor() == CPlayerState::kPV_Scan ||
        playerState->GetTransitioningVisor() == CPlayerState::kPV_Scan ||
        player->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed ||
        mGunDrawBlockCount != 0) {
      draw = false;
    }
    if (draw) {
      DrawGun(mgr);
    }
    break;
  }
  case kGHS_Holstering:
    if (GetPlayer(mgr)->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      mPressedInputFlags = 0;
      mReleasedInputFlags = 0;
      mLastInputFlags = 0;
      mInputFlags = 0;
    }
    if (mGunHolsterRemTime > 0.f) {
      mGunHolsterRemTime -= dt;
    } else {
      mGunHolsterState = kGHS_Holstered;
    }
    break;
  default:
    break;
  }
}

void CPlayerGunBase::UpdateTransform(CStateManager& mgr, const CVector3f& position,
                                     const CTransform4f& rotation, CTransform4f& result) {
  CUnitVector3f axis(rotation.GetColumn(kDX));
  switch (mGunHolsterState) {
  case kGHS_Drawing: {
    float t = CMath::Limit(mGunHolsterRemTime / 0.45f, 1.f);
    if (t > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          axis, CRelAngle::FromRadians(-t * gpTweakPlayerGun->GetFixedVerticalAim()));
      result = quat.BuildTransform4f() * rotation.GetRotation();
      result.SetTranslation(position);
    }
    break;
  }
  case kGHS_Holstered: {
    CQuaternion quat = CQuaternion::AxisAngle(
        axis, CRelAngle::FromRadians(-gpTweakPlayerGun->GetFixedVerticalAim()));
    result = quat.BuildTransform4f() * rotation.GetRotation();
    result.SetTranslation(position);
    break;
  }
  case kGHS_Holstering: {
    float t = 1.f - CMath::Limit(mGunHolsterRemTime / gpTweakPlayerGun->GetGunHolsterTime(), 1.f);
    if (GetPlayer(mgr)->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      t = 1.f - CMath::Limit(mGunHolsterRemTime / 0.1f, 1.f);
    }
    if (t > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          axis, CRelAngle::FromRadians(-t * gpTweakPlayerGun->GetFixedVerticalAim()));
      result = quat.BuildTransform4f() * rotation.GetRotation();
      result.SetTranslation(position);
    }
    break;
  }
  default:
    break;
  }
  mTransform = result;
}

CPlayer* CPlayerGunBase::GetPlayer(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerUniqueId));
}

CPlayer* CPlayerGunBase::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerUniqueId));
}

void CPlayerGunBase::AddGunDrawBlock() { ++mGunDrawBlockCount; }

void CPlayerGunBase::RemoveGunDrawBlock() {
  if (mGunDrawBlockCount != 0) {
    --mGunDrawBlockCount;
  }
}
