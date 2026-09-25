#ifndef _CPARTICLEDATABASE
#define _CPARTICLEDATABASE

#include "Kyoto/Animation/CCharacterInfo.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/rc_ptr.hpp"

class CAnimData;
class CCharLayoutInfo;
class CColor;
class CFrustumPlanes;
class CParticleData;
class CPositionalParticleData;
class CParticleGenInfo;
class CStateManager;
class CTransform4f;
class CVector3f;
class CGenDescription;
class CSwooshDescription;
class CElectricDescription;
// Guessed names: resource tags are established, concrete description names are not.
class CParticleDescriptionSPSC;
class CParticleDescriptionSRSC;

class CParticleDatabase {
public:
  typedef rstl::map< uint, rstl::auto_ptr< CParticleGenInfo > > DrawMap;

  CParticleDatabase();
  ~CParticleDatabase();

  rstl::optional_object< CAABox > GetTotalBounds() const;
  void ClearAllNonPersistentEffects(CStateManager* mgr);
  void DestroyAllActiveParticles();
  void SetModulationColorAllActiveEffects(const CColor& color);
  void SuspendAllActiveEffects(CStateManager* mgr);
  void DeleteAllLights(CStateManager* mgr);
  void RenderSystemsToBeDrawnLastPOICheck(uint mask, uint target) const;
  void RenderSystemsToBeDrawnLast() const;
  void RenderSystemsToBeDrawnFirstPOICheck(uint mask, uint target) const;
  void RenderSystemsToBeDrawnFirst() const;
  void AddToRendererClippedMasked(const CFrustumPlanes& frustum, uint mask, uint target) const;
  void RenderSystemsNormallyAddedToRenderer() const; // Guessed name
  void AddToRendererClipped(const CFrustumPlanes& frustum) const;
  void Update(float dt, CAnimData& animData, const CCharLayoutInfo& layout, const CTransform4f& xf,
              const CVector3f& scale, CStateManager* mgr);
  void SetParticleExternalParam(uint name, int index, float value);
  void SetParticleEffectState(uint name, bool active, CStateManager* mgr);
  CParticleGenInfo* GetParticleEffect(uint name);
  void AddParticleEffect(uint name, int flags, const CPositionalParticleData& data,
                         const CVector3f& scale, CStateManager* mgr, TAreaId areaId, uint lightId);
  void AddParticleEffect(uint name, int flags, const CParticleData& data, const CVector3f& scale,
                         CStateManager* mgr, TAreaId areaId, bool oneShot, uint lightId);
  void CacheParticleDesc(const SObjectTag& tag);
  void CacheParticleDesc(const CCharacterInfo::CParticleResData& data);
  void SetUpdatesEnabled(bool enabled) { mUpdatesEnabled = enabled; }
  bool AreAnySystemsDrawnWithModel() const { return mAnySystemsDrawnWithModel; }

private:
  // Guessed name
  static void AccumulateBounds(rstl::optional_object< CAABox >& bounds, const DrawMap& map);
  void DestroyParticlesForParticleDB(const DrawMap& map);
  static void SetModulationColorAllActiveEffectsForParticleDB(const CColor& color,
                                                              const DrawMap& map);
  static void SuspendAllActiveEffectsForParticleDB(CStateManager* mgr, const DrawMap& map);
  static void DeleteAllLightsForParticleDB(CStateManager* mgr, const DrawMap& map);
  static void RenderParticleGenMapMasked(const DrawMap& map, uint mask, uint target);
  static void RenderParticleGenMap(const DrawMap& map);
  void AddToRendererClippedParticleGenMapMasked(const DrawMap& map, const CFrustumPlanes& frustum,
                                                uint mask, uint target) const;
  void AddToRendererClippedParticleGenMap(const DrawMap& map, const CFrustumPlanes& frustum) const;
  static void UpdateParticleGenDB(float dt, CAnimData& animData, const CCharLayoutInfo& layout,
                                  const CTransform4f& xf, const CVector3f& scale,
                                  CStateManager* mgr, DrawMap& map, bool deleteIfDone);
  static void SetParticleEffectState(CParticleGenInfo* effect, bool active, CStateManager* mgr);
  void InsertParticleGen(bool oneShot, int flags, uint name,
                         const rstl::auto_ptr< CParticleGenInfo >& gen);

  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > > mParticleDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > > mSwooshDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > > mElectricDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CParticleDescriptionSPSC > > > mSpscDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CParticleDescriptionSRSC > > > mSrscDescs;
  DrawMap mRendererDrawLoop;
  DrawMap mFirstDrawLoop;
  DrawMap mLastDrawLoop;
  DrawMap mRendererDraw;
  DrawMap mFirstDraw;
  DrawMap mLastDraw;
  bool mUpdatesEnabled : 1;
  bool mAnySystemsDrawnWithModel : 1;
};
CHECK_SIZEOF(CParticleDatabase, 0xe0)

#endif // _CPARTICLEDATABASE
