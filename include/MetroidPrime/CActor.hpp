#ifndef _CACTOR
#define _CACTOR

#include "types.h"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CActorLights;
class CActorParameters;
class CScannableObjectInfo;
class CSimpleShadow;

class CDamageInfo;
class CDamageVulnerability;
class CFrustumPlanes;
class CHealthInfo;
class CScriptWater;
class CWeaponMode;
class CInt32POINode;

class CActor : public CEntity {
public:
  enum EThermalFlags {
    kTF_None = 0,
    kTF_Cold = 1,
    kTF_Hot = 2,
  };
  enum EFluidState {
    kFS_EnteredFluid,
    kFS_InFluid,
    kFS_LeftFluid,
  };
  enum EScanState {
    kSS_Start,
    kSS_Processing,
    kSS_Done,
  };

  CActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint inGrave,
         const CTransform4f& xf, const CModelData& mData, const CMaterialList& list,
         const CActorParameters& params, TUniqueId nextDrawNode);
  ~CActor() override;

  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
  void SetActive(const bool active) override;

  virtual void PreRender(CStateManager&, const CFrustumPlanes&);
  virtual void AddToRenderer(const CStateManager&) const;
  virtual void Render(const CStateManager&) const;
  virtual bool CanRenderUnsorted(const CStateManager&) const;
  virtual void CalculateRenderBounds(CStateManager& mgr);
  const CHealthInfo* GetHealthInfo(const CStateManager& mgr) const {
    return const_cast< CActor* >(this)->HealthInfo(const_cast< CStateManager& >(mgr));
  }
  virtual CHealthInfo* HealthInfo(CStateManager&);
  virtual void UnkA(); // must be before GetSortingBounds
  virtual const CDamageVulnerability* GetDamageVulnerability() const;
  virtual const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                             const CDamageInfo&) const;
  virtual rstl::optional_object< CAABox > GetTouchBounds() const;
  virtual void Touch(CActor&, CStateManager&);
  virtual CVector3f GetOrbitPosition(const CStateManager&) const;
  virtual CVector3f GetAimPosition(const CStateManager&, float) const;
  virtual CVector3f GetHomingPosition(const CStateManager&, float) const;
  virtual CVector3f GetScanObjectIndicatorPosition(const CStateManager&) const;
  virtual EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                                 const CWeaponMode&,
                                                                 int /*EProjectileAttrib?*/) const;
  virtual void FluidFXThink(EFluidState, CScriptWater&, CStateManager&);
  virtual void OnScanStateChange(EScanState, CStateManager&);
  virtual CAABox GetSortingBounds(const CStateManager&) const;
  virtual void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt);

  CAdvancementDeltas UpdateAnimation(float dt, CStateManager& mgr, bool advTree);

  void ProcessSoundEvent(int sfxId, float weight, int flags, float fallOff, float maxDist,
                         uchar minVol, uchar maxVol, const CVector3f& toListener,
                         const CVector3f& position, int aid, CStateManager& mgr, bool translateId);

  void UpdateSfxEmitters();
  void RemoveEmitter();
  void SetModelData(const CModelData& modelData, CStateManager& mgr);
  float GetAverageAnimVelocity(int anim);
  void EnsureRendered(const CStateManager& mgr) const;
  void EnsureRendered(const CStateManager& mgr, const CVector3f& pos, const CAABox& bounds) const;
  void DrawTouchBounds() const;
  bool IsModelOpaque(const CStateManager& mgr) const;
  void RenderInternal(const CStateManager& mgr) const;
  void CreateShadow(bool);

  const CTransform4f& GetTransform() const { return m_transform; }
  void SetTransform(const CTransform4f& xf) {
    m_transform = xf;
    SetTransformDirty(true);
    SetTransformDirtySpare(true);
    SetPreRenderHasMoved(true);
  }
  void SetTransformAlt(const CTransform4f& xf);
  void SetRotation(const CQuaternion& rot) { SetTransform(rot.BuildTransform4f(GetTranslation())); }
  CQuaternion GetRotation() const { return CQuaternion::FromMatrix(GetTransform()); }
  const CVector3f& GetTranslation() const { return m_position; }
  void SetTranslation(const CVector3f& vec);
  CTransform4f GetLocatorTransform(const rstl::string& segName) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& segName) const;
  float GetYaw() const;
  float GetPitch() const;
  void SetActorLights(rstl::auto_ptr< CActorLights > lights);
  void SetInFluid(bool b, TUniqueId uid);

  CScannableObjectInfo* GetScannableObjectInfo() const;
  void MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr);

  /// ????
  bool NullModel() const { return !GetAnimationData() && !GetModelData()->HasNormalModel(); }

  bool HasModelData() const {
    return GetModelData() && (GetModelData()->HasAnimation() || GetModelData()->HasNormalModel());
  }
  CModelData* ModelData() { return m_modelData.get(); }
  const CModelData* GetModelData() const { return m_modelData.get(); }

  bool HasAnimation() const { return GetModelData() && GetModelData()->HasAnimation(); }
  CAnimData* AnimationData() { return ModelData()->AnimationData(); }
  const CAnimData* GetAnimationData() const { return GetModelData()->GetAnimationData(); }

  bool HasShadow() const { return GetShadow() != nullptr; }
  CSimpleShadow* Shadow() { return x94_simpleShadow.get(); }
  const CSimpleShadow* GetShadow() const { return x94_simpleShadow.get(); }

  bool HasActorLights() const { return !x90_actorLights.null(); }
  CActorLights* ActorLights() { return x90_actorLights.get(); }
  const CActorLights* GetActorLights() const { return x90_actorLights.get(); }

  const CModelFlags& GetModelFlags() const { return xb4_drawFlags; }
  void SetModelFlags(const CModelFlags& flags) { xb4_drawFlags = flags; }

  const CMaterialList& GetMaterialList() const { return m_material; }
  CMaterialList& MaterialList() { return m_material; }

  const CMaterialFilter& GetMaterialFilter() const;
  void SetMaterialFilter(const CMaterialFilter& filter);

  bool GetTransformDirty() const { return m_notInSortedLists; }
  bool GetTransformDirtySpare() const { return m_transformDirty; }
  bool GetPreRenderHasMoved() const { return m_actorLightsDirty; }
  bool GetPreRenderClipped() const { return m_outOfFrustum; }
  bool GetCalculateLighting() const { return m_calculateLighting && HasActorLights(); }
  bool GetDrawShadow() const { return m_shadowEnabled; }
  bool GetShadowDirty() const { return m_shadowDirty; }
  bool GetMuted() const { return m_muted; }
  // EThermalFlags GetThermalFlags() const {
  //   return static_cast< EThermalFlags >(m_thermalVisorFlags);
  // }
  bool GetRenderParticleDatabaseInside() const { return m_renderParticleDBInside; }
  bool GetTargetable() const { return m_targetable; }

  void SetTransformDirty(bool b) { m_notInSortedLists = b; }
  void SetTransformDirtySpare(bool b) { m_transformDirty = b; }
  void SetPreRenderHasMoved(bool b) { m_actorLightsDirty = b; }
  void SetPreRenderClipped(bool b) { m_outOfFrustum = b; }
  void SetCalculateLighting(bool b);
  void SetDrawShadow(bool b) { m_shadowEnabled = b; }
  void SetShadowDirty(bool b) { m_shadowDirty = b; }
  void SetMuted(bool b);
  // void SetThermalFlags(EThermalFlags flags) { m_thermalVisorFlags = flags; }
  void SetRenderParticleDatabaseInside(bool b) { m_renderParticleDBInside = b; }
  void SetTargetable(bool b) { m_targetable = b; }

  void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes,
                      EMaterialTypes, CStateManager&);
  void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes,
                      CStateManager&);
  void RemoveMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void RemoveMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
  void RemoveMaterial(EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes,
                   CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, EMaterialTypes, CStateManager&);
  void AddMaterial(EMaterialTypes, CStateManager&);
  void AddMaterial(const CMaterialList& l) { m_material.Add(l); }
  void SetMaterialList(const CMaterialList& l, CStateManager&);

  const CAABox& GetRenderBoundsCached() const { return m_renderBounds; }
  void SetRenderBounds(const CAABox& bounds) { m_renderBounds = bounds; }

  bool GetUseInSortedLists() const;
  void SetUseInSortedLists(bool use);
  bool GetCallTouch() const;
  void SetCallTouch(bool value);
  // GetOrbitDistanceCheck__6CActorCFv
  // GetCalculateLighting__6CActorCFv
  // GetDrawShadow__6CActorCFv
  // GetRenderBoundsCached__6CActorCFv
  // GetRenderParticleDatabaseInside__6CActorCFv
  // HasModelParticles__6CActorCFv
  void SetVolume(uchar volume);
  void SetSoundEventPitchBend(int);
  CSfxHandle GetSfxHandle() const;
  bool CanDrawStatic() const;
  bool fn_8004CD00(const CStateManager& mgr) const;
  int fn_8004CAA0(const CStateManager& mgr) const;

  void SetNextDrawNode(TUniqueId id) { xc6_nextDrawNode = id; }

  void SetDirtyFlags();

private:
  CTransform4f m_transform;                   // x24
  CVector3f m_position;                       // x54
  rstl::single_ptr< CModelData > m_modelData; // x60
  int postModelDataFiller;
  CMaterialList m_material; // x68
  CMaterialFilter x70_materialFilter;
  TSfxId x88_sfxId;
  CSfxHandle x8c_loopingSfxHandle;
  rstl::single_ptr< CActorLights > x90_actorLights;
  rstl::single_ptr< CSimpleShadow > x94_simpleShadow;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > x98_scanObjectInfo;
  CAABox otherBounds;
  CAABox m_renderBounds;
  CModelFlags xb4_drawFlags;
  float xbc_time;
  uint xc0_pitchBend;
  TUniqueId xc4_fluidId;
  TUniqueId xc6_nextDrawNode;
  int xc8_drawnToken;
  int xcc_addedToken;
  float xd0_damageMag;
  uchar xd4_maxVol;
  rstl::reserved_vector< CSfxHandle, 2 > xd8_nonLoopingSfxHandles;
  char actor_padding[84];
  uint m_nextNonLoopingSfxHandle : 3; // xe4_23
  uint m_notInSortedLists : 1;        // xe4_26
  uint m_transformDirty : 1;          // xe4_27
  uint m_actorLightsDirty : 1;        // xe4_28
  uint m_renderBoundsDirty : 1;       // xe4_29
  uint m_outOfFrustum : 1;            // xe4_30
  uint m_calculateLighting : 1;       // xe4_31
  uint m_shadowEnabled : 1;           // xe5_24
  uint m_shadowDirty : 1;             // xe5_25
  uint m_muted : 1;                   // xe5_26 // correct
  uint m_useInSortedLists : 1;        // xe5_27 // correct
  uint m_globalTimeProvider : 1;      // xe5_28
  uint m_callTouch : 1;               // xe5_29 // correct
  uint m_renderUnsorted : 1;          // xe5_30 // wrong bit, check CanRenderUnsorted
  uint unk : 1;
  uint m_pointGeneratorParticles : 1;
  uint m_fluidCounter : 3;           // xe6_24
  uint m_renderParticleDBInside : 1; // xe6_29 // wrong bit, check AddToRenderer
  uint m_enablePitchBend : 1;        // xe6_30
  uint m_targetableVisorFlags : 3;   // xe6_31
  uint m_drawEnabled : 1;            // xe7_29
  uint m_enableRender : 1;           // xe7_27
  uint m_worldLightingDirty : 1;     // xe7_28
  uint m_doTargetDistanceTest : 1;   // xe7_30
  uint m_targetable : 1;             // xe7_31
};
CHECK_SIZEOF(CActor, 0x158)

#endif // _CACTOR
