#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CWeaponAssetInfo.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "dolphin/gx.h"

const int CPlasmaProjectile::kMaxPlasmaLights = 3;
const float CPlasmaProjectile::kInvMaxPlasmaLights = 1.f / CCast::ToReal32(kMaxPlasmaLights - 1);
static const CColor skCoreColor(1.f, 1.f, 1.f, 0.3f);

CPlasmaProjectile::CPlasmaProjectile(const TToken< CWeaponDescription >& description,
                                     const rstl::string& name, EWeaponType type,
                                     const CBeamInfo& beamInfo, const CTransform4f& xf,
                                     EMaterialTypes material, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                     const CWeaponAssetInfo& resources, bool drawOwnerFirst,
                                     uint attribs)
: CBeamProjectile(description, name, type, xf, beamInfo.GetLength(), beamInfo.GetRadius(),
                  beamInfo.GetTravelSpeed(), material, damage, uid, areaId, owner, attribs,
                  (beamInfo.GetBeamAttributes() & 0x200) != 0)
, mBeamAttributes(beamInfo.GetBeamAttributes())
, mLifeTime(beamInfo.GetLifeTime())
, mPulseSpeed(beamInfo.GetPulseSpeed())
, mShutdownTime(beamInfo.GetShutdownTime())
, mExpansionSpeed(beamInfo.GetExpansionSpeed())
, mMaxLength(beamInfo.GetLength() / 32.f)
, mCoreColor(skCoreColor)
, mInnerColor(beamInfo.GetInnerColor())
, mOuterColor(beamInfo.GetOuterColor())
, mPhazonDamage()
, mExpansionState(kES_Inactive)
, mInitialDamage(0.f)
, mBeamWidth(0.f)
, mLifeTimer(0.f)
, mExpansionT(0.f)
, mExpansion(0.f)
, mBeamAngle(0.f)
, mEnergyPulseStartY(0.f)
, mShutdownTimer(0.f)
, mContactPulseTimer(0.f)
, mEnergyPulseTimer(0.f)
, mPlayerEffectPulseTimer(0.f)
, mPlayerDamageDuration(0.f)
, mPlayerDamageTimer(0.f)
, mTexture(gpSimplePool->GetObj(SObjectTag('TXTR', beamInfo.GetTextureId())))
, mGlowTexture(gpSimplePool->GetObj(SObjectTag('TXTR', beamInfo.GetGlowTextureId())))
, mPulseFxDesc(gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetPulseFXId())))
, mContactFxDesc(beamInfo.GetContactFXId() != kInvalidAssetId
                     ? rstl::optional_object< TLockedToken< CGenDescription > >(
                           gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetContactFXId())))
                     : rstl::optional_object_null())
, mMuzzleFxDesc(beamInfo.GetMuzzleFXId() != kInvalidAssetId
                    ? rstl::optional_object< TLockedToken< CGenDescription > >(
                          gpSimplePool->GetObj(SObjectTag('PART', beamInfo.GetMuzzleFXId())))
                    : rstl::optional_object_null())
, mContactGen(mContactFxDesc ? rs_new CElementGen(*mContactFxDesc, CElementGen::kMOT_One) : nullptr)
, mPulseGen(rs_new CElementGen(mPulseFxDesc, CElementGen::kMOT_Normal))
, mWeaponGen()
, mMuzzleGen(mMuzzleFxDesc ? rs_new CElementGen(*mMuzzleFxDesc, CElementGen::kMOT_Normal) : nullptr)
, mMuzzleScale(1.f, 1.f, 1.f)
, mFreezeSteamTxtr(resources.GetAsset(0))
, mFreezeIceTxtr(resources.GetAsset(1))
, mVisorElectric(resources.GetAsset(2) != kInvalidAssetId
                     ? rstl::optional_object< TToken< CElectricDescription > >(
                           gpSimplePool->GetObj(SObjectTag('ELSC', resources.GetAsset(2))))
                     : rstl::optional_object_null())
, mVisorParticle(resources.GetAsset(3) != kInvalidAssetId
                     ? rstl::optional_object< TToken< CGenDescription > >(
                           gpSimplePool->GetObj(SObjectTag('PART', resources.GetAsset(3))))
                     : rstl::optional_object_null())
, mFreezeSfx(resources.GetAsset(4))
, mElectricSfx(resources.GetAsset(5))
, mSustainedDamagePlayerId(kInvalidUniqueId)
, x6a6_0_(false)
, mEnableEnergyPulse(true)
, mFiring(false)
, mTexturesLoaded(false)
, mDrawOwnerFirst(drawOwnerFirst)
, mInitialDamageEnabled(false)
, mInitialDamagePending(false) {
  mTexture.Lock();
  mGlowTexture.Lock();
  if (mContactGen.get()) {
    const float scale = beamInfo.GetContactFxScale();
    mContactGen->SetGlobalScale(CVector3f(scale, scale, scale));
    mContactGen->SetParticleEmission(false);
  }
  const float pulseScale = beamInfo.GetPulseFxScale();
  mPulseGen->SetGlobalScale(CVector3f(pulseScale, pulseScale, pulseScale));
  mPulseGen->SetParticleEmission(false);
  if (mMuzzleGen.get()) {
    mMuzzleGen->SetGlobalScale(CVector3f(pulseScale, pulseScale, pulseScale));
    mMuzzleGen->SetParticleEmission(false);
  }
}

float CPlasmaProjectile::UpdateBeamState(float dt, CStateManager& mgr) {
  switch (mExpansionState) {
  case kES_Attack:
    if (mExpansionT > 0.5f) {
      mExpansionState = kES_Sustain;
    } else {
      mExpansionT += dt * mExpansionSpeed;
    }
    break;
  case kES_Sustain:
    if (mBeamAttributes & 4) {
      if (mLifeTimer > mLifeTime) {
        mExpansionState = kES_Release;
      } else {
        mLifeTimer += dt;
      }
    }
    break;
  case kES_Release:
    mExpansionT += dt * mExpansionSpeed;
    if (mExpansionT > 1.f) {
      mExpansionT = 1.f;
      mExpansionState = kES_Done;
      mEnableEnergyPulse = false;
    }
    break;
  case kES_Done:
    mShutdownTimer += dt;
    if (mShutdownTimer > mShutdownTime &&
        (!mContactGen.get() || mContactGen->GetParticleCountAll() == 0)) {
      mExpansionState = kES_Inactive;
      ResetBeam(mgr, true);
    }
    break;
  default:
    break;
  }
  return -4.f * mExpansionT * (mExpansionT - 1.f);
}

void CPlasmaProjectile::MakeBillboardEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric,
    const rstl::string& name, CStateManager& mgr, uint playerMask) {
  mgr.AddObject(
      rs_new CHUDBillboardEffect(particle, electric, mgr.AllocateUniqueId(), true, name,
                                 CHUDBillboardEffect::GetNearClipDistance(mgr, playerMask),
                                 CHUDBillboardEffect::GetScaleForPOV(mgr), playerMask,
                                 CColor::White(), CVector3f::One(), CVector3f::Zero(), false));
}

void CPlasmaProjectile::UpdatePlayerEffects(float dt, CStateManager& mgr) {
  mPlayerEffectPulseTimer -= dt;
  if (mExpansionState == kES_Attack || mExpansionState == kES_Sustain) {
    if ((mBeamAttributes & 0x100) && mAppliedDamageToPlayer) {
      mExpansionState = kES_Release;
    }
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetCollisionActorId()));
    if (GetDamageType() == kDT_Actor && player) {
      if (mInitialDamageEnabled && mInitialDamagePending) {
        CDamageInfo damage = GetCurrentDamageInfo();
        damage.SetDamage(mInitialDamage);
        mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetOwnerId(), damage, GetFilter(),
                        CVector3f::Zero());
        mInitialDamagePending = false;
      }
      if (mPlayerEffectPulseTimer <= 0.f) {
        if ((mBeamAttributes & 8) && mSustainedDamagePlayerId == kInvalidUniqueId) {
          mSustainedDamagePlayerId = player->GetUniqueId();
          mPlayerDamageTimer = 0.f;
          player->PushSustainedDamage();
        }
        switch (GetType()) {
        case kWT_Dark:
          player->GetKnockBackManager().Freeze(player->GetTweakPlayer()->GetFrozenTimeout(),
                                               *player);
          break;
        case kWT_Light:
          if (mVisorElectric) {
            MakeBillboardEffect(rstl::optional_object_null(), mVisorElectric,
                                rstl::string_l("PlasmaElectricFx"), mgr,
                                mgr.MaskUIdNumPlayers(player->GetUniqueId()));
            CSfxManager::SfxStart(mElectricSfx, 0x7f, player->GetSoundPan(CPlayer::kMSP_4));
            player->SetHudDisable(3.f);
            player->SetOrbitRequestForTarget(player->GetOrbitTargetId(),
                                             CPlayer::kOR_ActivateOrbitSource, mgr);
            player->GetPlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.2f, 3.f);
          }
          break;
        case kWT_Annihilator:
          if (mVisorParticle) {
            MakeBillboardEffect(mVisorParticle, rstl::optional_object_null(),
                                rstl::string_l("PlasmaVisorFx"), mgr,
                                mgr.MaskUIdNumPlayers(player->GetUniqueId()));
          }
          break;
        default:
          break;
        }
        mPlayerEffectPulseTimer = 0.75f;
      }
    }
  }
  if (mSustainedDamagePlayerId != kInvalidUniqueId) {
    mgr.ApplyDamage(GetUniqueId(), mSustainedDamagePlayerId, GetOwnerId(),
                    CDamageInfo(mPhazonDamage, dt), GetFilter(), CVector3f::Zero());
    mPlayerDamageTimer += dt;
    if (mPlayerDamageTimer >= mPlayerDamageDuration) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(GetOwnerId()))) {
        player->PopSustainedDamage();
      }
      mPlayerDamageTimer = 0.f;
      mSustainedDamagePlayerId = kInvalidUniqueId;
    }
  }
}

void CPlasmaProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mTexturesLoaded = mTexture.IsLoaded() && mGlowTexture.IsLoaded();
  CauseDamage(mExpansionState == kES_Attack || mExpansionState == kES_Sustain);
  CBeamProjectile::UpdateFx(xf, dt, mgr);
  UpdatePlayerEffects(dt, mgr);

  if (mBeamAttributes & 1) {
    rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
    for (int i = 7; i > 0; --i) {
      cache[i] = cache[i - 1];
    }
    cache[0] = GetCurrentPos();
  }
  const bool contact = GetDamageType() != kDT_None && mEnableEnergyPulse;
  if (mContactGen.get()) {
    mContactPulseTimer -= dt;
    if (contact && mContactPulseTimer <= 0.f) {
      mContactGen->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal()));
      mContactGen->SetTranslation(GetCurrentPos() + 0.001f * GetSurfaceNormal());
      mContactGen->SetParticleEmission(true);
      mContactPulseTimer = 1.f / 16.f;
    } else {
      mContactGen->SetParticleEmission(false);
    }
    mContactGen->Update(dt);
  }
  if (mMuzzleGen.get()) {
    mMuzzleGen->SetGlobalOrientation(xf);
    mMuzzleGen->SetGlobalTranslation(xf.GetTranslation());
    mMuzzleGen->SetParticleEmission(true);
    mMuzzleGen->SetGlobalScale(mMuzzleScale);
    mMuzzleGen->Update(dt);
  }
  const float expansion = UpdateBeamState(dt, mgr);
  UpdateEnergyPulse(dt);
  mBeamAngle += 720.f * dt;
  if (mBeamAngle > 360.f) {
    mBeamAngle = 0.f;
  }
  mBeamWidth = expansion * GetMaxRadius();
  mExpansion = expansion;
  mEnergyPulseStartY += dt * mPulseSpeed;
  if (mEnergyPulseStartY > 5.f) {
    mEnergyPulseStartY = 0.f;
  }
  UpdateLights(expansion, dt, mgr);
}

bool CPlasmaProjectile::CanRenderUnsorted(const CStateManager&) const { return false; }

void CPlasmaProjectile::AddToRenderer(const CStateManager& mgr) const {
  if (GetActive()) {
    if (mContactGen.get()) {
      gpRender->AddParticleGen(*mContactGen);
    }
    if (mMuzzleGen.get()) {
      gpRender->AddParticleGen(*mMuzzleGen);
    }
    if (mBeamAttributes & 2) {
      gpRender->AddParticleGen(*mPulseGen);
    }
  }
  EnsureRendered(mgr, GetBeamTransform().GetTranslation(), GetSortingBounds(mgr));
}

void CPlasmaProjectile::Render(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }
  CTransform4f xf = GetBeamTransform();
  if (!(mBeamAttributes & 1)) {
    xf.AddTranslation(mgr.GetCurrentRenderCameraManager()->GetGlobalCameraTranslation(mgr, true));
  }
  gpRender->SetDepthReadWrite(true, false);
  if ((mBeamAttributes & 1) && mEnableEnergyPulse && mExpansionState != kES_Attack) {
    RenderMotionBlur();
  }
  if (!(mBeamAttributes & 0x10)) {
    gpRender->SetModelMatrix(xf);
    RenderBeam(3, 0.25f * mBeamWidth, mCoreColor, 4);
  }
  if (!(mBeamAttributes & 0x20)) {
    gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(mBeamAngle)));
    RenderBeam(4, 0.5f * mBeamWidth, mInnerColor, 1);
  }
  if (!(mBeamAttributes & 0x40)) {
    gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(-mBeamAngle)));
    RenderBeam(8, mBeamWidth, mOuterColor, 3);
  }
  if (!(mBeamAttributes & 0x80)) {
    gpRender->SetModelMatrix(xf);
    RenderBeam(6, 1.25f * mBeamWidth, mOuterColor, 0xd);
  }
}

void CPlasmaProjectile::Fire(const CTransform4f& xf, CStateManager& mgr, bool flag) {
  SetActive(true);
  SetLightsActive(true, mgr);
  mEnableEnergyPulse = true;
  mFiring = true;
  x6a6_0_ = flag;
  mExpansionState = kES_Attack;
  mInitialDamagePending = mInitialDamageEnabled;
  if (mBeamAttributes & 1) {
    rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
    for (int i = 0; i < cache.size(); ++i) {
      cache[i] = xf.GetTranslation();
    }
  }
}

void CPlasmaProjectile::ResetBeam(CStateManager& mgr, bool fullReset) {
  if (fullReset) {
    SetActive(false);
    SetLightsActive(false, mgr);
    mLifeTimer = 0.f;
    mExpansionT = 0.f;
    mBeamAngle = 0.f;
    mShutdownTimer = 0.f;
    mContactPulseTimer = 0.f;
    mEnergyPulseTimer = 0.f;
    mPlayerEffectPulseTimer = 0.f;
    mExpansionState = kES_Inactive;
  } else {
    mExpansionState = kES_Release;
  }
  mFiring = false;
  mPulseGen->SetParticleEmission(false);
  if (mContactGen.get()) {
    mContactGen->SetParticleEmission(false);
  }
  if (mMuzzleGen.get()) {
    mMuzzleGen->SetParticleEmission(false);
  }
}

void CPlasmaProjectile::RenderBeam(int subdivisions, float width, const CColor& color,
                                   int flags) const {
  CTexture* texture = nullptr;
  if (flags & 1) {
    texture = flags & 8 ? mGlowTexture.GetObject() : mTexture.GetObject();
    if (!texture) {
      return;
    }
  }
  bool flip = false;
  const int count = subdivisions + 1;
  const float angleStep = (2.f * M_PIF) / subdivisions;
  const float uvY0 = -(0.0625f * mEnergyPulseStartY);
  const float uvY1 = uvY0 + ((flags & 3) == 3) ? 2.f : 0.5f * GetCurrentLength();
  const CVector3f beamEnd(0.f, GetCurrentLength(), 0.f);
  float angle = 0.f;
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  const GXVtxDescList vtxDesc[] = {{GX_VA_POS, GX_DIRECT},
                                   {GX_VA_CLR0, GX_DIRECT},
                                   {GX_VA_TEX0, GX_DIRECT},
                                   {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  if (flags & 0x10) {
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else if (flags & 4) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  }
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEXCOORD0, GX_IDENTITY, false,
                      GX_PTIDENTITY);
  if (flags & 1) {
    CGX::SetNumTexGens(1);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  } else {
    CGX::SetNumTexGens(0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  }
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  const uint rgba = color.GetColor_u32();
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, count * 2);
  for (int i = 0; i < count; ++i) {
    const float x = CMath::FastCosR(angle);
    const float z = CMath::FastSinR(angle);
    const float uvX = flags & 8 ? 0.5f * z : flip ? width : 0.f;
    flip ^= true;
    const float px = width * x;
    const float pz = width * z;
    const CVector3f position(px, 0.f, pz);
    GXPosition3f32(position.GetX(), position.GetY(), position.GetZ());
    GXColor1u32(rgba);
    GXTexCoord2f32(uvX, uvY0);
    const CVector3f end = position + beamEnd;
    GXPosition3f32(end.GetX(), end.GetY(), end.GetZ());
    GXColor1u32(rgba);
    GXTexCoord2f32(uvX, uvY1);
    angle += angleStep;
  }
  CGX::End();
  if (flags & 8) {
    CGraphics::SetCullMode(kCM_Front);
  }
}

void CPlasmaProjectile::UpdateEnergyPulse(float dt) {
  if (GetDamageType() != kDT_None && mEnableEnergyPulse) {
    mEnergyPulseTimer -= dt;
    if (mEnergyPulseTimer <= 0.f) {
      mEnergyPulseTimer = 2.f * dt;
      mPulseGen->SetParticleEmission(true);
      const float lengthRatio = GetCurrentLength() / GetMaxLength();
      for (float t = 0.f; t <= lengthRatio; t += 0.1f) {
        const float y = t * GetMaxLength() + mEnergyPulseStartY;
        if (y <= GetCurrentLength()) {
          mPulseGen->SetTranslation(CVector3f(0.f, y, 0.f));
          mPulseGen->ForceParticleCreation(1);
        }
      }
      mPulseGen->SetGlobalOrientAndTrans(GetBeamTransform());
      mPulseGen->SetParticleEmission(false);
    }
  }
  mPulseGen->Update(dt);
}

void CPlasmaProjectile::RenderMotionBlur() const {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();
  const CVector3f origin = GetBeamTransform().GetTranslation();
  const uint outerColor = mOuterColor.GetColor_u32();
  const uint color0 = (outerColor & 0xffffff00) | 0x3f;
  const uint color1 = outerColor & 0xffffff00;
  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_CLR0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumTexGens(0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 16);
  const rstl::reserved_vector< CVector3f, 8 >& points = GetPointCache();
  for (int i = 0; i < 8; ++i) {
    const uint color = CColor::Lerp(color0, color1, 0.125f * static_cast< float >(i));
    GXPosition3f32(origin.GetX(), origin.GetY(), origin.GetZ());
    GXColor1u32(color);
    const CVector3f& point = points[i];
    GXPosition3f32(point.GetX(), point.GetY(), point.GetZ());
    GXColor1u32(color);
  }
  CGX::End();
  CGraphics::SetCullMode(kCM_Front);
}

void CPlasmaProjectile::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_XDelete) {
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DeletePlasmaLights(mgr);
    if (mSustainedDamagePlayerId != kInvalidUniqueId) {
      if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(mSustainedDamagePlayerId))) {
        player->PopSustainedDamage();
      }
      mSustainedDamagePlayerId = kInvalidUniqueId;
    }
  } else if (msg.GetMessage() == kSM_XCRT) {
    const TLockedToken< CWeaponDescription > desc = mProjectile.GetWeaponDescription();
    if (desc->mAPSM) {
      mWeaponGen = rs_new CElementGen(*desc->mAPSM);
    }
    if (mWeaponGen.get() && mWeaponGen->SystemHasLight()) {
      CreatePlasmaLights(static_cast< const TToken< CWeaponDescription >& >(desc).GetTag().GetId(),
                         mWeaponGen->GetLight(), mgr);
    } else {
      mWeaponGen = nullptr;
    }
    if (mDrawOwnerFirst) {
      SetNextDrawNode(GetOwnerId());
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
  }
  CGameProjectile::AcceptScriptMsg(mgr, msg);
}

void CPlasmaProjectile::SetLightsActive(bool active, CStateManager& mgr) {
  for (int i = 0; i < mLights.size(); ++i) {
    if (mLights[i] == kInvalidUniqueId) {
      continue;
    }
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLights[i]))) {
      light->SetActive(active);
    }
  }
}

void CPlasmaProjectile::CreatePlasmaLights(uint sourceId, const CLight& light, CStateManager& mgr) {
  DeletePlasmaLights(mgr);
  mLights.reserve(kMaxPlasmaLights);
  for (int i = 0; i < kMaxPlasmaLights; ++i) {
    const TUniqueId id = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CGameLight(id, GetAreaIdForPersistence(), GetActive(), rstl::string(),
                                    GetTransform(), GetUniqueId(), light, sourceId, 0, 0.f));
    mLights.push_back(id);
  }
}

void CPlasmaProjectile::DeletePlasmaLights(CStateManager& mgr) {
  for (rstl::vector< TUniqueId >::const_iterator it = mLights.begin(); it != mLights.end(); ++it) {
    if (*it != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(*it);
    }
  }
  mLights = rstl::vector< TUniqueId >();
}

void CPlasmaProjectile::UpdateLights(float expansion, float dt, CStateManager& mgr) {
  if (mWeaponGen.get() && mWeaponGen->SystemHasLight()) {
    mWeaponGen->Update(dt);
    CLight light = mWeaponGen->GetLight();
    light.SetColor(CColor(CColor::Lerp(0, light.GetColor().GetColor_u32(), expansion)));
    const float spacing = kInvMaxPlasmaLights * GetCurrentLength();
    float y = 0.f;
    for (rstl::vector< TUniqueId >::const_iterator it = mLights.begin(); it != mLights.end();
         ++it) {
      if (CGameLight* gameLight = TCastToPtr< CGameLight >(mgr.ObjectById(*it))) {
        gameLight->SetTransform(CTransform4f::Identity());
        gameLight->SetTranslation(GetBeamTransform() * CVector3f(0.f, y, 0.f));
        gameLight->SetLight(light);
      }
      y += spacing;
    }
  }
}

void CPlasmaProjectile::SetInitialDamage(float damage) {
  mInitialDamage = damage;
  mInitialDamageEnabled = damage > 0.f;
}
