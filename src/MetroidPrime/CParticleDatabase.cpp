#include "MetroidPrime/CParticleDatabase.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSortedParticleSystem.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CParticleGenInfo.hpp"
#include "MetroidPrime/CParticleGenInfoGeneric.hpp"
#include "MetroidPrime/CPositionalParticleData.hpp"

template < class T >
static int GetGraphicLightId(const rstl::ncrc_ptr< CParticleGen >& system,
                             const TLockedToken< T >& desc) {
  const TToken< T >& token = desc;
  return system->SystemHasLight() ? token.GetTag().GetId() : -1;
}

CParticleDatabase::CParticleDatabase() : mUpdatesEnabled(true), mAnySystemsDrawnWithModel(false) {}

CParticleDatabase::~CParticleDatabase() {}

template < class T, FourCC Type >
static void CacheParticleId(const CAssetId& id,
                            rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< T > > >& cache) {
  if (cache.find(id) == cache.end()) {
    rstl::rc_ptr< TLockedToken< T > > desc(
        rs_new TLockedToken< T >(gpSimplePool->GetObj(SObjectTag(Type, id))));
    cache.insert(rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< T > > >(id, desc));
  }
}

template < class T, FourCC Type >
static void CacheParticleList(const rstl::vector< CAssetId >& ids,
                              rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< T > > >& cache) {
  for (rstl::vector< CAssetId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    const CAssetId id = *it;
    if (cache.find(id) == cache.end()) {
      rstl::rc_ptr< TLockedToken< T > > desc(
          rs_new TLockedToken< T >(gpSimplePool->GetObj(SObjectTag(Type, id))));
      cache.insert(rstl::pair< CAssetId, rstl::rc_ptr< TLockedToken< T > > >(id, desc));
    }
  }
}

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& data) {
  CacheParticleList< CGenDescription, 'PART' >(data.GetParts(), mParticleDescs);
  CacheParticleList< CSwooshDescription, 'SWHC' >(data.GetSwooshes(), mSwooshDescs);
  CacheParticleList< CElectricDescription, 'ELSC' >(data.GetElectrics(), mElectricDescs);
  CacheParticleList< CSpawnSystemDescription, 'SPSC' >(data.GetSpawnSystems(), mSpscDescs);
  CacheParticleList< CSortedParticleSystemDescription, 'SRSC' >(data.GetSortedSystems(),
                                                                mSrscDescs);
}

void CParticleDatabase::CacheParticleDesc(const SObjectTag& tag) {
  const CAssetId id = tag.GetId();
  switch (tag.GetType()) {
  case 'PART':
    CacheParticleId< CGenDescription, 'PART' >(id, mParticleDescs);
    break;
  case 'SWHC':
    CacheParticleId< CSwooshDescription, 'SWHC' >(id, mSwooshDescs);
    break;
  case 'ELSC':
    CacheParticleId< CElectricDescription, 'ELSC' >(id, mElectricDescs);
    break;
  case 'SPSC':
    CacheParticleId< CSpawnSystemDescription, 'SPSC' >(id, mSpscDescs);
    break;
  case 'SRSC':
    CacheParticleId< CSortedParticleSystemDescription, 'SRSC' >(id, mSrscDescs);
    break;
  }
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
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect == nullptr) {
    const SObjectTag& tag = data.GetParticleAssetInfo();
    const float scaleFactor = data.GetScale();
    const CVector3f particleScale =
        (flags & 2) ? CVector3f(scaleFactor, scaleFactor, scaleFactor)
                    : CVector3f(scaleFactor * scale.GetX(), scaleFactor * scale.GetY(),
                                scaleFactor * scale.GetZ());
    rstl::auto_ptr< CParticleGenInfo > gen;
    switch (tag.GetType()) {
    case 'PART': {
      rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > >::iterator it =
          mParticleDescs.find(tag.GetId());
      if (it != mParticleDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CElementGen(*it->second);
        const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, data.GetDuration(),
                                             data.GetSegmentId(), particleScale,
                                             data.GetParentedMode(), flags, mgr, areaId,
                                             particleLightId, kPGT_Normal);
      }
      break;
    }
    case 'SWHC': {
      rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > >::iterator it =
          mSwooshDescs.find(tag.GetId());
      if (it != mSwooshDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CParticleSwoosh(*it->second, 0);
        const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, data.GetDuration(),
                                             data.GetSegmentId(), particleScale,
                                             data.GetParentedMode(), flags, mgr, areaId,
                                             particleLightId, kPGT_Normal);
      }
      break;
    }
    case 'ELSC': {
      rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > >::iterator it =
          mElectricDescs.find(tag.GetId());
      if (it != mElectricDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system = rs_new CParticleElectric(*it->second);
        const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, data.GetDuration(),
                                             data.GetSegmentId(), particleScale,
                                             data.GetParentedMode(), flags, mgr, areaId,
                                             particleLightId, kPGT_Normal);
      }
      break;
    }
    case 'SPSC': {
      rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSpawnSystemDescription > > >::iterator it =
          mSpscDescs.find(tag.GetId());
      if (it != mSpscDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system =
            rs_new CParticleSpawnSystem(*it->second, CElementGen::kOSF_One, false);
        const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, data.GetDuration(),
                                             data.GetSegmentId(), particleScale,
                                             data.GetParentedMode(), flags, mgr, areaId,
                                             particleLightId, kPGT_Normal);
      }
      break;
    }
    case 'SRSC': {
      rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSortedParticleSystemDescription > > >::iterator it =
          mSrscDescs.find(tag.GetId());
      if (it != mSrscDescs.end()) {
        rstl::ncrc_ptr< CParticleGen > system =
            rs_new CSortedParticleSystem(*it->second, CElementGen::kOSF_One, false);
        const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
        gen = rs_new CParticleGenInfoGeneric(tag, system, data.GetDuration(),
                                             data.GetSegmentId(), particleScale,
                                             data.GetParentedMode(), flags, mgr, areaId,
                                             particleLightId, kPGT_Normal);
      }
      break;
    }
    }
    if (!gen.null()) {
      gen->SetIsActive(true);
      gen->SetParticleEmission(true, mgr);
      gen->SetIsGrabInitialData(true);
      InsertParticleGen(oneShot, flags, name, gen);
    }
  } else if (!effect->GetIsActive()) {
    effect->SetParticleEmission(true, mgr);
    effect->SetIsActive(true);
    effect->SetIsGrabInitialData(true);
    effect->SetFlags(flags);
  }
}

void CParticleDatabase::AddParticleEffect(uint name, int flags, const CPositionalParticleData& data,
                                          const CVector3f& scale, CStateManager* mgr,
                                          TAreaId areaId, uint lightId) {
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect == nullptr) {
    const SObjectTag& tag = data.GetParticleAssetInfo();
    if (tag.GetType() != 'PART')
      return;
    rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > >::iterator it =
        mParticleDescs.find(tag.GetId());
    if (it == mParticleDescs.end())
      return;
    const float scaleFactor = data.GetScale();
    const CVector3f particleScale =
        (flags & 2) ? CVector3f(scaleFactor, scaleFactor, scaleFactor)
                    : CVector3f(scaleFactor * scale.GetX(), scaleFactor * scale.GetY(),
                                scaleFactor * scale.GetZ());
    rstl::ncrc_ptr< CParticleGen > system = rs_new CElementGen(*it->second);
    const uint particleLightId = lightId + GetGraphicLightId(system, *it->second);
    rstl::auto_ptr< CParticleGenInfo > gen = rs_new CParticleGenInfoGeneric(
        tag, system, data.GetDuration(), CSegId(0), particleScale, CParticleData::kPM_Initial,
        flags, mgr, areaId, particleLightId, kPGT_Auxiliary);
    gen->SetGlobalOrientation(data.GetTransform(), mgr);
    gen->SetGlobalTranslation(data.GetTransform().GetTranslation(), mgr);
    gen->SetGlobalScale(particleScale);
    gen->SetIsGrabInitialData(false);
    InsertParticleGen(false, flags, name, gen);
  } else if (!effect->GetIsActive()) {
    effect->SetParticleEmission(true, mgr);
    effect->SetIsActive(true);
    effect->SetIsGrabInitialData(true);
    effect->SetFlags(flags);
  }
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
  CParticleGenInfo* effect = GetParticleEffect(name);
  if (effect != nullptr) {
    CElementGen* system = static_cast< CElementGen* >(
        static_cast< CParticleGenInfoGeneric* >(effect)->GetParticleSystem().GetPtr());
    system->SetExternalParam(index, value);
  }
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
  DrawMap::iterator it = map.begin();
  while (it != map.end()) {
    CParticleGenInfo& info = *it->second;
    if (info.GetIsActive() || info.HasActiveParticles()) {
      if (info.GetType() == kPGT_Normal && info.GetSegmentId() != CSegId::Invalid()) {
        const CSegId seg = info.GetSegmentId();
        const CParticleData::EParentedMode mode = info.GetParentedMode();
        if (mode == CParticleData::kPM_Initial) {
          if (info.GetIsGrabInitialData()) {
            animData.BuildPoseIfNecessary();
            CPoseAsTransforms_Linear& pose = animData.Pose();
            const CVector3f& offset = pose.GetOffset(seg);
            CMatrix3f rotation = (info.GetFlags() & 0x10) ? CMatrix3f::Identity()
                                                         : pose.GetRotation(seg);
            if (info.GetFlags() & 0x10000)
              rotation = rotation * layout.GetLinearRotations()[seg.val()].BuildTransform();
            const CVector3f scaledOffset(offset.GetX() * scale.GetX(),
                                         offset.GetY() * scale.GetY(),
                                         offset.GetZ() * scale.GetZ());
            const CTransform4f composed = xf * CTransform4f(rotation, scaledOffset);
            info.SetCurTransform(composed.GetRotation());
            info.SetCurOffset(composed.GetTranslation());
            info.ResetTime();
            info.SetIsGrabInitialData(false);
          }
          info.SetOrientation(info.GetCurTransform(), mgr);
          info.SetTranslation(info.GetCurOffset(), mgr);
        } else if (mode == CParticleData::kPM_ContinuousEmitter ||
                   mode == CParticleData::kPM_ContinuousSystem) {
          animData.BuildPoseIfNecessary();
          CPoseAsTransforms_Linear& pose = animData.Pose();
          const CVector3f& offset = pose.GetOffset(seg);
          if (info.GetIsGrabInitialData()) {
            info.ResetTime();
            info.SetIsGrabInitialData(false);
          }
          CMatrix3f rotation = pose.GetRotation(seg);
          if (info.GetFlags() & 0x10000)
            rotation = rotation * layout.GetLinearRotations()[seg.val()].BuildTransform();
          const CVector3f scaledOffset(offset.GetX() * scale.GetX(), offset.GetY() * scale.GetY(),
                                       offset.GetZ() * scale.GetZ());
          const CTransform4f composed = xf * CTransform4f(rotation, scaledOffset);
          const CTransform4f& orientation =
              (info.GetFlags() & 0x10) ? xf : composed;
          if (mode == CParticleData::kPM_ContinuousEmitter) {
            info.SetTranslation(composed.GetTranslation(), mgr);
            info.SetOrientation(orientation.GetRotation(), mgr);
          } else {
            info.SetGlobalTranslation(composed.GetTranslation(), mgr);
            info.SetGlobalOrientation(orientation.GetRotation(), mgr);
          }
        }
        if (mode == CParticleData::kPM_Initial || mode == CParticleData::kPM_ContinuousEmitter ||
            mode == CParticleData::kPM_ContinuousSystem) {
          const CVector3f& particleScale = info.GetScale();
          if (info.GetFlags() & 0x2000) {
            info.SetGlobalScale(CVector3f(particleScale.GetX() * scale.GetX(),
                                          particleScale.GetY() * scale.GetY(),
                                          particleScale.GetZ() * scale.GetZ()));
          } else {
            info.SetGlobalScale(particleScale);
          }
        }
      }
      const float duration =
          info.GetInactiveStartTime() == 0.f ? 10000000.f : info.GetInactiveStartTime();
      if (duration <= info.GetCurrentTime() && info.GetIsActive()) {
        info.SetIsActive(false);
        info.SetParticleEmission(false, mgr);
        info.MarkFinishTime();
        if (info.GetFlags() & 1)
          info.DestroyParticles();
      }
    }
    info.Update(dt, mgr);
    if (!info.GetIsActive()) {
      if (!info.HasActiveParticles() && info.GetCurrentTime() - info.GetFinishTime() > 5.f &&
          deleteIfDone) {
        info.DeleteLight(mgr);
        it = map.erase(it);
        continue;
      }
    } else if (info.IsSystemDeletable()) {
      info.DeleteLight(mgr);
      it = map.erase(it);
      continue;
    }
    info.OffsetTime(dt);
    ++it;
  }
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
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    CParticleGenInfo* gen = it->second.get();
    if (frustum.BoxInFrustumPlanes(gen->GetBounds()))
      gen->AddToRenderer();
  }
}

void CParticleDatabase::AddToRendererClippedParticleGenMapMasked(const DrawMap& map,
                                                                 const CFrustumPlanes& frustum,
                                                                 uint mask, uint target) const {
  for (DrawMap::const_iterator it = map.begin(); it != map.end(); ++it) {
    CParticleGenInfo* gen = it->second.get();
    if ((gen->GetFlags() & mask) == target && frustum.BoxInFrustumPlanes(gen->GetBounds()))
      gen->AddToRenderer();
  }
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
  mRendererDrawLoop.clear();
  mFirstDrawLoop.clear();
  mLastDrawLoop.clear();
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
