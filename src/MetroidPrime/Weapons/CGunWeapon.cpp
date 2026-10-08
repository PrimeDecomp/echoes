#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/GunResNames.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/GunController/CGunController.hpp"
#include "MetroidPrime/Weapons/WeaponSound.hpp"
#include "Weapons/CWeaponDescription.hpp"

const char* const CGunWeapon::skMuzzleLocator = "LBEAM";
const char* const CGunWeapon::skElbowLocator = "elbow";

CPlayerState::EBeamId GetWeaponIndex(EWeaponType type) {
  switch (type) {
  case kWT_Power:
    return CPlayerState::kBI_Power;
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
, mArmModel(gpSimplePool->GetObj(NWeaponRes::kVariaArm))
, mXferEffect(gpSimplePool->GetObj(NWeaponRes::skBeamXferNames[GetWeaponIndex(type)]))
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
  mSolidModelData = CModelData(CAnimRes(mAncsId, 0, mScale, 0, false));
  mHoloModelData = CModelData(CAnimRes(mAncsId, 1, mScale, 0, false));
  if (!x271_26) {
    mSolidModelData->LockTextures();
    mHoloModelData->LockTextures();
  }
  LoadSuitArm();
  mGunController = rs_new CGunController(*mSolidModelData);
  mModelTouchEnabled = true;
}

void CGunWeapon::LoadProjectileData(CStateManager& mgr) {
  CRandom16 random(mgr.GetUpdateFrameIdx());
  CGlobalRandom grand(random);
  for (int i = 0; i < mWeapons.capacity(); ++i) {
    CWeaponDescription& weapon = *mWeapons[i].GetObject();
    CVector3f velocity = CVector3f::Zero();
    if (const CVectorElement* ivec = weapon.mIVEC) {
      ivec->GetValue(0, velocity);
    }
    mVelInfo.AddVelocity(velocity);
    float tratValue = 0.f;
    if (const CRealElement* trat = weapon.mTRAT) {
      trat->GetValue(0, tratValue);
    }
    mVelInfo.AddTrat(tratValue);
    const bool homing = weapon.mHOMG;
    mVelInfo.AddTargetHoming(homing);
    if (velocity.GetY() > 0.f) {
      mVelInfo.Velocity(i) *= 60.f;
    } else {
      mVelInfo.Velocity(i) = CVector3f::Forward();
    }
  }
}

void CGunWeapon::EnterComboFire(CStateManager& mgr) {
  if (!mGunController.null()) {
    mGunController->EnterComboFire(mgr, mBeamId);
  }
}

bool CGunWeapon::IsChargeAnimOver() const {
  if (mEnableCharge) {
    if (mSolidModelData->GetAnimationData()->IsAnimTimeRemaining(0.001f,
                                                                 rstl::string_l("Whole Body"))) {
      return false;
    }
  }
  return true;
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
  if (!mLoaded) {
    return 0.f;
  }
  if (int(type) < 0 || int(type) > 11) {
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
  if (mLoaded) {
    if (mSpecialAnimationPlaying) {
      CAnimData& animData = *mSolidModelData->AnimationData();
      if (mAnimationTimer == 0.f) {
        mAnimationTimer = animData.GetAnimationDuration(mAnimIds[15]);
      }
      if (mSpeedUpAnimation && animData.GetPlaybackRate() != 3.f) {
        mAnimationTimer /= 3.f;
        animData.SetPlaybackRate(3.f);
      }
      mAnimationTimer -= dt;
      if (mAnimationTimer <= 0.f) {
        mAnimationTimer = 0.f;
        mSpecialAnimationPlaying = false;
        mSpeedUpAnimation = false;
        animData.SetPlaybackRate(1.f);
      }
    }
    mSolidModelData->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
    if (!mGunController.null()) {
      mGunController->Update(dt, mgr);
    }
    if (mSuitArmLocked) {
      LoadSuitArm();
    }
  } else if (mGunCharacter && mGunCharacter->HasLock()) {
    if (mGunCharacter->IsLoaded()) {
      if ((mLoadFlags & 1) != 1) {
        LoadGunModels();
        LoadAnimations();
        mLoadFlags |= 1;
      }
      if ((mLoadFlags & 8) != 8 && IsAnimsLoaded()) {
        mLoadFlags |= 8;
      }
    }
    LoadFxIdle(dt, mgr);
    if ((mLoadFlags & 0x1f) == 0x1f) {
      CSkinnedModel& model = mSolidModelData->PickAnimatedModel(CModelData::kWM_Normal);
      const bool modelLoaded = model.GetModel()->IsLoaded(mShaderIdx);
      const bool armLoaded = mSuitArmModelData->IsLoaded(mCurrentPlayerSuit);
      if (modelLoaded && armLoaded) {
        mLoaded = true;
      }
    }
  }
  if (!mgr.IsMultiplayer()) {
    mCurrentPlayerSuit = GetPlayer(mgr)->GetPlayerState()->GetCurrentSuitRaw();
  }
}

void CGunWeapon::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mLoaded && !mFrozenGenerator.null() && mFrozenEffect != kFFT_None) {
    mFrozenGenerator->Render();
  }
}

void CGunWeapon::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  if (mLoaded && mFrozenEffect != kFFT_None) {
    if (mFrozenEffect == kFFT_Thawed) {
      if (mFrozenGenerator.null() || mFrozenGenerator->IsSystemDeletable()) {
        mFrozenEffect = kFFT_None;
        mFrozenGenerator = nullptr;
      } else {
        mFrozenGenerator->SetTranslation(xf.GetTranslation());
        mFrozenGenerator->SetOrientation(xf.GetRotation());
      }
    } else if (!mFrozenGenerator.null()) {
      mFrozenGenerator->SetGlobalOrientAndTrans(xf);
    }
    if (!mFrozenGenerator.null()) {
      mFrozenGenerator->Update(dt);
    }
  }
}

void CGunWeapon::UpdateMuzzleFx(float dt, const CVector3f& scale, const CVector3f& pos,
                                bool emitting) {
  if (!mMuzzleGenerators.empty() && mMuzzleGenerators[mMuzzleEffectIdx].get()) {
    mMuzzleGenerators[mMuzzleEffectIdx].get()->SetGlobalTranslation(pos);
    mMuzzleGenerators[mMuzzleEffectIdx].get()->SetGlobalScale(scale);
    mMuzzleGenerators[mMuzzleEffectIdx].get()->SetParticleEmission(emitting);
    mMuzzleGenerators[mMuzzleEffectIdx].get()->Update(dt);
  }
}

CElementGen* CGunWeapon::GetMuzzleFx(int index) const {
  if (!mMuzzleGenerators.empty()) {
    if (CElementGen* gen = mMuzzleGenerators[index].get()) {
      return gen;
    }
  }
  return nullptr;
}

void CGunWeapon::DrawMuzzleFx(const CStateManager& mgr) const {
  if (mLoaded && !mMuzzleGenerators.empty()) {
    if (CElementGen* effect = mMuzzleGenerators[mMuzzleEffectIdx].get()) {
      effect->Render();
    }
  }
}

void CGunWeapon::ActivateCharge(bool enable, bool resetEffect) {
  if (mLoaded && !mMuzzleGenerators.empty()) {
    if (CElementGen* effect = mMuzzleGenerators[mMuzzleEffectIdx].get()) {
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
  if (!mLoaded) {
    return;
  }
  const CTransform4f armXf =
      xf * mSolidModelData->GetScaledLocatorTransform(rstl::string_l(skElbowLocator));
  CSkinnedModel::SetPointGeneratorFunc(const_cast< CGunWeapon* >(this),
                                       &CGunWeapon::PointGenerator);
  if (!mDrawHologram) {
    mSolidModelData->Render(mgr, xf, lights, flags.UseShaderSet(mShaderIdx));
  } else {
    DrawHologram(mgr, xf, flags);
  }
  if (drawSuitArm && mSuitArmModelData) {
    mSuitArmModelData->Render(mgr, armXf, lights, flags.UseShaderSet(mCurrentPlayerSuit));
  }
  CSkinnedModel::ClearPointGeneratorFunc();
  if (mRainSplashGenerator && mRainSplashGenerator->IsRaining()) {
    mRainSplashGenerator->Draw(xf);
  }
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId));
  const CTransform4f view = CGraphics::GetViewMatrix();
  CGraphics::SetViewPointMatrix(xf.GetInverse() * view);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (CElementGen* effect = player->GetDarkAetherParticles()) {
    effect->Render();
  }
  CGraphics::SetViewPointMatrix(view);
}

void CGunWeapon::DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                              const CModelFlags& flags) const {
  if (!mLoaded) {
    return;
  }
  if (mDrawHologram) {
    mHoloModelData->RenderSolid(CModelData::kWM_Normal, xf, false, flags);
  } else {
    const CVector3f& scale = mSolidModelData->GetScale();
    CTransform4f modelMatrix(xf);
    modelMatrix *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    gpRender->SetModelMatrix(modelMatrix);
    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(CColor::White());
    mSolidModelData->GetAnimationData()->Render(
        **mHoloModelData->GetAnimationData()->GetModelData(), flags);
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
  }
}

void CGunWeapon::Fire(const TCachedToken< CWeaponDescription >& projectile, bool underwater,
                      float dt, CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, uint projectileAttributes,
                      ushort soundId, TUniqueId* projectileId, CSfxHandle* soundHandle,
                      float chargeFactor1, float chargeFactor2) {
  CDamageInfo damage(GetDamageInfo(mgr, chargeState, chargeFactor1));
  CVector3f scale =
      (chargeState == CPlayerState::kCS_Normal && (projectileAttributes & 8) == 0 ? 1.f
                                                                                  : chargeFactor2) *
      CVector3f::One();
  const bool partialCharge =
      chargeState == CPlayerState::kCS_Normal ? false : !close_enough(chargeFactor1, 1.f);
  const uint chargeAttributes =
      (partialCharge ? CWeapon::kPA_ParticleOPTS : 0) |
      (chargeState != CPlayerState::kCS_Normal ? CWeapon::kPA_Charged : 0) | projectileAttributes;
  CPlayer* player = GetPlayer(mgr);
  const int absorbedShots = player->GetPlayerGun()->GetAbsorbedPhazonShots();
  if (absorbedShots == gpTweakPlayerGun->GetMaxAbsorbedPhazonShots()) {
    const float factor =
        float(absorbedShots) / float(gpTweakPlayerGun->GetMaxAbsorbedPhazonShots());
    const SWeaponInfo info = gpTweakPlayerGun->GetPhazonBeamInfo();
    CDamageInfo phazonDamage(info.mCharged.GetWeaponMode(), factor * info.mCharged.GetDamage(),
                             factor * info.mCharged.GetRadius(),
                             factor * info.mCharged.GetKnockBackPower());
    phazonDamage.SetRadiusDamage(factor * info.mCharged.GetRadiusDamage());
    phazonDamage.SetApplyRadiusDamage(false);
    scale = factor * CVector3f::One();
    damage = phazonDamage;
  }
  CEnergyProjectile* proj = rs_new CEnergyProjectile(
      true, projectile, mWeaponType, xf, kMT_ProjectilePassthrough, damage, mgr.AllocateUniqueId(),
      player->GetCurrentAreaId(), mPlayerId, homingTarget, chargeAttributes, underwater, scale,
      CImpactVisorEffect::None(), false, true, false, chargeFactor1, 4.f, 4.f);
  if (proj) {
    mgr.AddObject(proj);
    if (chargeState != CPlayerState::kCS_Normal && chargeFactor1 == 1.f) {
      proj->SetX4104(true);
    }
    const rstl::optional_object< CAABox > playerBounds = player->GetTouchBounds();
    if (playerBounds) {
      const rstl::list< CEntity* >& docks = mgr.GetDockList();
      for (rstl::list< CEntity* >::const_iterator it = docks.begin(); it != docks.end(); ++it) {
        const CActor* dock = static_cast< CActor* >(*it);
        if (dock->GetCurrentAreaId() == player->GetCurrentAreaId()) {
          const rstl::optional_object< CAABox > dockBounds = dock->GetTouchBounds();
          if (dockBounds && dockBounds->DoBoundsOverlap(*playerBounds)) {
            proj->SetTouchedDock(dock->GetUniqueId());
          }
        }
      }
    }
    proj->InitializeMuzzleOffset(mWeaponType == kWT_Light ? FLT_MAX : 0.5f, mgr);
    proj->SetFluidList(player->GetCameraManager()->GetFirstPersonCamera()->GetFluidList());
    proj->Think(dt, mgr);
  }
  if (projectileId) {
    *projectileId = proj ? proj->GetUniqueId() : kInvalidUniqueId;
  }
  if (chargeState != CPlayerState::kCS_Normal && (projectileAttributes & 0x800000) == 0) {
    mEnableCharge = true;
    const CCameraShakerData shaker = gpTweakPlayerGunSingle->GetRecoilCameraShakerData();
    GetPlayerFromAll(mgr)->CameraManager()->CameraShakerManager()->AddCameraShaker(shaker, mgr,
                                                                                   false, false);
  }
  if ((projectileAttributes & 0x1000000) == 0) {
    CAnimData& animData = *mSolidModelData->AnimationData();
    animData.EnableLooping(false);
    animData.SetAnimation(CAnimPlaybackParms(mShootAnimIds[chargeState], -1, 1.f, true), false);
  }
  if (soundId != CSfxManager::kInternalInvalidSfxId) {
    const CSfxHandle handle = PlaySfxForPlayer(GetPlayer(mgr), soundId, mSoundVolume,
                                               mgr.GetNextAreaId().Value(), underwater, false);
    if (soundHandle) {
      *soundHandle = handle;
    }
  }
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
  const SWeaponInfo& info = GetWeaponInfo();
  if (chargeState == CPlayerState::kCS_Normal) {
    return info.mNormal.ApplyDoubleDamage(*GetPlayer(mgr)->GetPlayerState());
  }
  const CDamageInfo& charged = info.mCharged;
  if (chargeFactor == 1.f) {
    return charged.ApplyDoubleDamage(*GetPlayer(mgr)->GetPlayerState());
  }
  CDamageInfo damage(charged.GetWeaponMode(), chargeFactor * charged.GetDamage(),
                     chargeFactor * charged.GetRadius(), chargeFactor * charged.GetKnockBackPower(),
                     false, false);
  damage.SetRadiusDamage(chargeFactor * charged.GetRadiusDamage());
  return damage.ApplyDoubleDamage(*GetPlayer(mgr)->GetPlayerState());
}

CAABox CGunWeapon::GetBounds() const {
  if (mSolidModelData) {
    if (!mBounds) {
      mBounds = CAABox::MakeMaxInvertedBox();
      CAABox& bounds = *mBounds;
      const rstl::vector< rstl::pair< rstl::string, CAABox > >& boxes =
          mSolidModelData->GetAnimationData()->GetCharacterInfo().GetAnimBBoxList();
      for (int i = 0; i < boxes.size(); ++i) {
        const CAABox& box = boxes[i].second;
        bounds.AccumulateBounds(box.GetMinPoint());
        bounds.AccumulateBounds(box.GetMaxPoint());
      }
    }
    return *mBounds;
  }
  return CAABox::Identity();
}

CAABox CGunWeapon::GetBounds(const CTransform4f& xf) const {
  return GetBounds().GetTransformedAABox(xf);
}

void CGunWeapon::Touch(const CStateManager& mgr) {
  if (!mgr.IsMultiplayer() && mSolidModelData) {
    if (mModelTouchEnabled) {
      mSolidModelData->Touch(mgr, mShaderIdx);
    }
    if (mSuitArmModelData) {
      mSuitArmModelData->Touch(mgr, mCurrentPlayerSuit);
    }
  }
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
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    mMuzzleEffects[i].Lock();
    mWeapons[i].Lock();
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    mFrozenEffects[i].Lock();
  }
}

void CVelocityInfo::Clear() {
  mVel = rstl::reserved_vector< CVector3f, 2 >();
  mTargetHoming = rstl::reserved_vector< bool, 2 >();
  mTrat = rstl::reserved_vector< float, 2 >();
}

void CGunWeapon::Unload(CStateManager& mgr) {
  const bool singlePlayer = !mgr.IsMultiplayer();
  if (singlePlayer) {
    UnlockTokens();
  }
  mLoadFlags = 0;
  mFrozenEffect = kFFT_None;
  mSolidModelData = rstl::optional_object< CModelData >();
  mHoloModelData = rstl::optional_object< CModelData >();
  mSuitArmModelData = rstl::optional_object< CModelData >();
  mGunController = nullptr;
  mRainSplashGenerator = nullptr;
  mFrozenGenerator = nullptr;
  if (singlePlayer) {
    FreeResPools();
    mGunCharacter->Unlock();
  }
  mMuzzleGenerators = rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 >();
  mVelInfo.Clear();
  mLoaded = false;
}

bool CGunWeapon::IsLoaded() const { return mLoaded; }

void CGunWeapon::AllocResPools(CPlayerState::EBeamId beam) {
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    const int idx = beam * 2 + i;
    mMuzzleEffects.push_back(
        TCachedToken< CGenDescription >(gpSimplePool->GetObj(NWeaponRes::skMuzzleNames[idx])));
    mWeapons.push_back(
        TCachedToken< CWeaponDescription >(gpSimplePool->GetObj(NWeaponRes::skWeaponNames[idx])));
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    const int idx = beam * 2 + i;
    mFrozenEffects.push_back(
        TCachedToken< CGenDescription >(gpSimplePool->GetObj(NWeaponRes::skFrozenNames[idx])));
  }
}

void CGunWeapon::FreeResPools() {
  mXferEffect.Unlock();
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    mMuzzleEffects[i].Unlock();
    mWeapons[i].Unlock();
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    mFrozenEffects[i].Unlock();
  }
  mAnims = rstl::vector< CToken >();
}

void CGunWeapon::LoadFxIdle(float dt, CStateManager& mgr) {
  if (!NWeaponTypes::are_tokens_ready(mDeps)) {
    return;
  }
  if ((mLoadFlags & 2) && (mLoadFlags & 4) && (mLoadFlags & 0x10)) {
    return;
  }
  bool loaded = true;
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    if (!mMuzzleEffects[i].TryCache()) {
      loaded = false;
      break;
    }
    if (!mWeapons[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    if (!mFrozenEffects[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  if (!mXferEffect.TryCache()) {
    loaded = false;
  }
  if (loaded) {
    if ((mLoadFlags & 2) != 2) {
      LoadMuzzleFx(dt);
      mLoadFlags |= 2;
    }
    mLoadFlags |= 0x10;
    if ((mLoadFlags & 4) != 4) {
      LoadProjectileData(mgr);
      mLoadFlags |= 4;
    }
  }
}

void CGunWeapon::LoadAnimations() {
  CAnimData& animData = *mSolidModelData->AnimationData();
  BuildAnimationIdList(animData);
  const CPASAnimState* state = animData.GetPASDatabase().GetAnimState(pas::kAS_LoopReaction);
  rstl::vector< int > animIds;
  const int numAnims = state->GetNumAnims();
  animIds.reserve(numAnims);
  for (int i = 0; i < numAnims; ++i) {
    animIds.push_back_unsafe(state->GetAnimInfoByIndex(i)->GetAnimId());
  }
  NWeaponTypes::get_token_vector(animData, animIds, mAnims, true);
  int defaultAnim = 10;
  if (mSubtypeBasePose) {
    defaultAnim = 0;
  }
  mSolidModelData->AnimationData()->SetAnimation(
      CAnimPlaybackParms(mAnimIds[defaultAnim], -1, 1.f, true), true);
}

bool CGunWeapon::IsAnimsLoaded() const {
  for (rstl::vector< CToken >::const_iterator it = mAnims.begin(); it != mAnims.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::LockTokens() {
  AsyncLoadSuitArm();
  NWeaponTypes::lock_tokens(mDeps);
}

void CGunWeapon::UnlockTokens() {
  mArmModel.Unlock();
  NWeaponTypes::unlock_tokens(mDeps);
}

void CGunWeapon::ReleaseResources(CStateManager& mgr) {
  if (!mgr.IsMultiplayer()) {
    mRainSplashGenerator = nullptr;
    mFrozenGenerator = nullptr;
    mMuzzleGenerators = rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 >();
    mXferEffect.Unlock();
    for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
      mMuzzleEffects[i].Unlock();
      mWeapons[i].Unlock();
    }
    for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
      mFrozenEffects[i].Unlock();
    }
    NWeaponTypes::unlock_tokens(mDeps);
  }
}

void CGunWeapon::FillTokenVector(const rstl::vector< SObjectTag >& tags,
                                 rstl::vector< CToken >& objects, bool includeTxtr) {
  for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    if (!includeTxtr && token.GetReferenceType() == 'TXTR') {
      continue;
    }
    objects.push_back_unsafe(token);
  }
}

void CGunWeapon::BuildDependencyList(CPlayerState::EBeamId beam) {
  const TLockedToken< CDependencyGroup > dependencies =
      gpSimplePool->GetObj(NWeaponRes::skDependencyNames[beam]);
  const TLockedToken< CDependencyGroup > animDependencies =
      gpSimplePool->GetObj(NWeaponRes::kPowerAnimDependencyGroup);
  const rstl::vector< SObjectTag >& depTags = dependencies->GetObjectTagVector();
  const rstl::vector< SObjectTag >& animTags = animDependencies->GetObjectTagVector();
  mDeps.reserve(depTags.size() + animTags.size());
  FillTokenVector(depTags, mDeps, true);
  FillTokenVector(animTags, mDeps, false);
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
  if (mGunController.null()) {
    return false;
  }
  return mGunController->IsFidgetLoaded();
}

void CGunWeapon::AsyncLoadSuitArm() {
  mSuitArmModelData = rstl::optional_object< CModelData >();
  mArmModel.Lock();
  mSuitArmLocked = true;
}

void CGunWeapon::LoadSuitArm() {
  if (mArmModel.IsLoaded()) {
    mSuitArmModelData =
        CModelData(CStaticRes(NWeaponTypes::get_asset_id_from_name(NWeaponRes::kVariaArm), mScale));
    mSuitArmLocked = false;
    if (!x271_26) {
      mSuitArmModelData->LockTextures();
      mArmModel.Unlock();
    }
  }
}

void CGunWeapon::PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                                void* context) {
  if (context) {
    CGunWeapon& weapon = *static_cast< CGunWeapon* >(context);
    if (weapon.mRainSplashGenerator) {
      if (weapon.mRainSplashGenerator->IsRaining()) {
        weapon.mRainSplashGenerator->GeneratePoints(model, workspace);
      }
      weapon.mRainSplashPosition = weapon.mRainSplashGenerator->GeneratePoint(model, workspace);
    }
  }
}

void CGunWeapon::EnableFrozenEffect(EFrozenFxType type) {
  switch (type) {
  case kFFT_Thawed:
    if (mFrozenEffect != kFFT_Thawed) {
      mFrozenGenerator = rs_new CElementGen(mFrozenEffects[1]);
      mFrozenGenerator->SetGlobalScale(mScale);
    }
    break;
  case kFFT_Frozen:
    if (mFrozenEffect != kFFT_Frozen) {
      mFrozenGenerator = rs_new CElementGen(mFrozenEffects[0]);
      mFrozenGenerator->SetGlobalScale(mScale);
    }
    break;
  }
  mFrozenEffect = type;
}

void DrawClipCube(const CAABox& aabb) {
  // Render AABB as completely transparent object, only modifying Z-buffer
  const CColor color(1.f, 1.f, 1.f, 0.f);
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetCullMode(kCM_None);

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->EndPrimitive();

  CGraphics::SetCullMode(kCM_Front);
}

void CGunWeapon::InitializeResources(CStateManager& mgr) {
  if (!mResourcesAllocated) {
    AllocResPools(mBeamId);
    BuildDependencyList(mBeamId);
    mAncsId = NWeaponTypes::get_asset_id_from_name(NWeaponRes::skBeamNames[mBeamId]);
    mGunCharacter = TToken< CAnimCharacterSet >(gpSimplePool->GetObj(SObjectTag('ANCS', mAncsId)));
    mResourcesAllocated = true;
    mCurrentPlayerSuit =
        mgr.IsMultiplayer()
            ? static_cast< CPlayerState::EPlayerSuit >(mgr.MaskUIdNumPlayers(mPlayerId))
            : GetPlayer(mgr)->GetPlayerState()->GetCurrentSuitRaw();
  }
}

void CGunWeapon::BuildAnimationIdList(const CAnimData& animData) {
  const CPASDatabase& db = animData.GetPASDatabase();
  mAnimIds.clear();
  mAnimIds.reserve(21);
  for (int i = 0; i < 21; ++i) {
    const CPASAnimParmData parms(pas::EAnimationState(9), CPASAnimParm::FromEnum(i));
    const rstl::pair< float, int > best = db.FindBestAnimation(parms, -1);
    mAnimIds.push_back_unsafe(best.second);
  }

  mShootAnimIds.clear();
  mShootAnimIds.reserve(2);
  mShootAnimIds.push_back_unsafe(mAnimIds[4]);
  mShootAnimIds.push_back_unsafe(mAnimIds[3]);
}

CPlayer* CGunWeapon::GetPlayer(CStateManager& mgr) const {
  return const_cast< CPlayer* >(TCastToConstPtr< CPlayer >(mgr.GetObjectById(mPlayerId)));
}

CPlayer* CGunWeapon::GetPlayerFromAll(CStateManager& mgr) const {
  return TCastToPtr< CPlayer >(mgr.ObjectById(mPlayerId));
}
