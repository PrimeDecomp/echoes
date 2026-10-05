#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/math.hpp"

extern const bool gkWaterFog;

const float CScriptWater::kSplashScales[6] = {1.f, 3.f, 0.71f, 1.19f, 0.71f, 1.f};

CScriptWater::CScriptWater(
    CStateManager& mgr, TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    const CVector3f& position, const CAABox& bounds, const CDamageInfo& damage,
    const CVector3f& forceField, uint triggerFlags, float alphaInTime, float alphaOutTime,
    float morphInTime, float morphOutTime, int fluidType, CAssetId lightMap, CAssetId colorMap,
    const CColor& baseColor, CAssetId colorWarpMap, CAssetId glossMap, CAssetId envMap,
    float envMapSize, CAssetId texture, CAssetId foamMap, CAssetId alphaMap, float alpha,
    float glossFlat, float unknown1, float unknown2, float unknown3, const CFluidUVMotion& uvMotion,
    const CColor& splashColor, const CColor& insideFogColor, CAssetId splashParticle1,
    CAssetId splashParticle2, CAssetId splashParticle3, CAssetId visorRunoffParticle,
    CAssetId unmorphVisorRunoffParticle, TSfxId visorRunoffSfx, TSfxId unmorphVisorRunoffSfx,
    TSfxId splashSfx1, TSfxId splashSfx2, TSfxId splashSfx3, const CColor& fogColor, float fogBias,
    float fogMagnitude, float fogSpeed, float viscosity, bool displaySurface, float unknownScale,
    const CVector2f& uvScale, const CVector2f& uvOffset, const CVector2f& surfaceScale,
    bool useDynamicLights, float unknown4, float unknown5, float unknown6, float unknown7,
    bool occlusion, bool filterSoundEffects, int unknown8)
: CScriptTrigger(uid, name, info, position, bounds, damage, forceField,
                 triggerFlags | kTFL_BlockEnvironmentalEffects, false, false)
, mFluidPlane(nullptr)
, mPositionMorphed(position)
, mExtentMorphed(bounds.GetWidth(), bounds.GetHeight(), bounds.GetDepth())
, mMorphInTime(morphInTime)
, mPositionOrig(position)
, mExtentOrig(mExtentMorphed)
, mDamageOrig(damage.GetDamage())
, mDamageMorphed(damage.GetDamage())
, mMorphOutTime(morphOutTime)
, mMorphFactor(0.f)
, mSurfaceBounds(CAABox::MakeMaxInvertedBox())
, mFogBias(fogBias)
, mFogMagnitude(fogMagnitude)
, mOrigFogBias(fogBias)
, mOrigFogMagnitude(fogMagnitude)
, mFogSpeed(fogSpeed)
, mFogColor(fogColor)
, mSplashParticle1Id(splashParticle1)
, mSplashParticle2Id(splashParticle2)
, mSplashParticle3Id(splashParticle3)
, mVisorRunoffParticleId(visorRunoffParticle)
, mUnmorphVisorRunoffParticleId(unmorphVisorRunoffParticle)
, mVisorRunoffSfx(visorRunoffSfx)
, mUnmorphVisorRunoffSfx(unmorphVisorRunoffSfx)
, mSplashColor(splashColor)
, mInsideFogColor(insideFogColor)
, mAlphaInTime(alphaInTime)
, mAlphaOutTime(alphaOutTime)
, mAlphaInRecip(alphaInTime ? 1.f / alphaInTime : 0.f)
, mAlphaOutRecip(alphaOutTime ? 1.f / alphaOutTime : 0.f)
, mAlpha(alpha)
, mGridDimX(
      static_cast< int >(CMath::FloorF((3.f + GetTriggerBoundsWR().GetWidth() - 0.01f) / 3.f)))
, mGridDimY(
      static_cast< int >(CMath::FloorF((3.f + GetTriggerBoundsWR().GetHeight() - 0.01f) / 3.f)))
, mGridCellCount((mGridDimX + 1) * (mGridDimY + 1))
, mPatchDimX(0)
, mPatchDimY(0)
, mTileIntersects(nullptr)
, mVertIntersects(nullptr)
, mPatchIntersects(nullptr)
, mComputedGridCellCount(0)
, x310_(unknown4)
, x314_(unknown5)
, x318_(unknown6)
, x31c_(unknown7)
, mSurfaceScale(surfaceScale * 3.f)
, x328_(unknown8)
, mMorphIn(false)
, mMorphing(false)
, mAllowRender(displaySurface)
, mRecomputeClipping(true)
, mAlphaIn(false)
, mAlphaOut(false)
, x32c_6_(occlusion)
, x32c_7_(filterSoundEffects) {
  mFluidPlane = rs_new CFluidPlaneCPU(
      GetFluidUVExtent(bounds), colorMap, baseColor, colorWarpMap, glossMap, lightMap, envMap,
      texture, useDynamicLights, fluidType, uvMotion, uvScale, uvOffset, unknownScale, alpha,
      glossFlat, unknown1, unknown2, unknown3, envMapSize, viscosity);

  for (int i = 0; i < 3; ++i) {
    mSplashEffects.push_back(rstl::optional_object< TLockedToken< CGenDescription > >());
  }
  if (mSplashParticle1Id != kInvalidAssetId) {
    mSplashEffects[0] = TLockedToken< CGenDescription >(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle1Id))));
  }
  if (mSplashParticle2Id != kInvalidAssetId) {
    mSplashEffects[1] = TLockedToken< CGenDescription >(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle2Id))));
  }
  if (mSplashParticle3Id != kInvalidAssetId) {
    mSplashEffects[2] = TLockedToken< CGenDescription >(
        TToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', mSplashParticle3Id))));
  }
  if (mVisorRunoffParticleId != kInvalidAssetId) {
    mVisorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mVisorRunoffParticleId))));
  }
  if (mUnmorphVisorRunoffParticleId != kInvalidAssetId) {
    mUnmorphVisorRunoffEffect = TLockedToken< CGenDescription >(TToken< CGenDescription >(
        gpSimplePool->GetObj(SObjectTag('PART', mUnmorphVisorRunoffParticleId))));
  }
  mSplashSounds.push_back(splashSfx1);
  mSplashSounds.push_back(splashSfx2);
  mSplashSounds.push_back(splashSfx3);

  SetCalculateLighting(true);
  if (lightMap != kInvalidAssetId) {
    ActorLights()->SetMaxAreaLights(0);
    ActorLights()->SetInArea(false);
  }
  ActorLights()->SetMaxDynamicLights(4);
  ActorLights()->SetCastShadows(false);
  ActorLights()->SetAmbienceGenerated(false);
  ActorLights()->SetFindNearestDynamicLights(true);
  ActorLights()->SetExcludeSpecialDynamicLights(true);
  CalculateRenderBounds();
  if (!GetActive()) {
    mAlpha = 0.f;
    mFogBias = 0.f;
    mFogMagnitude = 0.f;
  }
  SetupGrid(true);
}

CScriptWater::~CScriptWater() {}

void CScriptWater::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Next:
    if (GetActive()) {
      mMorphIn = !mMorphIn;
      if (mMorphIn) {
        for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
             conn != GetConnectionList().end(); ++conn) {
          if (conn->state != kSS_Play || conn->msg != kSM_Activate) {
            continue;
          }
          CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->objId);
          if (ids.first == ids.second) {
            continue;
          }
          const CScriptTrigger* trigger =
              TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(ids.first->second));
          if (trigger) {
            const CAABox& morphBounds = trigger->GetTriggerBounds();
            mPositionMorphed = trigger->GetTranslation();
            mExtentMorphed =
                CVector3f(morphBounds.GetWidth(), morphBounds.GetHeight(), morphBounds.GetDepth());
            mDamageMorphed = trigger->GetDamageInfo().GetDamage();
            const CAABox& originalBounds = GetTriggerBounds();
            mPositionOrig = GetTranslation();
            mExtentOrig = CVector3f(originalBounds.GetWidth(), originalBounds.GetHeight(),
                                    originalBounds.GetDepth());
            mDamageOrig = mDamageInfo.GetDamage();
            break;
          }
        }
      }
      SetMorphing(true);
    }
    break;
  case kSM_Activate:
    mAlphaOut = false;
    if (close_enough(mAlphaInTime, 0.f)) {
      mAlpha = mFluidPlane->GetAlpha();
      mFogBias = mOrigFogBias;
      mFogMagnitude = mOrigFogMagnitude;
    } else {
      mAlphaIn = true;
    }
    break;
  case kSM_Action:
    mAlphaIn = false;
    if (close_enough(mAlphaOutTime, 0.f)) {
      mAlpha = 0.f;
      mFogBias = 0.f;
      mFogMagnitude = 0.f;
    } else {
      mAlphaOut = true;
    }
    break;
  case kSM_XDelete:
    ClearSplashInhabitants();
    break;
  case kSM_XALD: {
    const TUniqueId id = FindConnectedObject(mgr, kSS_Connect, kSM_Reset);
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(id))) {
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player, kMT_Debris), GetMaterialFilter().GetExcludeList()));
      const CVector3f& min = mBounds.GetMinPoint();
      const CVector3f& max = mBounds.GetMaxPoint();
      actor->SetTranslation(
          CVector3f(GetTranslation().GetX(), GetTranslation().GetY(),
                    GetTranslation().GetZ() + (0.5f * (max.GetZ() - min.GetZ()) - 1.f)));
      const CAABox box(CVector3f(min.GetX(), min.GetY(), -0.5f),
                       CVector3f(max.GetX(), max.GetY(), 0.5f));
      actor->SetCollisionPrimitive(
          CCollidableAABox(box, actor->GetMaterialFilter().GetIncludeList()));
      actor->SetBoundingBox(box);
    }
    break;
  }
  default:
    break;
  }
  CScriptTrigger::AcceptScriptMsg(mgr, msg);
}

const CScriptWater* CScriptWater::GetNextConnectedWater(const CStateManager& mgr) const {
  for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
       conn != GetConnectionList().end(); ++conn) {
    if (conn->state != kSS_Play || conn->msg != kSM_Activate) {
      continue;
    }
    CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->objId);
    if (ids.first != ids.second) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(ids.first->second))) {
        return water;
      }
    }
  }
  return nullptr;
}

void CScriptWater::Touch(CActor& actor, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const int fluidCount = actor.GetFluidCount();
  CScriptTrigger::Touch(actor, mgr);
  if (actor.GetMaterialList().HasMaterial(kMT_Trigger)) {
    return;
  }
  for (rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = mWaterInhabitants.begin();
       it != mWaterInhabitants.end(); ++it) {
    if (it->first == actor.GetUniqueId()) {
      it->second = true;
      return;
    }
  }
  const rstl::optional_object< CAABox > bounds = actor.GetTouchBounds();
  if (!bounds) {
    return;
  }
  mWaterInhabitants.push_back(rstl::pair< TUniqueId, bool >(actor.GetUniqueId(), true));
  if (fluidCount == 0) {
    const float surfaceZ = GetTriggerBoundsWR().GetMaxPoint().GetZ();
    if (bounds->GetMinPoint().GetZ() <= surfaceZ && surfaceZ <= bounds->GetMaxPoint().GetZ()) {
      actor.FluidFXThink(kFS_EnteredFluid, *this, mgr);
    }
  }
}

void CScriptWater::UpdateSplashInhabitants(CStateManager& mgr) {
  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = mWaterInhabitants.begin();
  while (it != mWaterInhabitants.end()) {
    rstl::list< rstl::pair< TUniqueId, bool > >::iterator next = it;
    ++next;
    CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->first));
    bool intersects = false;
    if (actor) {
      const rstl::optional_object< CAABox > bounds = actor->GetTouchBounds();
      if (bounds) {
        const float surfaceZ = GetTriggerBoundsWR().GetMaxPoint().GetZ();
        intersects =
            bounds->GetMinPoint().GetZ() <= surfaceZ && surfaceZ <= bounds->GetMaxPoint().GetZ();
      }
    }
    if (actor && it->second) {
      if (intersects) {
        actor->FluidFXThink(kFS_InFluid, *this, mgr);
      }
      it->second = false;
    } else {
      mWaterInhabitants.erase(it);
      if (actor && actor->GetFluidCount() == 0 && intersects) {
        actor->FluidFXThink(kFS_LeftFluid, *this, mgr);
      }
    }
    it = next;
  }
}

void CScriptWater::ClearSplashInhabitants() {
  rstl::list< rstl::pair< TUniqueId, bool > >::iterator it = mWaterInhabitants.begin();
  while (it != mWaterInhabitants.end()) {
    rstl::list< rstl::pair< TUniqueId, bool > >::iterator next = it;
    ++next;
    mWaterInhabitants.erase(it);
    it = next;
  }
}

void CScriptWater::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CScriptTrigger::Think(dt, mgr);
  UpdateSplashInhabitants(mgr);
  if (mAlphaOut) {
    mAlpha -= dt * mFluidPlane->GetAlpha() * mAlphaOutRecip;
    mFogBias -= dt * mOrigFogBias * mAlphaOutRecip;
    mFogMagnitude -= dt * mOrigFogMagnitude * mAlphaOutRecip;
    if (mAlpha <= 0.f) {
      mAlpha = 0.f;
      mFogBias = 0.f;
      mFogMagnitude = 0.f;
      mAlphaOut = false;
    }
  } else if (mAlphaIn) {
    mAlpha += dt * mFluidPlane->GetAlpha() * mAlphaInRecip;
    mFogBias += dt * mOrigFogBias * mAlphaInRecip;
    mFogMagnitude += dt * mOrigFogMagnitude * mAlphaInRecip;
    if (mAlpha > mFluidPlane->GetAlpha()) {
      mAlpha = mFluidPlane->GetAlpha();
      mFogBias = mOrigFogBias;
      mFogMagnitude = mOrigFogMagnitude;
      mAlphaIn = false;
    }
  }
  if (mMorphing) {
    bool stillMorphing = true;
    if (mMorphIn) {
      mMorphFactor += dt / mMorphInTime;
      if (mMorphFactor > 1.f) {
        mMorphFactor = 1.f;
        stillMorphing = false;
      }
    } else {
      mMorphFactor -= dt / mMorphOutTime;
      if (mMorphFactor < 0.f) {
        mMorphFactor = 0.f;
        stillMorphing = false;
      }
    }
    SetTranslation(CVector3f::Lerp(mPositionOrig, mPositionMorphed, mMorphFactor));
    mDamageInfo.SetDamage((1.f - mMorphFactor) * mDamageOrig + mMorphFactor * mDamageMorphed);
    const CVector3f extent = CVector3f::Lerp(mExtentOrig, mExtentMorphed, mMorphFactor);
    mBounds = CAABox(-0.5f * extent, 0.5f * extent);
    CalculateRenderBounds();
    if (stillMorphing) {
      SetupGrid(false);
    } else {
      SetMorphing(false);
    }
  }
  SetupGridClipping(mgr, 4);
}

void CScriptWater::CalculateRenderBounds() {
  const CVector3f& min = mBounds.GetMinPoint();
  const CVector3f& max = mBounds.GetMaxPoint();
  mSurfaceBounds = CAABox(CVector3f(min.GetX(), min.GetY(), max.GetZ() - 1.f) + GetTranslation(),
                          CVector3f(max.GetX(), max.GetY(), max.GetZ() + 1.f) + GetTranslation());
}

void CScriptWater::PreRenderAllViewports(CStateManager& mgr) {
  const float fogHeight = rstl::max_val(0.01f, mFogBias + mFogMagnitude);
  const CVector3f& localMin = mBounds.GetMinPoint();
  const CVector3f& localMax = mBounds.GetMaxPoint();
  const CAABox visibleBounds(
      CVector3f(localMin.GetX(), localMin.GetY(), localMax.GetZ()) + GetTranslation(),
      CVector3f(localMax.GetX(), localMax.GetY(), localMax.GetZ() + fogHeight) + GetTranslation());
  SetOtherBounds(visibleBounds);
  SetRenderBounds(visibleBounds);
  UpdatePortalSystemState(mgr);

  const CTransform4f cameraXf =
      mgr.GetCurrentRenderCameraManager()->GetCurrentCameraTransform(mgr, true);
  const CAABox triggerBounds = GetTriggerBoundsWR();
  const float height = cameraXf.Get23() - triggerBounds.GetMaxPoint().GetZ();
  if (fabsf(height) > 0.5f) {
    if (height <= 0.f) {
      if (x32c_6_ || mFluidPlane->GetFluidType() == 2) {
        const CVector3f localCamera = cameraXf.GetTranslation() - GetTranslation();
        if (mBounds.PointInside(localCamera)) {
          mgr.SetAreaClipPlane(GetCurrentAreaId(),
                               CPlane(triggerBounds.GetMaxPoint().GetZ() + 0.01f, CVector3f::Up()));
        }
      }
    } else if (x32c_6_) {
      mgr.SetAreaClipPlane(GetCurrentAreaId(),
                           CPlane(-triggerBounds.GetMaxPoint().GetZ(), CVector3f::Down()));
    }
  }
}

CAABox CScriptWater::GetSortingBounds(const CStateManager&) const {
  // The original's maxZ < maxZ - 1 comparison leaves the surface bounds unchanged.
  return mSurfaceBounds;
}

void CScriptWater::AddToRenderer(const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }
  if (mFluidPlane->GetFluidType() == 2) {
    Render(mgr);
  } else {
    const CPlane plane(mBounds.GetMaxPoint().GetZ() + GetTranslation().GetZ(),
                       CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes));
    mgr.AddDrawableActorPlane(*this, plane, GetSortingBounds(mgr));
  }
}

void CScriptWater::PreRender(CStateManager& mgr) {
  if (!mAllowRender) {
    SetPreRenderClipped(true);
    return;
  }
  SetPreRenderClipped(!mgr.fn_800366e4(this));
  if (GetPreRenderClipped() || GetCurrentAreaId() == kInvalidAreaId) {
    return;
  }
  if (ActorLights()->GetMaxAreaLights() != 0 &&
      (GetPreRenderHasMoved() || ActorLights()->GetNeedsRelight())) {
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
    if (area.IsLoaded()) {
      ActorLights()->BuildAreaLightList(mgr, area, GetTriggerBoundsWR());
      SetPreRenderHasMoved(false);
    }
  }
  ActorLights()->BuildDynamicLightList(mgr, GetTriggerBoundsWR());
  mFluidPlane->PreRender(mgr, GetFluidUVExtent(mSurfaceBounds));
}

void CScriptWater::Render(const CStateManager& mgr) const {
  if (GetActive() && !GetPreRenderClipped()) {
    GetActorLights()->ActivateLights();
    const CAABox& bounds = mSurfaceBounds;
    const float zOffset = 0.5f * (bounds.GetMaxPoint().GetZ() + bounds.GetMinPoint().GetZ()) -
                          GetTranslation().GetZ();
    const CAABox localBounds = bounds.GetTransformedAABox(CTransform4f::Translate(
        -GetTranslation().GetX(), -GetTranslation().GetY(), -GetTranslation().GetZ() - zOffset));
    CTransform4f xf = GetTransform();
    xf.AddTranslationZ(zOffset);
    const CVector2f uvExtent = GetFluidUVExtent(localBounds);
    const CAABox renderBounds(CVector3f::Zero(), CVector3f(uvExtent.GetX(), uvExtent.GetY(), 1.f));
    xf.AddTranslation(
        CVector3f(localBounds.GetMinPoint().GetX(), localBounds.GetMinPoint().GetY(), 0.f));
    const CTransform4f scale = CTransform4f::Scale(mSurfaceScale.GetX(), mSurfaceScale.GetY(), 1.f);
    xf = xf * scale;
    const CTransform4f areaXf = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetTM() * scale;
    mFluidPlane->Render(mgr, mAlpha, renderBounds, xf, areaXf, GetUniqueId(), mTileIntersects.get(),
                        mGridDimX, mGridDimY);
    if (mFogBias != 0.f && mgr.GetPlayerState()->CanVisorSeeFog(mgr) && gkWaterFog) {
      const float wave = CMath::FastSinR(mFogSpeed * CGraphics::GetSecondsMod900());
      const float fogLevel = mgr.IntegrateVisorFog(mFogMagnitude * wave + mFogBias);
      if (fogLevel > 0.f) {
        const CAABox fogBounds = GetTriggerBoundsWR();
        const CVector3f& min = fogBounds.GetMinPoint();
        const CVector3f& max = fogBounds.GetMaxPoint();
        const CAABox fogBox(CVector3f(min.GetX(), min.GetY(), max.GetZ()),
                            CVector3f(max.GetX(), max.GetY(), max.GetZ() + fogLevel));
        const CTransform4f modelXf =
            CTransform4f::Translate(fogBox.GetCenterPoint()) *
            CTransform4f::Scale((fogBox.GetMaxPoint() - fogBox.GetMinPoint()) * 0.5f);
        const CAABox unitBox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f));
        gpRender->SetModelMatrix(modelXf);
        gpRender->SetAmbientColor(CColor::White());
        gpRender->RenderFogVolume(mFogColor, unitBox, nullptr, nullptr);
      }
    }
    CGraphics::DisableAllLights();
  }
  CActor::Render(mgr);
}

CVector2f CScriptWater::GetFluidUVExtent(const CAABox& bounds) const {
  return CVector2f(bounds.GetWidth() / mSurfaceScale.GetX(),
                   bounds.GetHeight() / mSurfaceScale.GetY());
}

int CScriptWater::GetSplashIndex(float scale) const {
  int index = static_cast< int >(scale * 3.f);
  if (index > 2) {
    --index;
  }
  return index;
}

const rstl::optional_object< TLockedToken< CGenDescription > >&
CScriptWater::GetSplashEffect(float scale) const {
  return mSplashEffects[GetSplashIndex(scale)];
}

TSfxId CScriptWater::GetSplashSound(float scale) const {
  return mSplashSounds[GetSplashIndex(scale)];
}

float CScriptWater::GetSplashEffectScale(float scale) const {
  if (close_enough(scale, 1.f)) {
    return kSplashScales[5];
  }
  const int index = GetSplashIndex(scale);
  scale *= 3.f;
  scale -= CMath::FloorF(scale);
  return (1.f - scale) * kSplashScales[index * 2] + scale * kSplashScales[index * 2 + 1];
}

EWeaponCollisionResponseTypes CScriptWater::GetCollisionResponseType(const CVector3f&,
                                                                     const CVector3f&,
                                                                     const CWeaponMode&,
                                                                     int) const {
  return kWCR_Water;
}

void CScriptWater::SetMorphing(bool morphing) {
  if (morphing != mMorphing) {
    mMorphing = morphing;
    SetupGrid(!morphing);
  }
}

void CScriptWater::SetupGridClipping(CStateManager& mgr, int computeVerts) {
  if (mRecomputeClipping) {
    mComputedGridCellCount = 0;
    mVertIntersects = static_cast< bool* >(nullptr);
    mRecomputeClipping = false;
  }
  if (mComputedGridCellCount >= mGridCellCount) {
    return;
  }
  static CMaterialFilter solidFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Unknown59));
  if (mVertIntersects.null()) {
    mVertIntersects = rs_new bool[(mGridDimX + 1) * (mGridDimY + 1)];
  }
  const CVector3f down(0.f, 0.f, -1.f);
  const CAABox surfaceBounds = GetTriggerBoundsWR();
  const float baseZ = surfaceBounds.GetMaxPoint().GetZ() + 0.8f;
  const CAABox bounds = GetTriggerBoundsWR();
  const int stride = mGridDimX + 1;
  int row = mComputedGridCellCount / stride;
  int column = mComputedGridCellCount % stride;
  bool* vertex = mVertIntersects.get() + mComputedGridCellCount;
  const float height = mBounds.GetMaxPoint().GetZ() - mBounds.GetMinPoint().GetZ();
  const float baseX = bounds.GetMinPoint().GetX();
  float yOffset = 3.f * float(row);
  const float baseY = bounds.GetMinPoint().GetY();
  float xOffset = 3.f * float(column);
  const float length = rstl::min_val(2.f * height + 0.8f, 120.f);
  const int end = rstl::min_val(mGridCellCount, mComputedGridCellCount + computeVerts);
  for (int i = mComputedGridCellCount; i < end; ++i, ++vertex) {
    const CVector3f position(baseX + xOffset, baseY + yOffset, baseZ);
    const CRayCastResult hit = mgr.RayStaticIntersection(position, down, length, solidFilter);
    *vertex = hit.IsValid();
    ++column;
    xOffset += 3.f;
    if (column > mGridDimX) {
      yOffset += 3.f;
      xOffset = 0.f;
      column = 0;
    }
  }
  mComputedGridCellCount += computeVerts;
  if (mComputedGridCellCount < mGridCellCount) {
    return;
  }
  mComputedGridCellCount = mGridCellCount;
  mTileIntersects = rs_new char[mGridDimX * mGridDimY];
  for (int y = 0; y < mGridDimY; ++y) {
    char* tileRow = mTileIntersects.get() + y * mGridDimX;
    const bool* vertexRow = mVertIntersects.get() + y * stride;
    for (int x = 0; x < mGridDimX; ++x) {
      tileRow[x] =
          vertexRow[x] || vertexRow[x + 1] || vertexRow[x + stride] || vertexRow[x + stride + 1];
    }
  }
  const int tilesPerPatch = 7;
  mPatchDimX = (mGridDimX + tilesPerPatch - 1) / tilesPerPatch;
  mPatchDimY = (mGridDimY + tilesPerPatch - 1) / tilesPerPatch;
  mPatchIntersects = rs_new char[mPatchDimX * mPatchDimY];
  for (int py = 0; py < mPatchDimY; ++py) {
    for (int px = 0; px < mPatchDimX; ++px) {
      bool allClear = true;
      bool allIntersect = true;
      for (int y = py * tilesPerPatch; y < rstl::min_val(mGridDimY, (py + 1) * tilesPerPatch);
           ++y) {
        if (!allClear && !allIntersect) {
          break;
        }
        for (int x = px * tilesPerPatch; x < rstl::min_val(mGridDimX, (px + 1) * tilesPerPatch);
             ++x) {
          if (mTileIntersects.get()[x + y * mGridDimX]) {
            allClear = false;
            if (!allIntersect) {
              break;
            }
          } else {
            allIntersect = false;
            if (!allClear) {
              break;
            }
          }
        }
      }
      mPatchIntersects.get()[px + py * mPatchDimX] = allIntersect ? 1 : allClear ? 0 : 2;
    }
  }
  mVertIntersects = static_cast< bool* >(nullptr);
}

void CScriptWater::SetupGrid(bool recomputeClipping) {
  const CAABox xBounds = GetTriggerBoundsWR();
  const int dimX = static_cast< int >(CMath::FloorF((3.f + xBounds.GetWidth() - 0.01f) / 3.f));
  const CAABox yBounds = GetTriggerBoundsWR();
  const int dimY = static_cast< int >(CMath::FloorF((3.f + yBounds.GetHeight() - 0.01f) / 3.f));
  mGridCellCount = (dimX + 1) * (dimY + 1);
  mComputedGridCellCount = mGridCellCount;
  mVertIntersects = static_cast< bool* >(nullptr);
  if (mTileIntersects.null() || dimX != mGridDimX || dimY != mGridDimY) {
    mTileIntersects = rs_new char[dimX * dimY];
  }
  mGridDimX = dimX;
  mGridDimY = dimY;
  for (int y = 0; y < mGridDimY; ++y) {
    char* row = mTileIntersects.get() + y * mGridDimX;
    for (int x = 0; x < mGridDimX; ++x) {
      row[x] = 1;
    }
  }
  if (mPatchIntersects.null() || mPatchDimX != 0 || mPatchDimY != 0) {
    mPatchIntersects = rs_new char[32];
  }
  for (int i = 0; i < 32; ++i) {
    mPatchIntersects.get()[i] = 1;
  }
  mPatchDimX = 0;
  mPatchDimY = 0;
  mRecomputeClipping = recomputeClipping;
}

bool CScriptWater::CanRippleAtPoint(const CVector3f& point) const {
  if (mTileIntersects.null()) {
    return true;
  }
  const CAABox bounds = GetTriggerBoundsWR();
  const int x = static_cast< int >((point.GetX() - bounds.GetMinPoint().GetX()) / 3.f);
  if (x < 0 || x >= mGridDimX) {
    return false;
  }
  const int y = static_cast< int >((point.GetY() - bounds.GetMinPoint().GetY()) / 3.f);
  if (y < 0 || y >= mGridDimY) {
    return false;
  }
  return mTileIntersects.get()[x + y * mGridDimX] != 0;
}

void CScriptWater::InhabitantAdded(CActor& actor, CStateManager& mgr) {
  CScriptTrigger::InhabitantAdded(actor, mgr);
  const int previousFluidCount = actor.GetFluidCount();
  actor.SetInFluid(mgr, true, GetUniqueId());
  if (previousFluidCount == 0 && ShouldSendScriptMsgs(actor, mgr)) {
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XENF, kInvalidUniqueId);
    if (CGameCamera* camera = TCastToPtr< CGameCamera >(actor)) {
      camera->UnkVtable84();
    }
  }
}

void CScriptWater::InhabitantExited(CActor& actor, CStateManager& mgr) {
  CScriptTrigger::InhabitantExited(actor, mgr);
  actor.SetInFluid(mgr, false, GetUniqueId());
  if (actor.GetFluidCount() == 0 && ShouldSendScriptMsgs(actor, mgr)) {
    mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XEXF, kInvalidUniqueId);
    if (CGameCamera* camera = TCastToPtr< CGameCamera >(actor)) {
      camera->UnkVtable88(GetUniqueId());
    }
  }
}

void CScriptWater::InhabitantIdle(CActor& actor, CStateManager& mgr) {
  CScriptTrigger::InhabitantIdle(actor, mgr);
  mgr.SendScriptMsg(&actor, GetUniqueId(), kSM_XINF, kInvalidUniqueId);
}

CFluidUVMotion::SFluidLayerMotion LdrToFluidLayerMotion(const SLdrLayerInfo& data) {
  return CFluidUVMotion::SFluidLayerMotion(
      static_cast< CFluidUVMotion::EFluidMotion >(data.motionType), data.timeToCycleTex,
      M_2PIF * data.rotation / 360.f - M_2PIF, data.amplitude, data.textureScale);
}

CEntity* LoadWater(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrWater sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrWater.inc"

  const CVector3f halfExtent = 0.5f * sldrThis.editorProperties.transform.scale;
  const CAABox bounds(-halfExtent, halfExtent);
  const CFluidUVMotion uvMotion(
      sldrThis.flowSpeed, M_2PIF * sldrThis.flowOrientation / 360.f - M_2PIF,
      LdrToFluidLayerMotion(sldrThis.flowColor), LdrToFluidLayerMotion(sldrThis.flowColorWarp),
      LdrToFluidLayerMotion(sldrThis.flowGloss1), LdrToFluidLayerMotion(sldrThis.flowGloss2),
      LdrToFluidLayerMotion(sldrThis.flowRefractWarp));
  const CFluidUVMotion uvMotionCopy(uvMotion);
  return rs_new CScriptWater(
      mgr, mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      sldrThis.editorProperties.transform.position, bounds,
      LdrToDamageInfo(sldrThis.trigger.damage), sldrThis.trigger.forceField,
      sldrThis.trigger.flagsTrigger, sldrThis.alphaFadeinTime, sldrThis.alphaFadeoutTime,
      sldrThis.morphTimeTo, sldrThis.morphTimeRestore, sldrThis.fluidType, sldrThis.lightMap,
      sldrThis.colorMap, sldrThis.baseColor, sldrThis.colorWarpMap, sldrThis.glossMap,
      sldrThis.envMap, sldrThis.envMapSize, sldrThis.refractWarpMap, sldrThis.foamMap,
      sldrThis.alphaMap, sldrThis.alpha, sldrThis.glossFlat, sldrThis.glossTopDown,
      sldrThis.refractWarpFlat, sldrThis.refractWarpTopDown, uvMotionCopy, sldrThis.splashColor,
      sldrThis.underwaterFogColor, sldrThis.splash_Small, sldrThis.splash_Medium,
      sldrThis.splash_Big, sldrThis.visorRunoff, sldrThis.visorRunoffBall,
      static_cast< TSfxId >(sldrThis.sound_SoundRunoff),
      static_cast< TSfxId >(sldrThis.sound_SoundRunoffBall),
      static_cast< TSfxId >(sldrThis.sound_Splash_Small),
      static_cast< TSfxId >(sldrThis.sound_Splash_Medium),
      static_cast< TSfxId >(sldrThis.sound_Splash_Big), sldrThis.fogColor, sldrThis.fogHeight,
      sldrThis.fogBobHeight, sldrThis.fogBobFreq, sldrThis.viscosity, sldrThis.renderSurface,
      sldrThis.lightMapResolution, CVector2f(sldrThis.lightMapScaleX, sldrThis.lightMapScaleY),
      CVector2f(sldrThis.lightMapOffsetX, sldrThis.lightMapOffsetY),
      CVector2f(sldrThis.renderTileScaleX, sldrThis.renderTileScaleY), sldrThis.useDynamicLights,
      sldrThis.fogNoGravSuitDist, sldrThis.fogNoGravSuitFactor, sldrThis.fogGravSuitDist,
      sldrThis.fogGravSuitFactor, sldrThis.unknown_0xc71c0d63, sldrThis.filterSoundEffects,
      sldrThis.unknown_0x414379ea);
}
