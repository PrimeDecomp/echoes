#include "MetroidPrime/CParticleDatabase.hpp"

#include "MetroidPrime/CParticleGenInfo.hpp"

CParticleDatabase::CParticleDatabase() : mUpdatesEnabled(true), mAnySystemsDrawnWithModel(false) {}

CParticleDatabase::~CParticleDatabase() {}

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& data) {
  // TODO: correct CParticleResData's five resource lists before traversing them.
}

void CParticleDatabase::CacheParticleDesc(const SObjectTag& tag) {
  // TODO: cache the five supported resource description types.
}

void CParticleDatabase::InsertParticleGen(bool oneShot, int flags, uint name,
                                          const rstl::auto_ptr< CParticleGenInfo >& gen) {
  DrawMap* map;
  switch (flags & 0x60) {
  case 0x20:
    map = oneShot ? &mFirstDraw : &mFirstDrawLoop;
    break;
  case 0x40:
    map = oneShot ? &mLastDraw : &mLastDrawLoop;
    break;
  default:
    map = oneShot ? &mRendererDraw : &mRendererDrawLoop;
    break;
  }
  map->insert(DrawMap::value_type(name, gen));
  if (flags & 0x60)
    mAnySystemsDrawnWithModel = true;
}

void CParticleDatabase::AddParticleEffect(uint name, int flags, const CParticleData& data,
                                          const CVector3f& scale, CStateManager* mgr,
                                          TAreaId areaId, bool oneShot, uint lightId) {
  // TODO: cached PART/SWHC/ELSC/SPSC/SRSC construction and effect initialization.
}

void CParticleDatabase::AddParticleEffect(uint name, int flags, const CPositionalParticleData& data,
                                          const CVector3f& scale, CStateManager* mgr,
                                          TAreaId areaId, uint lightId) {
  // TODO: instantiate a positional PART effect from the cached description.
}

CParticleGenInfo* CParticleDatabase::GetParticleEffect(uint name) {
  {
    DrawMap::iterator it = mRendererDrawLoop.find(name);
    if (it != mRendererDrawLoop.end())
      return it->second.get();
  }
  {
    DrawMap::iterator it = mFirstDrawLoop.find(name);
    if (it != mFirstDrawLoop.end())
      return it->second.get();
  }
  {
    DrawMap::iterator it = mLastDrawLoop.find(name);
    if (it != mLastDrawLoop.end())
      return it->second.get();
  }
  {
    DrawMap::iterator it = mRendererDraw.find(name);
    if (it != mRendererDraw.end())
      return it->second.get();
  }
  {
    DrawMap::iterator it = mFirstDraw.find(name);
    if (it != mFirstDraw.end())
      return it->second.get();
  }
  {
    DrawMap::iterator it = mLastDraw.find(name);
    if (it != mLastDraw.end())
      return it->second.get();
  }
  return nullptr;
}

void CParticleDatabase::SetParticleEffectState(CParticleGenInfo* effect, bool active,
                                               CStateManager* mgr) {
  if (effect == nullptr)
    return;
  effect->SetParticleEmission(active, mgr);
  effect->SetIsActive(active);
  if (!active && (effect->GetFlags() & 1))
    effect->DestroyParticles();
  effect->SetIsGrabInitialData(true);
}

void CParticleDatabase::SetParticleEffectState(uint name, bool active, CStateManager* mgr) {
  SetParticleEffectState(GetParticleEffect(name), active, mgr);
}

void CParticleDatabase::SetParticleExternalParam(uint name, int index, float value) {
  // TODO: set the element generator's external parameter after finding the effect.
}

void CParticleDatabase::Update(float dt, CAnimData& animData, const CCharLayoutInfo& layout,
                               const CTransform4f& xf, const CVector3f& scale, CStateManager* mgr) {
  if (!mUpdatesEnabled)
    return;
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mRendererDrawLoop, true);
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mFirstDrawLoop, true);
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mLastDrawLoop, true);
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mRendererDraw, false);
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mFirstDraw, false);
  UpdateParticleGenDB(dt, animData, layout, xf, scale, mgr, mLastDraw, false);
  mAnySystemsDrawnWithModel =
      mFirstDrawLoop.size() || mLastDrawLoop.size() || mFirstDraw.size() || mLastDraw.size();
}

void CParticleDatabase::UpdateParticleGenDB(float dt, CAnimData& animData,
                                            const CCharLayoutInfo& layout, const CTransform4f& xf,
                                            const CVector3f& scale, CStateManager* mgr,
                                            DrawMap& map, bool deleteIfDone) {
  // TODO: segment transforms, parenting, lifetime and deletion.
}

void CParticleDatabase::AddToRendererClipped(const CFrustumPlanes& frustum) const {
  AddToRendererClippedParticleGenMap(mRendererDraw, frustum);
  AddToRendererClippedParticleGenMap(mRendererDrawLoop, frustum);
}

void CParticleDatabase::RenderSystemsNormallyAddedToRenderer() const {
  RenderParticleGenMap(mRendererDraw);
  RenderParticleGenMap(mRendererDrawLoop);
}

void CParticleDatabase::AddToRendererClippedMasked(const CFrustumPlanes& frustum, uint mask,
                                                   uint target) const {
  AddToRendererClippedParticleGenMapMasked(mRendererDraw, frustum, mask, target);
  AddToRendererClippedParticleGenMapMasked(mRendererDrawLoop, frustum, mask, target);
}

void CParticleDatabase::AddToRendererClippedParticleGenMap(const DrawMap& map,
                                                           const CFrustumPlanes& frustum) const {
  // TODO: particle bounds/frustum test and renderer submission.
}

void CParticleDatabase::AddToRendererClippedParticleGenMapMasked(const DrawMap& map,
                                                                 const CFrustumPlanes& frustum,
                                                                 uint mask, uint target) const {
  // TODO: masked particle bounds/frustum test and renderer submission.
}

void CParticleDatabase::RenderSystemsToBeDrawnFirst() const {
  RenderParticleGenMap(mFirstDraw);
  RenderParticleGenMap(mFirstDrawLoop);
}

void CParticleDatabase::RenderSystemsToBeDrawnFirstPOICheck(uint mask, uint target) const {
  RenderParticleGenMapMasked(mFirstDraw, mask, target);
  RenderParticleGenMapMasked(mFirstDrawLoop, mask, target);
}

void CParticleDatabase::RenderSystemsToBeDrawnLast() const {
  RenderParticleGenMap(mLastDraw);
  RenderParticleGenMap(mLastDrawLoop);
}

void CParticleDatabase::RenderSystemsToBeDrawnLastPOICheck(uint mask, uint target) const {
  RenderParticleGenMapMasked(mLastDraw, mask, target);
  RenderParticleGenMapMasked(mLastDrawLoop, mask, target);
}

void CParticleDatabase::RenderParticleGenMap(const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    it->second->Render();
  }
}

void CParticleDatabase::RenderParticleGenMapMasked(const DrawMap& map, uint mask, uint target) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    if ((it->second->GetFlags() & mask) == target)
      it->second->Render();
  }
}

void CParticleDatabase::DeleteAllLights(CStateManager* mgr) {
  DeleteAllLightsForParticleDB(mgr, mRendererDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mFirstDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mLastDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mRendererDraw);
  DeleteAllLightsForParticleDB(mgr, mFirstDraw);
  DeleteAllLightsForParticleDB(mgr, mLastDraw);
}

void CParticleDatabase::DeleteAllLightsForParticleDB(CStateManager* mgr, const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    it->second->DeleteLight(mgr);
  }
}

void CParticleDatabase::SuspendAllActiveEffects(CStateManager* mgr) {
  SuspendAllActiveEffectsForParticleDB(mgr, mRendererDrawLoop);
  SuspendAllActiveEffectsForParticleDB(mgr, mFirstDrawLoop);
  SuspendAllActiveEffectsForParticleDB(mgr, mLastDrawLoop);
}

void CParticleDatabase::SuspendAllActiveEffectsForParticleDB(CStateManager* mgr,
                                                             const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    SetParticleEffectState(it->second.get(), false, mgr);
  }
}

void CParticleDatabase::SetModulationColorAllActiveEffects(const CColor& color) {
  SetModulationColorAllActiveEffectsForParticleDB(color, mRendererDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mFirstDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mLastDrawLoop);
  SetModulationColorAllActiveEffectsForParticleDB(color, mRendererDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, mFirstDraw);
  SetModulationColorAllActiveEffectsForParticleDB(color, mLastDraw);
}

void CParticleDatabase::SetModulationColorAllActiveEffectsForParticleDB(const CColor& color,
                                                                        const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    if (it->second.get())
      it->second->SetModulationColor(color);
  }
}

void CParticleDatabase::DestroyAllActiveParticles() {
  DestroyParticlesForParticleDB(mRendererDrawLoop);
  DestroyParticlesForParticleDB(mFirstDrawLoop);
  DestroyParticlesForParticleDB(mLastDrawLoop);
  DestroyParticlesForParticleDB(mRendererDraw);
  DestroyParticlesForParticleDB(mFirstDraw);
  DestroyParticlesForParticleDB(mLastDraw);
}

void CParticleDatabase::DestroyParticlesForParticleDB(const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    it->second->DestroyParticles();
  }
}

void CParticleDatabase::ClearAllNonPersistentEffects(CStateManager* mgr) {
  DeleteAllLightsForParticleDB(mgr, mRendererDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mFirstDrawLoop);
  DeleteAllLightsForParticleDB(mgr, mLastDrawLoop);
  // TODO: erase the three non-persistent maps.
}

void CParticleDatabase::AccumulateBounds(rstl::optional_object< CAABox >& bounds,
                                         const DrawMap& map) {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    rstl::optional_object< CAABox > partBounds = it->second->GetBounds();
    if (!partBounds)
      continue;
    if (!bounds) {
      bounds = partBounds;
    } else {
      bounds->AccumulateBounds(partBounds->GetMinPoint());
      bounds->AccumulateBounds(partBounds->GetMaxPoint());
    }
  }
}

rstl::optional_object< CAABox > CParticleDatabase::GetTotalBounds() const {
  rstl::optional_object< CAABox > bounds;
  AccumulateBounds(bounds, mFirstDrawLoop);
  AccumulateBounds(bounds, mLastDrawLoop);
  AccumulateBounds(bounds, mFirstDraw);
  AccumulateBounds(bounds, mLastDraw);
  return bounds;
}
