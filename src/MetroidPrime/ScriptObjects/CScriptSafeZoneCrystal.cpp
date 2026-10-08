#include "MetroidPrime/ScriptObjects/CScriptSafeZoneCrystal.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneCrystal.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "rstl/math.hpp"

static const CWeaponTypeVulnerability
    skNormalVulnerability(1.f, CWeaponTypeVulnerability::kE_Normal, true);
static CDamageVulnerability::TWeaponVulnerability skDarkCrystalOverrides[4] = {
    CDamageVulnerability::TWeaponVulnerability(kWT_Power, skNormalVulnerability),
    CDamageVulnerability::TWeaponVulnerability(kWT_Annihilator, skNormalVulnerability),
    CDamageVulnerability::TWeaponVulnerability(kWT_Light, skNormalVulnerability),
    CDamageVulnerability::TWeaponVulnerability(kWT_Dark, skNormalVulnerability),
};
static const CWeaponTypeVulnerability
    skPassThroughVulnerability(0.f, CWeaponTypeVulnerability::kE_PassThrough, true);
static CDamageVulnerability::TWeaponVulnerability skLightCrystalOverrides[1] = {
    CDamageVulnerability::TWeaponVulnerability(kWT_Dark, skNormalVulnerability),
};

// Guessed helpers; the REL carries out-of-line copies of each.
static inline rstl::auto_ptr< TCachedToken< CScannableObjectInfo > >
LoadScannableInfo(CAssetId id) {
  if (id == kInvalidAssetId) {
    return rstl::auto_ptr< TCachedToken< CScannableObjectInfo > >();
  }
  return rs_new TCachedToken< CScannableObjectInfo >(gpSimplePool->GetObj(SObjectTag('SCAN', id)),
                                                     true);
}

static inline CElementGen* CreateEffect(CAssetId id) {
  if (id != 0 && id != kInvalidAssetId) {
    return rs_new CElementGen(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', id))),
        CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  }
  return nullptr;
}

static inline rstl::optional_object< TLockedToken< CGenDescription > >
LoadOptionalEffect(CAssetId id) {
  if (id != 0 && id != kInvalidAssetId) {
    return TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', id)));
  }
  return rstl::optional_object< TLockedToken< CGenDescription > >();
}

static inline CDamageVulnerability GetCrystalVulnerability(bool isLight) {
  if (isLight) {
    return CDamageVulnerability(CDamageVulnerability::PassThroughVulnerabilty(),
                                skLightCrystalOverrides, 1, 7);
  }
  return CDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty(), skDarkCrystalOverrides, 4,
                              7);
}

CScriptSafeZoneCrystal::CScriptSafeZoneCrystal(
    TUniqueId uid, const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf,
    const CActorParameters& actorParms, CAssetId scanCollapsed, CAssetId scanEntangled,
    CAssetId scanLight, CAssetId scanAnnihilator, const CModelData& normalModel,
    const CModelData& entangledModel, const CModelData& hurtfulModel, const CModelData& echoModel,
    CAssetId collapsedEffect, CAssetId expandedEffect, CAssetId entangledEffect,
    CAssetId hurtfulEffect, CAssetId echoEffect, CAssetId refreshEffect, float maxTimeExpanded,
    float maxTimeEntangled, float maxTimeHurtful, float maxTimeEcho, float powerBeamHP,
    float refreshTime, float refreshDelay, bool isLight, bool initiallyEntangled,
    const CVector3f& hitRadius, const CVector3f& hitOffset, const CVector3f& effectOffset,
    const CMayaSpline& fadeSpline)
: CActor(uid, name, info, 0, xf, normalModel,
         CMaterialList(kMT_Immovable, kMT_ExcludeFromLineOfSightTest,
                       isLight ? kMT_NonSolidDamageable : kMT_Solid),
         actorParms, kInvalidUniqueId)
, mScanCollapsed(LoadScannableInfo(scanCollapsed).release())
, mScanEntangled(LoadScannableInfo(scanEntangled).release())
, mScanLight(LoadScannableInfo(scanLight).release())
, mScanAnnihilator(LoadScannableInfo(scanAnnihilator).release())
, mModels(nullptr)
, mCollapsedEffect(CreateEffect(collapsedEffect))
, mExpandedEffect(CreateEffect(expandedEffect))
, mEntangledEffect(CreateEffect(entangledEffect))
, mHurtfulEffect(CreateEffect(hurtfulEffect))
, mEchoEffect(CreateEffect(echoEffect))
, mRefreshEffectDesc(LoadOptionalEffect(refreshEffect))
, mRefreshEffect(nullptr)
, mRefreshCooldown(0.f)
, mMaxTimeExpanded(maxTimeExpanded)
, mMaxTimeEntangled(maxTimeEntangled)
, mMaxTimeHurtful(maxTimeHurtful)
, mMaxTimeEcho(maxTimeEcho)
, mRefreshTime(refreshTime)
, mRefreshDelay(refreshDelay)
, mStateTimer(0.f)
, mRefreshTimer(0.f)
, mFadeTimer(0.f)
, mIsLight(isLight)
, mDamaged(false)
, mFadeDone(false)
, mCollapsedAlive(false)
, mExpandedAlive(false)
, mHurtfulAlive(false)
, mEntangledAlive(false)
, mEchoAlive(false)
, mDisabled(false)
, mInitiallyEntangled(initiallyEntangled)
, mHitRadius(hitRadius)
, mHitOffset(hitOffset)
, mEffectOffset(effectOffset)
, mState(initiallyEntangled ? kS_Entangled : (isLight ? kS_Collapsed : kS_Expanded))
, mHealthInfo(powerBeamHP, 0.f)
, mVulnerability(GetCrystalVulnerability(isLight))
, mBounds(CAABox::MakeMaxInvertedBox())
, mFadeSpline(fadeSpline) {
  if (!isLight) {
    mModels = rs_new SSafeZoneCrystalModels(normalModel, entangledModel, hurtfulModel, echoModel);
  }
  UpdateBounds();
  UpdateVulnerability();
}

CScriptSafeZoneCrystal::~CScriptSafeZoneCrystal() {}

void CScriptSafeZoneCrystal::Touch(CActor& actor, CStateManager& mgr) {
  if (mIsLight && GetActive() && !mDisabled) {
    if (CGameProjectile* proj = TCastToPtr< CGameProjectile >(actor)) {
      int msg = -1;
      switch (proj->GetType()) {
      case kWT_Light:
        msg = kSM_InternalMessage02;
        break;
      case kWT_Annihilator:
        msg = kSM_InternalMessage03;
        break;
      case kWT_Power:
        if (mState != kS_Entangled) {
          msg = kSM_InternalMessage00;
        }
        if (mState == kS_Expanded) {
          SpawnRefreshEffect();
        }
        break;
      }
      if (msg != -1) {
        AcceptScriptMsg(mgr, CScriptMsg(proj->GetUniqueId(), GetUniqueId(),
                                        static_cast< EScriptObjectMessage >(msg)));
        AcceptScriptMsg(mgr, CScriptMsg(proj->GetUniqueId(), GetUniqueId(), kSM_Damage));
      }
    }
  }
}

void CScriptSafeZoneCrystal::SpawnRefreshEffect() {
  if (mRefreshEffectDesc && mRefreshCooldown <= 0.f) {
    mRefreshEffect =
        rs_new CElementGen(*mRefreshEffectDesc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mRefreshEffect->SetGlobalTranslation(GetTranslation() + mEffectOffset);
    mRefreshCooldown = 0.1f;
  }
}

void CScriptSafeZoneCrystal::AddEffectToRenderer(CElementGen* gen) {
  if (gen != nullptr && (gen->GetParticleEmission() || !gen->IsSystemDeletable())) {
    gpRender->AddParticleGen(*gen);
  }
}

void CScriptSafeZoneCrystal::AddToRenderer(const CStateManager& mgr) const {
  if (!mIsLight || mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan) {
    CActor::AddToRenderer(mgr);
  }
  AddEffectToRenderer(mCollapsedEffect.get());
  AddEffectToRenderer(mExpandedEffect.get());
  AddEffectToRenderer(mEntangledEffect.get());
  AddEffectToRenderer(mHurtfulEffect.get());
  AddEffectToRenderer(mEchoEffect.get());
  AddEffectToRenderer(mRefreshEffect.get());
}

CHealthInfo* CScriptSafeZoneCrystal::HealthInfo() { return &mHealthInfo; }

void CScriptSafeZoneCrystal::ResetHealth() {
  mHealthInfo.SetHP(mHealthInfo.GetInitialHP());
  mDamaged = false;
}

void CScriptSafeZoneCrystal::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CActor::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_Create) {
    UpdateModel(mgr);
  }
  if (GetActive()) {
    switch (msg.GetMessage()) {
    case kSM_Damage:
      if (!mDisabled) {
        if (mDamaged) {
          ResetHealth();
        }
        mDamaged = true;
        return;
      }
      break;
    case kSM_InternalMessage02:
      if (!mDisabled) {
        ResetHealth();
        SetState(mgr, kS_Hurtful);
        mStateTimer = mMaxTimeHurtful;
        return;
      }
      break;
    case kSM_InternalMessage01:
      if (!mDisabled) {
        ResetHealth();
        SetState(mgr, kS_Entangled);
        mStateTimer = mMaxTimeEntangled;
        return;
      }
      break;
    case kSM_InternalMessage03:
      if (!mDisabled) {
        ResetHealth();
        SetState(mgr, kS_Echo);
        mStateTimer = mMaxTimeEcho;
        return;
      }
      break;
    case kSM_InternalMessage04:
      ResetHealth();
      SetState(mgr, kS_Expanded);
      mStateTimer = mMaxTimeExpanded;
      return;
    case kSM_InternalMessage00:
      if (!mDisabled) {
        mDamaged = false;
        if (mState != kS_Entangled) {
          ResetHealth();
        }
        switch (mState) {
        case kS_Collapsed:
          SetState(mgr, kS_Expanded);
          return;
        case kS_Expanded:
          mStateTimer = mMaxTimeExpanded;
          return;
        case kS_Entangled:
          if (mHealthInfo.GetHP() <= 0.f) {
            mStateTimer = 0.f;
            mInitiallyEntangled = false;
            return;
          }
          break;
        }
      }
      break;
    case kSM_InternalMessage05:
      mDisabled = true;
      return;
    case kSM_InternalMessage06:
      mDisabled = false;
      break;
    }
  }
}

void CScriptSafeZoneCrystal::SetState(CStateManager& mgr, EState state) {
  if (state == mState) {
    return;
  }
  ResetHealth();
  switch (state) {
  case kS_Entangled:
    mStateTimer = mMaxTimeEntangled;
    SendScriptMsgs(kSS_InternalState00, mgr);
    break;
  case kS_Collapsed:
    SendScriptMsgs(kSS_InternalState00, mgr);
    if (mIsLight) {
      SendScriptMsgs(kSS_InternalState05, mgr);
    }
    break;
  case kS_Expanded:
    SendScriptMsgs(kSS_InternalState01, mgr);
    mStateTimer = mMaxTimeExpanded;
    break;
  case kS_Hurtful:
    SendScriptMsgs(kSS_InternalState02, mgr);
    mStateTimer = mMaxTimeHurtful;
    break;
  case kS_Echo:
    SendScriptMsgs(kSS_InternalState03, mgr);
    mStateTimer = mMaxTimeEcho;
    break;
  }
  mState = state;
  UpdateModel(mgr);
  UpdateVulnerability();
  mFadeTimer = 0.f;
  mFadeDone = false;
}

void CScriptSafeZoneCrystal::UpdateVulnerability() {
  if (mIsLight) {
    const CWeaponTypeVulnerability& vuln =
        mState == kS_Entangled ? skNormalVulnerability : skPassThroughVulnerability;
    mVulnerability.SetVulnerability(kWT_Power, vuln);
    mVulnerability.SetComboVulnerability(kWT_Power, vuln);
    mVulnerability.SetChargedVulnerability(kWT_Power, vuln);
  }
}

void CScriptSafeZoneCrystal::UpdateModel(CStateManager& mgr) {
  if (SSafeZoneCrystalModels* models = mModels.get()) {
    const CModelData* model = nullptr;
    switch (mState) {
    case kS_Entangled:
      model = &models->mEntangled;
      break;
    case kS_Expanded:
      model = &models->mNormal;
      break;
    case kS_Hurtful:
      model = &models->mHurtful;
      break;
    case kS_Echo:
      model = &models->mEcho;
      break;
    }
    if (model == nullptr) {
      if (!NullModel()) {
        SetModelData(CModelData::CModelDataNull(), mgr);
      }
    } else {
      SetModelData(*model, mgr);
    }
  }
}

void CScriptSafeZoneCrystal::UpdateBounds() {
  const CVector3f center = GetTranslation() + mHitOffset;
  mBounds = CAABox(center - mHitRadius, center + mHitRadius);
}

rstl::optional_object< CAABox > CScriptSafeZoneCrystal::GetTouchBounds() const { return mBounds; }

void CScriptSafeZoneCrystal::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (GetTransformDirtySpare()) {
      UpdateBounds();
      SetTransformDirtySpare(false);
    }
    UpdateStateTimer(dt, mgr);
    UpdateEffectsVisibility(dt, mgr);
    UpdateRegeneration(dt, mgr);
    UpdateRefreshEffect(dt, mgr);
    if (!mFadeDone) {
      const CVector3f effectPos = GetTranslation() + mEffectOffset;
      const float alpha =
          0.01f *
          mFadeSpline.EvaluateAt(
              (effectPos - mgr.CameraManager(0)->GetCurrentCamera(mgr, true)->GetTranslation())
                  .Magnitude());
      mCollapsedAlive =
          UpdateEffect(dt, mCollapsedEffect.get(), mCollapsedAlive, effectPos, kS_Collapsed, alpha);
      mExpandedAlive =
          UpdateEffect(dt, mExpandedEffect.get(), mExpandedAlive, effectPos, kS_Expanded, alpha);
      mHurtfulAlive =
          UpdateEffect(dt, mHurtfulEffect.get(), mHurtfulAlive, effectPos, kS_Hurtful, alpha);
      mEntangledAlive =
          UpdateEffect(dt, mEntangledEffect.get(), mEntangledAlive, effectPos, kS_Entangled, alpha);
      mEchoAlive = UpdateEffect(dt, mEchoEffect.get(), mEchoAlive, effectPos, kS_Echo, alpha);
    }
  }
}

void CScriptSafeZoneCrystal::UpdateStateTimer(float dt, CStateManager& mgr) {
  mStateTimer = rstl::max_val(0.f, mStateTimer - dt);
  switch (mState) {
  case kS_Expanded:
    if (mIsLight && mStateTimer <= 0.f) {
      SetState(mgr, kS_Collapsed);
    }
    break;
  case kS_Collapsed:
    break;
  case kS_Hurtful:
  case kS_Echo:
    if (mStateTimer <= 0.f) {
      SetState(mgr, mIsLight ? kS_Collapsed : kS_Expanded);
    }
    break;
  case kS_Entangled:
    if (mStateTimer <= 0.f && !mInitiallyEntangled) {
      SetState(mgr, mIsLight ? kS_Collapsed : kS_Expanded);
    }
    break;
  }
}

void CScriptSafeZoneCrystal::UpdateRegeneration(float dt, CStateManager& mgr) {
  if (mIsLight && mState != kS_Entangled && mState != kS_Collapsed && mStateTimer <= mRefreshTime) {
    if (mRefreshTimer <= 0.f) {
      SendScriptMsgs(kSS_InternalState04, mgr);
      mRefreshTimer = mRefreshDelay;
    } else {
      mRefreshTimer -= dt;
      if (mRefreshTimer < 0.f) {
        mRefreshTimer = 0.f;
      }
    }
  } else {
    mRefreshTimer = 0.f;
  }
}

void CScriptSafeZoneCrystal::UpdateRefreshEffect(float dt, CStateManager& mgr) {
  if (mRefreshEffectDesc) {
    mRefreshCooldown = rstl::max_val(0.f, mRefreshCooldown - dt);
    if (mRefreshEffect.get() != nullptr) {
      mRefreshEffect->SetGlobalTranslation(GetTranslation() + mEffectOffset);
      mRefreshEffect->Update(dt);
      if (mRefreshEffect->IsSystemDeletable() || gpMain->IsMaxSpeed()) {
        mRefreshEffect = nullptr;
      }
    }
  }
}

void CScriptSafeZoneCrystal::UpdateEffectsVisibility(float dt, CStateManager& mgr) {
  bool hidden = false;
  if (mgr.World()->Area(GetCurrentAreaId())->GetOcclusionState() == CGameArea::kOS_Occluded) {
    hidden = true;
    mFadeTimer = 1.f;
  } else {
    if (GetPreRenderClipped()) {
      mFadeTimer = rstl::min_val(1.f, mFadeTimer + dt);
    } else {
      mFadeTimer = 0.f;
    }
    if (mFadeTimer >= 1.f) {
      hidden = false;
    }
  }
  if (hidden && !mFadeDone) {
    mCollapsedAlive = KillEffectIfInactive(mCollapsedEffect.get(), kS_Collapsed);
    mExpandedAlive = KillEffectIfInactive(mExpandedEffect.get(), kS_Expanded);
    mHurtfulAlive = KillEffectIfInactive(mHurtfulEffect.get(), kS_Hurtful);
    mEntangledAlive = KillEffectIfInactive(mEntangledEffect.get(), kS_Entangled);
    mEchoAlive = KillEffectIfInactive(mEchoEffect.get(), kS_Echo);
  }
  mFadeDone = hidden;
}

bool CScriptSafeZoneCrystal::KillEffectIfInactive(CElementGen* gen, EState state) const {
  if (mState != state) {
    gen->DestroyParticles();
    return false;
  }
  return true;
}

bool CScriptSafeZoneCrystal::UpdateEffect(float dt, CElementGen* gen, bool alive,
                                          const CVector3f& pos, EState state, float alpha) const {
  if (gen != nullptr) {
    bool active = mState == state;
    if (!active && !alive) {
      return false;
    }
    gen->SetParticleEmission(active);
    gen->SetGeneratorRate(alpha);
    gen->SetGlobalTranslation(pos);
    if (!gpMain->IsMaxSpeed()) {
      gen->Update(dt);
    }
    if (!active) {
      return !gen->IsSystemDeletable();
    }
  }
  return true;
}

const CDamageVulnerability* CScriptSafeZoneCrystal::GetDamageVulnerability() const {
  return &mVulnerability;
}

CScannableObjectInfo* CScriptSafeZoneCrystal::GetScannableObjectInfo() const {
  TCachedToken< CScannableObjectInfo >* token = nullptr;
  switch (mState) {
  case kS_Expanded:
    break;
  case kS_Collapsed:
    token = mScanCollapsed.get();
    break;
  case kS_Entangled:
    token = mScanEntangled.get();
    break;
  case kS_Hurtful:
    token = mScanLight.get();
    break;
  case kS_Echo:
    token = mScanAnnihilator.get();
    break;
  }
  if (token != nullptr) {
    return token->GetObject();
  }
  return CActor::GetScannableObjectInfo();
}

SSafeZoneCrystalModels::SSafeZoneCrystalModels(const CModelData& normal,
                                               const CModelData& entangled,
                                               const CModelData& hurtful, const CModelData& echo)
: mNormal(normal), mEntangled(entangled), mHurtful(hurtful), mEcho(echo) {}

CEntity* REL_LoadSafeZoneCrystal(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSafeZoneCrystal sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSafeZoneCrystal.inc"

  const TUniqueId uid = mgr.AllocateUniqueId();
  const CVector3f& scale = sldrThis.editorProperties.transform.scale;
  CModelData normalModel(sldrThis.normalCrystal == kInvalidAssetId
                             ? CModelData::CModelDataNull()
                             : CModelData(CStaticRes(sldrThis.normalCrystal, scale)));
  CModelData entangledModel(sldrThis.entangledCrystal == kInvalidAssetId
                                ? CModelData::CModelDataNull()
                                : CModelData(CStaticRes(sldrThis.entangledCrystal, scale)));
  CModelData hurtfulModel(sldrThis.hurtfulCrystal == kInvalidAssetId
                              ? CModelData::CModelDataNull()
                              : CModelData(CStaticRes(sldrThis.hurtfulCrystal, scale)));
  CModelData echoModel(sldrThis.echoCrystal == kInvalidAssetId
                           ? CModelData::CModelDataNull()
                           : CModelData(CStaticRes(sldrThis.echoCrystal, scale)));
  return rs_new CScriptSafeZoneCrystal(
      uid, LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.editorProperties.name,
      LdrToTransform4f(sldrThis.editorProperties), LdrToActorParameters(sldrThis.actorParameters),
      sldrThis.scannableInfoCollapsed, sldrThis.scannableInfoEntangled, sldrThis.scannableInfoLight,
      sldrThis.scannableInfoAnnihilator, normalModel, entangledModel, hurtfulModel, echoModel,
      sldrThis.collapsedEffect, sldrThis.expandedEffect, sldrThis.entangledEffect,
      sldrThis.hurtfulEffect, sldrThis.echoEffect, sldrThis.powerBeamRefreshEffect,
      sldrThis.maxTimeExpanded, sldrThis.maxTimeEntangled, sldrThis.unknown_0xf0a45c32,
      sldrThis.unknown_0xd8116003, sldrThis.powerBeamHP, sldrThis.unknown_0x415046ed,
      sldrThis.unknown_0xec9c01b2, sldrThis.safezoneType == 1, sldrThis.initiallyEntangled,
      sldrThis.hitRadius, sldrThis.hitOffset, sldrThis.effectOffset, sldrThis.unknown_0xbbbee60b);
}
