#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/GunController/CGunController.hpp"

static const char* const skBeamXferNames[] = {
    "PowerXfer",
    "IceXfer",
    "WaveXfer",
    "PlasmaXfer",
};

const char* CGunWeapon::skMuzzleLocator = "LBEAM";
const char* CGunWeapon::skElbowLocator = "elbow";

CPlayerState::EBeamId GetWeaponIndex(EWeaponType type) {
  switch (type) {
  case kWT_Dark:
    return CPlayerState::kBI_Dark;
  case kWT_Light:
    return CPlayerState::kBI_Light;
  case kWT_Annihilator:
    return CPlayerState::kBI_Annihilator;
  default:
    return CPlayerState::kBI_Power;
  }
}

CGunWeapon::CGunWeapon(EWeaponType type, TUniqueId playerId, const CVector3f& scale, int flags)
: mScale(scale)
, mCurrentPlayerSuit(CPlayerState::kPS_Varia)
, mArmModel(gpSimplePool->GetObj("VariaArm"))
, mXferEffect(gpSimplePool->GetObj(skBeamXferNames[GetWeaponIndex(type)]))
, mRainSplashGenerator(nullptr)
, mWeaponType(type)
, mPlayerId(playerId)
, mPlayerMaterial(kMT_Player)
, mEnabledSecondaryEffect(kSFT_None)
, mBeamId(GetWeaponIndex(type))
, mFrozenEffect(kFFT_None)
, mMuzzleEffectIdx(0)
, mShaderIdx(mBeamId)
, mLoadFlags(0)
, mAncsId(kInvalidAssetId)
, mSoundVolume(0x4a)
, mAnimationTimer(0.f)
, mRainSplashPosition(CVector3f::Zero())
, x270_24(false)
, mEnableCharge(false)
, mLoaded(false)
, mSubtypeBasePose(false)
, mSuitArmLocked(false)
, mDrawHologram(false)
, mResourcesAllocated(false)
, mSpecialAnimationPlaying(false)
, mSpeedUpAnimation(false)
, mModelTouchEnabled(true)
, x271_26(flags & 1) {}

CGunWeapon::~CGunWeapon() {}

const SWeaponInfo& CGunWeapon::GetWeaponInfo() const {
  return gpTweakPlayerGun->GetBeamInfo(mBeamId);
}

void CGunWeapon::LoadMuzzleFx(float dt) {
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    CElementGen* effect = rs_new CElementGen(mMuzzleEffects[i]);
    effect->SetParticleEmission(false);
    effect->Update(dt);
    mMuzzleGenerators.push_back(rstl::auto_ptr< CElementGen >(effect));
  }
}

void CGunWeapon::LoadGunModels() {
  // TODO: Construct solid/hologram models with the selected PAS base pose, then the controller.
}

void CGunWeapon::LoadProjectileData(CStateManager& mgr) {
  // TODO: Read both projectile descriptions into the velocity, homing and travel-rate caches.
}

void CGunWeapon::EnterComboFire(CStateManager& mgr) {
  if (!mGunController.null()) {
    mGunController->EnterComboFire(mgr, mBeamId);
  }
}

bool CGunWeapon::IsChargeAnimOver() const {
  return !mEnableCharge || !mSolidModelData->GetAnimationData()->IsAnimTimeRemaining(
                               0.001f, rstl::string_l("Whole Body"));
}

void CGunWeapon::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (!mLoaded || int(type) < 0 || int(type) > 11) {
    return;
  }

  CAnimData& animData = *mSolidModelData->AnimationData();
  animData.EnableLooping(loop);
  animData.SetAnimation(CAnimPlaybackParms(mAnimIds[type], -1, 1.f, true), false);
}

float CGunWeapon::GetAnimDuration(NWeaponTypes::EGunAnimType type) const {
  if (!mLoaded || int(type) < 0 || int(type) > 11) {
    return 0.f;
  }
  return mSolidModelData->GetAnimationData()->GetAnimationDuration(mAnimIds[type]);
}

void CGunWeapon::Reset(CStateManager& mgr) {
  if (!mLoaded) {
    return;
  }

  mSolidModelData->AnimationData()->EnableLooping(false);
  if (mEnableCharge) {
    mEnableCharge = false;
  } else if (!mGunController.null()) {
    mGunController->Reset();
  }
}

void CGunWeapon::Update(float dt, CStateManager& mgr) {
  // TODO: Poll staged loads, advance the special-animation timer/controller and update the suit.
}

void CGunWeapon::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mLoaded && !mFrozenGenerator.null() && mFrozenEffect != kFFT_None) {
    mFrozenGenerator->Render();
  }
}

void CGunWeapon::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  // TODO: Update frozen/thawing particles, including the thaw effect's local transform.
}

void CGunWeapon::UpdateMuzzleFx(float dt, const CVector3f& scale, const CVector3f& pos,
                                bool emitting) {
  if (CElementGen* effect = GetMuzzleFx(mMuzzleEffectIdx)) {
    effect->SetGlobalTranslation(pos);
    effect->SetGlobalScale(scale);
    effect->SetParticleEmission(emitting);
    effect->Update(dt);
  }
}

CElementGen* CGunWeapon::GetMuzzleFx(int index) const {
  return mMuzzleGenerators.empty() ? nullptr : mMuzzleGenerators[index].get();
}

void CGunWeapon::DrawMuzzleFx(const CStateManager& mgr) const {
  if (mLoaded) {
    if (CElementGen* effect = GetMuzzleFx(mMuzzleEffectIdx)) {
      effect->Render();
    }
  }
}

void CGunWeapon::ActivateCharge(bool enable, bool resetEffect) {
  if (mLoaded) {
    if (CElementGen* effect = GetMuzzleFx(mMuzzleEffectIdx)) {
      effect->SetParticleEmission(false);
    }
  }

  mMuzzleEffectIdx = enable ? 1 : 0;
  if (mLoaded && (enable || resetEffect) && !mMuzzleGenerators.empty()) {
    mMuzzleGenerators[mMuzzleEffectIdx] =
        rstl::auto_ptr< CElementGen >(rs_new CElementGen(mMuzzleEffects[mMuzzleEffectIdx]));
  }
}

void CGunWeapon::Draw(bool drawSuitArm, int playerIndex, const CStateManager& mgr,
                      const CTransform4f& xf, const CModelFlags& flags,
                      const CActorLights* lights) const {
  // TODO: Draw the solid/hologram model, suit arm, rain splashes and Dark Aether damage effect.
}

void CGunWeapon::DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                              const CModelFlags& flags) const {
  // TODO: Render the hologram with its clipping cube and depth/blend state.
}

void CGunWeapon::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                      float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                      ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                      float chargeFactor1, float chargeFactor2) {
  // TODO: Spawn/register the projectile, apply charge/Phazon damage and recoil, and play its sound.
  // projectileId and soundHandle are optional outputs, not input values.
}

void CGunWeapon::ReturnToDefault(CStateManager& mgr, bool reset) {
  if (!mGunController.null()) {
    if (reset) {
      mGunController->Reset();
    } else {
      mGunController->ReturnToDefault(mgr, 0.f, false);
    }
  }
}

void CGunWeapon::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet) {
  if (!mGunController.null()) {
    mGunController->EnterFidget(mgr, type, mBeamId, animSet);
  }
}

CDamageInfo CGunWeapon::GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                                      float chargeFactor) {
  // TODO: Select normal/charged tweak damage, scale partial charge and apply player damage boosts.
  return CDamageInfo();
}

CAABox CGunWeapon::GetBounds() const {
  if (!mSolidModelData) {
    return CAABox::Identity();
  }

  if (!mBounds) {
    mBounds = CAABox::MakeMaxInvertedBox();
    const rstl::vector< rstl::pair< rstl::string, CAABox > >& boxes =
        mSolidModelData->GetAnimationData()->GetCharacterInfo().GetAnimBBoxList();
    for (int i = 0; i < boxes.size(); ++i) {
      mBounds->AccumulateBounds(boxes[i].second.GetMinPoint());
      mBounds->AccumulateBounds(boxes[i].second.GetMaxPoint());
    }
  }
  return *mBounds;
}

CAABox CGunWeapon::GetBounds(const CTransform4f& xf) const {
  return GetBounds().GetTransformedAABox(xf);
}

void CGunWeapon::Touch(const CStateManager& mgr) {
  // TODO: Single-player texture touching, respecting mModelTouchEnabled and suit shader selection.
}

void CGunWeapon::TouchHolo(const CStateManager& mgr) {
  if (mHoloModelData) {
    mHoloModelData->Touch(mgr, 0);
  }
}

void CGunWeapon::Load(CStateManager& mgr, bool subtypeBasePose) {
  LockTokens();
  mSubtypeBasePose = subtypeBasePose;
  mFrozenEffect = kFFT_None;
  mFrozenGenerator = nullptr;
  mGunCharacter->Lock();
  mXferEffect.Lock();
  for (int i = 0; i < mMuzzleEffects.size(); ++i) {
    mMuzzleEffects[i].Lock();
    mWeapons[i].Lock();
  }
  for (int i = 0; i < mFrozenEffects.size(); ++i) {
    mFrozenEffects[i].Lock();
  }
}

void CVelocityInfo::Clear() {
  mVel.clear();
  mTargetHoming.clear();
  mTrat.clear();
}

void CGunWeapon::Unload(CStateManager& mgr) {
  // TODO: Multiplayer-dependent resource unlocking, then model/controller and effect cleanup.
}

bool CGunWeapon::IsLoaded() const { return mLoaded; }

void CGunWeapon::AllocResPools(CPlayerState::EBeamId beam) {
  // TODO: Populate the two projectile, muzzle-effect and frozen-effect resource tokens.
}

void CGunWeapon::FreeResPools() {
  mXferEffect.Unlock();
  for (int i = 0; i < mMuzzleEffects.size(); ++i) {
    mMuzzleEffects[i].Unlock();
    mWeapons[i].Unlock();
  }
  for (int i = 0; i < mFrozenEffects.size(); ++i) {
    mFrozenEffects[i].Unlock();
  }
  mAnims = rstl::vector< CToken >();
}

void CGunWeapon::LoadFxIdle(float dt, CStateManager& mgr) {
  // TODO: Poll effect/projectile/dependency tokens and advance the independent load flags.
}

void CGunWeapon::LoadAnimations() {
  // TODO: Build PAS animation IDs, preload the gun animation tokens and select the default pose.
}

bool CGunWeapon::IsAnimsLoaded() const {
  for (int i = 0; i < mAnims.size(); ++i) {
    if (!mAnims[i].IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::LockTokens() {
  AsyncLoadSuitArm();
  for (int i = 0; i < mDeps.size(); ++i) {
    mDeps[i].Lock();
  }
}

void CGunWeapon::UnlockTokens() {
  for (int i = 0; i < mDeps.size(); ++i) {
    mDeps[i].Unlock();
  }
}

void CGunWeapon::ReleaseResources(CStateManager& mgr) {
  // TODO: In single-player, release effect generators and unlock pooled resources/dependencies.
}

void CGunWeapon::FillTokenVector(const rstl::vector< SObjectTag >& tags,
                                 rstl::vector< CToken >& objects, bool includeTxtr) {
  for (int i = 0; i < tags.size(); ++i) {
    CToken token = gpSimplePool->GetObj(tags[i]);
    if (includeTxtr || tags[i].GetType() != 'TXTR') {
      objects.push_back(token);
    }
  }
}

void CGunWeapon::BuildDependencyList(CPlayerState::EBeamId beam) {
  // TODO: Combine the selected beam DGRP and common animation DGRP; exclude animation textures.
}

void CGunWeapon::AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet) {
  if (!mGunController.null()) {
    mGunController->LoadFidgetAnimAsync(mgr, type, mBeamId, animSet);
  }
}

void CGunWeapon::UnLoadFidget() {
  if (!mGunController.null()) {
    mGunController->UnLoadFidget();
  }
}

bool CGunWeapon::IsFidgetLoaded() {
  return !mGunController.null() && mGunController->IsFidgetLoaded();
}

void CGunWeapon::AsyncLoadSuitArm() {
  mSuitArmModelData.clear();
  mArmModel.Lock();
  mSuitArmLocked = true;
}

void CGunWeapon::LoadSuitArm() {
  // TODO: Construct the VariaArm static model once loaded, then apply its texture-lock policy.
}

void CGunWeapon::PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                                void* context) {
  // TODO: Feed the rain generator's skinning callback and retain its sampled position.
}

void CGunWeapon::EnableFrozenEffect(EFrozenFxType type) {
  if ((type == kFFT_Frozen || type == kFFT_Thawed) && mFrozenEffect != type) {
    mFrozenGenerator = rs_new CElementGen(mFrozenEffects[type - 1]);
    mFrozenGenerator->SetGlobalScale(mScale);
  }
  mFrozenEffect = type;
}

void DrawClipCube(const CAABox& bounds) {
  // TODO: Render the six clipping faces while preserving the renderer's depth/cull state.
}

void CGunWeapon::InitializeResources(CStateManager& mgr) {
  // TODO: Initialize resource pools/dependencies/ANCS once, then select the suit/player shader.
}

void CGunWeapon::BuildAnimationIdList(const CAnimData& animData) {
  mAnimIds.clear();
  mAnimIds.reserve(21);
  for (int i = 0; i < 21; ++i) {
    const CPASAnimParmData parms(pas::EAnimationState(9), CPASAnimParm::FromEnum(i));
    mAnimIds.push_back(animData.GetPASDatabase().FindBestAnimation(parms, -1).second);
  }

  mShootAnimIds.clear();
  mShootAnimIds.reserve(2);
  mShootAnimIds.push_back(mAnimIds[4]);
  mShootAnimIds.push_back(mAnimIds[3]);
}

CPlayer* CGunWeapon::GetPlayer(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
}

CPlayer* CGunWeapon::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.GetObjectByIdFromListAll(mPlayerId));
}
