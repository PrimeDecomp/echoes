#ifndef _CACTOR
#define _CACTOR

#include "types.h"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
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
class CEchoEmitter;
struct SEchoParameters;
struct SLdrAudioPlaybackParms;
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
  virtual void ClearFluidList(CStateManager& mgr);
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
                                 const CSegId& locator, ushort pitchStart, ushort pitchEnd,
                                 float pitchDuration, uchar minVol, uchar maxVol,
                                 float distanceSquared, const CVector3f& position, int aid,
                                 CStateManager& mgr, bool translateId);

  CAdvancementDeltas UpdateAnimation(float dt, CStateManager& mgr, bool advTree);

  void UpdateSfxEmitters(CStateManager& mgr);
  void StopLoopedSounds();
  bool FindLoopedSound(ushort sfxId);
  void StopLoopedSound(ushort sfxId); // Guessed name.
  CSfxHandle PlayCustomSound(const CVector3f& position, const CVector3f& direction,
                             const SLdrAudioPlaybackParms& parameters, bool looped) const;
  void SetModelData(const CModelData& modelData, CStateManager& mgr);
  float GetAverageAnimVelocity(int anim);
  void EnsureRendered(const CStateManager& mgr) const;
  void EnsureRendered(const CStateManager& mgr, const CVector3f& pos, const CAABox& bounds) const;
  bool IsModelOpaque(const CStateManager& mgr) const;
  void RenderInternal(const CStateManager& mgr) const;
  void AllocateShadow(); // Guessed name.

  void UpdatePortalSystemState(CStateManager& mgr);
  // Despite the original name, this returns the minimum squared camera distance.
  float GetDistanceToCamera(CStateManager& mgr) const;
  void SetEchoEmitter(bool enabled, CEchoEmitter* emitter);
  CEchoEmitter* EchoEmitter() { return mEchoEmitter.get(); }
  CEchoEmitter* AllocateEchoEmitter(bool enabled, const CAABox& bounds,
                                    const SEchoParameters& parameters);
  void SetValidTarget(int playerIndex, bool enabled);
  void SetVisorOrbitableFlags(CVisorParameters::EVisorOrbitableFlags flags, bool enabled);

  const CTransform4f& GetTransform() const { return mTransform; }
  void SetTransform(const CTransform4f& xf);
  void SetRotation(const CQuaternion& rot) { SetTransform(rot.BuildTransform4f(GetTranslation())); }
  CQuaternion GetRotation() const { return CQuaternion::FromMatrix(GetTransform()); }
  const CVector3f& GetTranslation() const { return mPosition; }
  void SetTranslation(const CVector3f& vec);
  CTransform4f GetLocatorTransform(const rstl::string& segName) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& segName) const;
  CTransform4f GetScaledLocatorTransform(const CSegId& locator) const;
  float GetYaw() const;
  void SetActorLights(rstl::auto_ptr< CActorLights > lights);
  void SetWorldLightingDirty(bool dirty) { mWorldLightingDirty = dirty; }
  void SetInFluid(CStateManager& mgr, bool inFluid, TUniqueId uid);
  TUniqueId InFluidId() const;
  // Guessed names.
  void SetFluidList(const rstl::reserved_vector< TUniqueId, 4 >& fluids);
  const rstl::reserved_vector< TUniqueId, 4 >& GetFluidList() const;

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
  bool GetRenderParticleDatabaseInside() const { return mRenderParticleDBInside; }

  void SetTransformDirty(bool b) { mNotInSortedLists = b; }
  void SetTransformDirtySpare(bool b) { mTransformDirty = b; }
  void SetPreRenderHasMoved(bool b) { mActorLightsDirty = b; }
  void SetPreRenderClipped(bool b) { mOutOfFrustum = b; }
  void SetCalculateLighting(bool b);
  void SetDrawShadow(bool enabled);
  void SetShadowDirty(bool b) { mShadowDirty = b; }
  void SetMuted(bool b);
  void SetRenderParticleDatabaseInside(bool b) { mRenderParticleDBInside = b; }
  void SetDrawEnabled(bool enabled) { mDrawEnabled = enabled; }
  void SetDoTargetDistanceTest(bool enabled) { mDoTargetDistanceTest = enabled; }

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
  const CAABox& GetOtherBounds() const { return mOtherBounds; }
  void SetOtherBounds(const CAABox& bounds) { mOtherBounds = bounds; }

  bool GetUseInSortedLists() const;
  void SetUseInSortedLists(bool use);
  bool GetCallTouch() const;
  void SetCallTouch(bool value);
  void SetVolume(uchar volume);
  void SetSoundEventPitchBend(int);
  void ClearSoundEventPitchBend();
  bool CanDrawStatic() const;
  bool ShouldDrawShadow(const CStateManager& mgr) const; // Guessed name.
  int GetRenderAlphaBufferAlpha(const CStateManager& mgr) const;

  void SetNextDrawNode(TUniqueId id) { mNextDrawNode = id; }
  void SetPvsIndex(int index) { mPvsIndex = index; }

  void SetTransformDirty();

private:
  // Guessed names.
  void RemoveInvalidFluidIds(CStateManager& mgr);
  void RemoveLoopedSoundAt(int index);
  uchar GetVisorSoundVolume(const CStateManager& mgr) const;
  void PlayLoopedSound(ushort sfxId, int flags, float fallOff, float maxDist, uchar minVol,
                       uchar maxVol, bool nonEmitter, int area, bool useAcoustics,
                       const CSegId& locator, ushort pitchStart, ushort pitchEnd,
                       float pitchDuration, bool useEchoVolume);
  void AddLoopedSound(ushort sfxId, bool nonEmitter, int area, bool useAcoustics,
                      CAudioSys::C3DEmitterParmData& parameters, const CSegId& locator,
                      ushort pitchStart, ushort pitchEnd, float pitchDuration, bool useEchoVolume);

  CTransform4f mTransform;                   // x24
  CVector3f mPosition;                       // x54
  rstl::single_ptr< CModelData > mModelData; // x60
  CMaterialList mMaterial;                   // x68
  CMaterialFilter mMaterialFilter;
  rstl::reserved_vector< TLoopingSound, 4 > mLoopingSounds; // x88
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::single_ptr< CSimpleShadow > mSimpleShadow;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanObjectInfo;
  rstl::single_ptr< CEchoEmitter > mEchoEmitter;
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
  int mPvsIndex;
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
  uint mUsePortalVisibility : 1;
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
  uint mValidTargetPlayers : 4;
  uint mEchoEmitterEnabled : 1;
  uint mHighlightedInDarkVisor : 1;
  uint mDamageHighlight : 1;
  uint mTakesProjectedShadow : 1;
  uint mLoopingSoundCount : 3;
  uint mAlphaSorted : 1; // Guessed name
};
CHECK_SIZEOF(CActor, 0x158)
NESTED_CHECK_SIZEOF(CActor, SSound, 0x8)
NESTED_CHECK_SIZEOF(CActor, TLoopingSound, 0xc)

#endif // _CACTOR
