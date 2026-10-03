#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrWater.hpp"

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
  mSplashSounds.push_back(splashSfx1);
  mSplashSounds.push_back(splashSfx2);
  mSplashSounds.push_back(splashSfx3);

  // TODO: acquire the splash/runoff particle resources and configure actor lighting.
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
  // TODO: recover morph targets, alpha transitions and the connected collision actor.
  CScriptTrigger::AcceptScriptMsg(mgr, msg);
}

const CScriptWater* CScriptWater::GetNextConnectedWater(const CStateManager& mgr) const {
  // TODO: find the first water actor reached by a Play/Activate connection.
  return nullptr;
}

void CScriptWater::Touch(CActor& actor, CStateManager& mgr) {
  // TODO: track surface intersections and send the appropriate fluid entry callbacks.
}

void CScriptWater::UpdateSplashInhabitants(CStateManager& mgr) {
  // TODO: update touched entries, surface crossings and fluid exit callbacks.
}

void CScriptWater::ClearSplashInhabitants() { mWaterInhabitants.clear(); }

void CScriptWater::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CScriptTrigger::Think(dt, mgr);
    UpdateSplashInhabitants(mgr);
    // TODO: advance alpha/fog fading and morph position, bounds and damage.
    SetupGridClipping(mgr, 4);
  }
}

void CScriptWater::CalculateRenderBounds() {
  const CVector3f& min = mBounds.GetMinPoint();
  const CVector3f& max = mBounds.GetMaxPoint();
  mSurfaceBounds = CAABox(CVector3f(min.GetX(), min.GetY(), max.GetZ() - 1.f) + GetTranslation(),
                          CVector3f(max.GetX(), max.GetY(), max.GetZ() + 1.f) + GetTranslation());
}

void CScriptWater::PreRenderAllViewports(CStateManager& mgr) {
  // TODO: update fog-volume render bounds and per-viewport occlusion planes.
}

CAABox CScriptWater::GetSortingBounds(const CStateManager&) const {
  // TODO: recover the intent of the original's redundant surface-height adjustment.
  return mSurfaceBounds;
}

void CScriptWater::AddToRenderer(const CStateManager& mgr) const {
  // TODO: submit the surface plane and sorting bounds, or render the opaque fluid directly.
}

void CScriptWater::PreRender(CStateManager& mgr) {
  // TODO: establish visibility, update lights and prepare the fluid plane's UV extent.
}

void CScriptWater::Render(const CStateManager& mgr) const {
  // TODO: render the scaled fluid surface and the visor-dependent fog volume.
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
  // TODO: incrementally ray-test vertices and derive tile/patch coverage flags.
}

void CScriptWater::SetupGrid(bool recomputeClipping) {
  // TODO: resize and initialize the grid buffers when the surface dimensions change.
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
  // TODO: update the actor's fluid count and send entry messages/camera callbacks.
}

void CScriptWater::InhabitantExited(CActor& actor, CStateManager& mgr) {
  // TODO: update the actor's fluid count and send exit messages/camera callbacks.
}

void CScriptWater::InhabitantIdle(CActor& actor, CStateManager& mgr) {
  // TODO: send the per-inhabitant inside-fluid message.
}

CFluidUVMotion::SFluidLayerMotion LdrToFluidLayerMotion(const SLdrLayerInfo& data) {}

CEntity* LoadWater(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {}
