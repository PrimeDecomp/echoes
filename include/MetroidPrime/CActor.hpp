#ifndef _CACTOR
#define _CACTOR

#include "types.h"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Animation/CSegId.hpp"
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

  // Echoes sound records include a locator and a volume-selection flag.
  struct SSound {
    SSound(const CSfxHandle& handle, const CSegId& locator, bool useEchoVolume);

    CSfxHandle mHandle;
    CSegId mLocator;
    bool mUseEchoVolume : 1;
  };
  typedef rstl::pair< TSfxId, SSound > TLoopingSound;

  CActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, uint inGrave,
         const CTransform4f& xf, const CModelData& mData, const CMaterialList& list,
         const CActorParameters& params, TUniqueId nextDrawNode);

  // CEntity
  ~CActor() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;
  void SetActive(const bool active) override;

  // CActor
  virtual void UnkVtable20(CStateManager& mgr); // Original name unknown
  virtual void PreRender(CStateManager&);
  virtual void AddToRenderer(const CStateManager&) const;
  virtual void Render(const CStateManager&) const;
  virtual bool CanRenderUnsorted(const CStateManager&) const;
  virtual void PreRenderAllViewports(CStateManager& mgr);
  virtual CHealthInfo* HealthInfo();
  virtual const CHealthInfo* GetHealthInfo() const {
    return const_cast< CActor* >(this)->HealthInfo();
  }
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
  virtual CScannableObjectInfo* GetScannableObjectInfo() const;
  virtual void ProcessSoundEvent(int sfxId, float weight, int flags, float fallOff, float maxDist,
                                 uchar minVol, uchar maxVol, const CVector3f& toListener,
                                 const CVector3f& position, int aid, CStateManager& mgr,
                                 bool translateId);

  CAdvancementDeltas UpdateAnimation(float dt, CStateManager& mgr, bool advTree);

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
  void fn_8004ab14(); // Allocate the simple shadow if model data is available.

  const CTransform4f& GetTransform() const { return mTransform; }
  void SetTransform(const CTransform4f& xf) {
    mTransform = xf;
    SetTransformDirty(true);
    SetTransformDirtySpare(true);
    SetPreRenderHasMoved(true);
  }
  void SetTransformAlt(const CTransform4f& xf);
  void SetRotation(const CQuaternion& rot) { SetTransform(rot.BuildTransform4f(GetTranslation())); }
  CQuaternion GetRotation() const { return CQuaternion::FromMatrix(GetTransform()); }
  const CVector3f& GetTranslation() const { return mPosition; }
  void SetTranslation(const CVector3f& vec);
  CTransform4f GetLocatorTransform(const rstl::string& segName) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& segName) const;
  float GetYaw() const;
  float GetPitch() const;
  void SetActorLights(rstl::auto_ptr< CActorLights > lights);
  void SetInFluid(bool b, TUniqueId uid);

  void MoveScannableObjectInfoToActor(CActor* actor, CStateManager& mgr);

  /// ????
  bool NullModel() const { return !GetAnimationData() && !GetModelData()->HasNormalModel(); }

  bool HasModelData() const {
    return GetModelData() && (GetModelData()->HasAnimation() || GetModelData()->HasNormalModel());
  }
  CModelData* ModelData() { return mModelData.get(); }
  const CModelData* GetModelData() const { return mModelData.get(); }

  bool HasAnimation() const { return GetModelData() && GetModelData()->HasAnimation(); }
  CAnimData* AnimationData() { return ModelData()->AnimationData(); }
  const CAnimData* GetAnimationData() const { return GetModelData()->GetAnimationData(); }

  bool HasShadow() const { return GetShadow() != nullptr; }
  CSimpleShadow* Shadow() { return mSimpleShadow.get(); }
  const CSimpleShadow* GetShadow() const { return mSimpleShadow.get(); }

  bool HasActorLights() const { return !mActorLights.null(); }
  CActorLights* ActorLights() { return mActorLights.get(); }
  const CActorLights* GetActorLights() const { return mActorLights.get(); }

  const CModelFlags& GetModelFlags() const { return mDrawFlags; }
  void SetModelFlags(const CModelFlags& flags) { mDrawFlags = flags; }

  const CMaterialList& GetMaterialList() const { return mMaterial; }
  CMaterialList& MaterialList() { return mMaterial; }

  const CMaterialFilter& GetMaterialFilter() const;
  void SetMaterialFilter(const CMaterialFilter& filter);

  bool GetTransformDirty() const { return mNotInSortedLists; }
  bool GetTransformDirtySpare() const { return mTransformDirty; }
  bool GetPreRenderHasMoved() const { return mActorLightsDirty; }
  bool GetPreRenderClipped() const { return mOutOfFrustum; }
  bool GetCalculateLighting() const { return mCalculateLighting && HasActorLights(); }
  bool GetDrawShadow() const { return mShadowEnabled; }
  bool GetShadowDirty() const { return mShadowDirty; }
  bool GetMuted() const { return mMuted; }
  // EThermalFlags GetThermalFlags() const {
  //   return static_cast< EThermalFlags >(mThermalVisorFlags);
  // }
  bool GetRenderParticleDatabaseInside() const { return mRenderParticleDBInside; }
  bool GetTargetable() const { return mTargetable; }

  void SetTransformDirty(bool b) { mNotInSortedLists = b; }
  void SetTransformDirtySpare(bool b) { mTransformDirty = b; }
  void SetPreRenderHasMoved(bool b) { mActorLightsDirty = b; }
  void SetPreRenderClipped(bool b) { mOutOfFrustum = b; }
  void SetCalculateLighting(bool b);
  void SetDrawShadow(bool b) { mShadowEnabled = b; }
  void SetShadowDirty(bool b) { mShadowDirty = b; }
  void SetMuted(bool b);
  // void SetThermalFlags(EThermalFlags flags) { mThermalVisorFlags = flags; }
  void SetRenderParticleDatabaseInside(bool b) { mRenderParticleDBInside = b; }
  void SetTargetable(bool b) { mTargetable = b; }
  void SetDrawEnabled(bool enabled) { mDrawEnabled = enabled; }

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
  void AddMaterial(const CMaterialList& l) { mMaterial.Add(l); }
  void SetMaterialList(const CMaterialList& l, CStateManager&);

  const CAABox& GetRenderBoundsCached() const { return mRenderBounds; }
  void SetRenderBounds(const CAABox& bounds) { mRenderBounds = bounds; }
  void SetOtherBounds(const CAABox& bounds) { mOtherBounds = bounds; }

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

  void SetNextDrawNode(TUniqueId id) { mNextDrawNode = id; }

  void SetDirtyFlags();

private:
  // The component's concrete type and remaining virtual interface are unresolved.
  class CUnknownComponent {
  public:
    virtual ~CUnknownComponent();
  };

  CTransform4f mTransform;                   // x24
  CVector3f mPosition;                       // x54
  rstl::single_ptr< CModelData > mModelData; // x60
  int mPostModelDataFiller;
  CMaterialList mMaterial; // x68
  CMaterialFilter mMaterialFilter;
  rstl::reserved_vector< TLoopingSound, 4 > mLoopingSounds; // x88
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::single_ptr< CSimpleShadow > mSimpleShadow;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanObjectInfo;
  rstl::single_ptr< CUnknownComponent > xc8_;
  CAABox mOtherBounds;
  CAABox mRenderBounds;
  CModelFlags mDrawFlags;
  float mTime;
  uint mPitchBend;
  rstl::reserved_vector< TUniqueId, 4 > mFluidIds;
  rstl::reserved_vector< TUniqueId, 4 > mPreviousFluidIds;
  bool mFluidIdsChanged : 1;
  TUniqueId mNextDrawNode;
  int mDrawnToken;
  int mAddedToken;
  int x134_;
  uchar mMaxVol;
  uchar mNormalVolume;
  uchar mEchoVolume;
  rstl::reserved_vector< SSound, 2 > mNonLoopingSounds; // x13c
  uint mNextNonLoopingSfxHandle : 3;                    // x150
  uint mNotInSortedLists : 1;
  uint mTransformDirty : 1;
  uint mActorLightsDirty : 1;
  uint mRenderBoundsDirty : 1;
  uint mOutOfFrustum : 1;
  uint mCalculateLighting : 1; // x151
  uint mShadowEnabled : 1;
  uint mShadowDirty : 1;
  uint mMuted : 1;
  uint mUseInSortedLists : 1;
  uint x151_5_ : 1;
  uint mCallTouch : 1;
  uint mGlobalTimeProvider : 1;
  uint mRenderUnsorted : 1; // x152
  uint mPointGeneratorParticles : 1;
  uint mRenderParticleDBInside : 1;
  uint mEnablePitchBend : 1;
  uint mTargetableVisorFlags : 4;
  uint mEnableRender : 1; // x153
  uint mWorldLightingDirty : 1;
  uint mDrawEnabled : 1;
  uint mDoTargetDistanceTest : 1;
  uint x153_4_ : 1;
  uint x153_5_ : 1;
  uint mTargetable : 1;
  uint x153_7_ : 1;
  uint x154_0_ : 1;
  uint x154_1_ : 1;
  uint x154_2_ : 1;
  uint x154_3_ : 1;
  uint mLoopingSoundCount : 3;
  uint x154_7_ : 1;
};
CHECK_SIZEOF(CActor, 0x158)
NESTED_CHECK_SIZEOF(CActor, SSound, 0x8)
NESTED_CHECK_SIZEOF(CActor, TLoopingSound, 0xc)

#endif // _CACTOR
