#include "MetroidPrime/Player/CPlayerGunBase.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

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
, x3ae_25_(false)
, mInBigStrike(false)
, mMissileMode(false)
, mInPhazonPool(false) {}

CPlayerGunBase::~CPlayerGunBase() {}

CPlayer* CPlayerGunBase::GetPlayer(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerUniqueId));
}

CPlayer* CPlayerGunBase::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(mPlayerUniqueId));
}

CWorldShadow* CPlayerGunBase::GetWorldShadow() { return mWorldShadow.get(); }
const CWorldShadow* CPlayerGunBase::GetWorldShadow() const { return mWorldShadow.get(); }
void CPlayerGunBase::AddGunDrawBlock() { ++mGunDrawBlockCount; }
void CPlayerGunBase::RemoveGunDrawBlock() {
  if (mGunDrawBlockCount != 0) {
    --mGunDrawBlockCount;
  }
}

void CPlayerGunBase::Reset(CStateManager& mgr) {
  const bool wasInBigStrike = mInBigStrike;
  mInBigStrike = true;
  ProcessInput(CFinalInput(), mgr);
  mInBigStrike = wasInBigStrike;
  mGunDrawBlockCount = 0;
}

void CPlayerGunBase::Update(float dt, CStateManager& mgr) {
  // TODO: Read underwater state from this player's current camera.
  mFiredWeaponFlags = 0;
  if (mCooldown > 0.f) {
    mCooldown -= dt;
  }
  if (mSecondaryCooldown > 0.f) {
    mSecondaryCooldown -= dt;
  }
  mRainSplashGenerator->Update(dt, mgr);
}

void CPlayerGunBase::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Populate fire/charge/missile flags using the player's input mapping and strike gates.
}

void CPlayerGunBase::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: Gun-light lifecycle, player sound volume, and Phazon-pool entry/exit messages.
  CEntity::AcceptScriptMsg(mgr, msg);
}

void CPlayerGunBase::UpdateTransform(CStateManager& mgr, const CVector3f& position,
                                     const CTransform4f& rotation, CTransform4f& result) {
  // TODO: Apply the holster-state quaternion rotation and update mTransform.
}

void CPlayerGunBase::UpdateGunHolster(const CFinalInput& input, CStateManager& mgr) {
  // TODO: Resolve scan/cinematic/input transitions and advance drawing/holstering timers.
}

void CPlayerGunBase::DrawGun(CStateManager& mgr) {
  // TODO: Check grapple cooldown before entering Drawing with the draw timer.
}

void CPlayerGunBase::HolsterGun(CStateManager& mgr) {
  // TODO: Select the normal/morph holster duration, reverse a partial draw, and clear aim.
}

void CPlayerGunBase::Holster(CStateManager& mgr) {
  mGunHolsterState = kGHS_Holstered;
  mGunHolsterRemTime = 0.f;
  // TODO: Clear the player's aim target.
}

void CPlayerGunBase::CreateGunLight(CStateManager& mgr) {
  // TODO: Allocate/register the gun's CGameLight and store its unique ID.
}

void CPlayerGunBase::DeleteGunLight(CStateManager& mgr) {
  // TODO: Free the script light before invalidating its ID.
}
