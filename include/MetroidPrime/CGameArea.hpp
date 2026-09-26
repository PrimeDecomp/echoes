#ifndef _CGAMEAREA
#define _CGAMEAREA

#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetaRender/IRenderer.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/IGameArea.hpp"
#include "WorldFormat/CAreaRenderOctTree.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"
#include "WorldFormat/CWorldLight.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"

class CAreaOctTree;
class CAreaBspTree;
class CPVSAreaSet;
class CPFArea;
class CPortalArea;
class CStaticGeometryMap;
class CScriptAreaProperties;
class CStateManager;
struct TLayerId;

class CGameArea : public IGameArea {
public:
  enum EOcclusionState { kOS_Occluded, kOS_Visible };
  enum EARAMTransfer { kAT_Blocking, kAT_Async };
  // Guessed phase names, derived from the streaming switch.
  enum EPhase {
    kP_Allocate,
    kP_ReadDependencies,
    kP_PrepareDependencies,
    kP_WaitForDependencies,
    kP_LoadHeader,
    kP_LoadSectionSizes,
    kP_ReserveSections,
    kP_LoadDataSections,
    kP_WaitForData,
    kP_WaitForValidation,
    kP_FinishDependencies,
    kP_PostConstruct,
    kP_LoadScriptObjects,
    kP_FinishScriptObjects,
    kP_Unknown14,
    kP_Unknown15,
    kP_Loaded
  };
  // Guessed phase names. Ready precedes script activation.
  enum ELayerPhase {
    kLP_Inactive,
    kLP_CancelPending,
    kLP_RestartPending,
    kLP_Loading,
    kLP_Ready,
    kLP_Active
  };

  class CAreaFog {
  public:
    CAreaFog();
    void DisableFog();
    bool IsFogDisabled() const;
    void SetFogExplicit(ERglFogMode mode, const CColor& color, const CVector2f& range);
    void FadeFog(ERglFogMode mode, const CColor& color, const CVector2f& range, float colorSpeed,
                 const CVector2f& rangeSpeed);
    void RollFogOut(float rangeSpeed, float colorSpeed, const CColor& color);
    CColor GetColor() const;
    void Update(float dt);
    void SetCurrent() const;

  private:
    ERglFogMode mFogMode;
    CVector2f mRangeCur;
    CVector2f mRangeTarget;
    CVector2f mRangeDelta;
    CVector3f mColorCur;
    CVector3f mColorTarget;
    float mColorDelta;
  };

  class CAreaObjectList : public CObjectList {
  public:
    explicit CAreaObjectList(TAreaId areaId) : CObjectList(kOL_Invalid, false), mAreaId(areaId) {}

    // CObjectList
    uchar IsQualified(const CEntity& entity) override;

  private:
    TAreaId mAreaId;
  };

  // Guessed name. One pending compressed MREA block, including its disk request.
  struct SDecompressionRequest {
    uchar* mOutput;
    const uchar* mInput;
    uint mCompressedSize;
    uint mRemainingSize;
    CDvdRequest* mRequest;
  };

  // These auxiliary types still need their payload layouts recovered.
  struct SScriptLoadState;
  struct SUnresolvedListEntry;

  struct CPostConstructed {
    int mMreaVersion;
    rstl::auto_ptr< CAreaOctTree > mCollision;
    int mCollisionSize;
    rstl::optional_object< CAreaRenderOctTree > mRenderOctTree;
    rstl::vector< CMetroidModelInstance > mModelInstances;
    rstl::vector< SAreaSurface > mSurfaces;
    rstl::vector< uint > mAmbientLightIds;
    rstl::vector< signed char > mAmbientLightIndices;
    rstl::single_ptr< CAreaBspTree > mBspTree;
    rstl::vector< CWorldLight > mLightsA;
    rstl::vector< CLight > mGfxLightsA;
    rstl::vector< CWorldLight > mLightsB;
    rstl::vector< CLight > mGfxLightsB;
    rstl::single_ptr< CPVSAreaSet > mPvs;
    int mPvsVersion;
    rstl::optional_object< TLockedToken< CPFArea > > mPathToken;
    CPFArea* mPathArea;
    rstl::single_ptr< TLockedToken< CStaticGeometryMap > > mStaticGeometryMap;
    rstl::single_ptr< CPortalArea > mPortalArea;
    rstl::single_ptr< CAreaObjectList > mAreaObjectList;
    rstl::single_ptr< CAreaObjectList > xfc_;
    rstl::single_ptr< CAreaFog > mAreaFog;
    rstl::vector< rstl::auto_ptr< char > > mLayerScriptBuffers;
    rstl::vector< uint > mLayerScriptSizes;
    rstl::auto_ptr< char > mGeneratedScriptBuffer;
    uint mGeneratedScriptSize;
    rstl::single_ptr< SScriptLoadState > mScriptLoadState;
    const uchar* mFirstMaterial;
    const CScriptAreaProperties* mAreaAttributes;
    EOcclusionState mOcclusionState;
    int mOcclusionFrameCount;
    float mOccludedTime;
    int mFirstAramSection;
    uint mFirstMaterialSection;
    rstl::vector< rstl::vector< CToken > > mLayerTokens;
    rstl::vector< rstl::pair< CARAMToken, int > > mAramTokens;
    uint mAramBytes;
    uint x174_;
    uint mModelsInMram : 1;
    uint mModelsConstructed : 1;
    uint x178_2_ : 1;
    uint mFinishedOccluding : 1;
    uint mOcclusionPinged : 1;
    uint mPvsHasActors : 1;
    uint mPvsHasLights : 1;
    uint mScriptObjectsInitialized : 1;
    uint mStreamingDelay : 3;
    uint mDocksDisabled : 1;
    float mWorldLightingLevel;
    float mXraySpeed;
    float mXrayTarget;
    float mWeaponWorldLightingSpeed;
    float mWeaponWorldLightingTarget;
    int x190_;
    rstl::list< rstl::vector< uint > > x194_;
    rstl::list< SUnresolvedListEntry > x1ac_;
    rstl::list< TUniqueId > mDockIds;
    CTransform4f mInverseTransform;
    rstl::vector< rstl::vector< CRELFileToken > > mLayerRelTokens;
    rstl::vector< CRELFileToken* > mSortedRelTokens;
    rstl::vector< rstl::vector< TEditorId > > mLayerEditorIds;
    rstl::list< rstl::auto_ptr< CDvdRequest > > mLoadTransactions;
    rstl::list< SDecompressionRequest > mDecompressionRequests;
    rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > > mMreaSectionBuffers;
    int mMreaSize;
    int mLoadedSectionCount;
    int mLoadedBlockCount;
    int mMreaDataOffset;
    int mFirstScriptSection;
    rstl::vector< bool > mActiveLayers;
    rstl::vector< uint > mLayerFileOffsets;
    rstl::list< rstl::pair< int, rstl::auto_ptr< CDvdRequest > > > mLayerLoadTransactions;
    uint mDependencyDmaHandle;
    rstl::single_ptr< uchar > mSerializedDependencies;

    // TODO: recover auxiliary payloads before defining construction and destruction.
    explicit CPostConstructed(const CGameArea& area);
    ~CPostConstructed();
  };

  CGameArea(CInputStream& in, int index, int mlvlVersion);

  // IGameArea
  ~CGameArea() override;
  const CTransform4f& IGetTM() const override;
  CAssetId IGetStringTableAssetId() const override;
  uint IGetNumAttachedAreas() const override;
  TAreaId IGetAttachedAreaId(int index) const override;
  bool IIsActive() const override;
  CAssetId IGetAreaAssetId() const override;
  int IGetAreaSaveId() const override;
  rstl::string IGetInternalAreaName() const override;

  int GetPhase() const { return mPhase; }
  CAssetId GetAreaAssetId() const { return mAreaAssetId; }
  bool IsLoaded() const { return mPhase == kP_Loaded; }
  EOcclusionState GetOcclusionState() const { return mPostConstructed->mOcclusionState; }
  const CTransform4f& GetTM() const { return mTransform; }
  TAreaId GetId() const { return mSelfIdx; }
  const CAABox& GetAABB() const { return mBounds; }
  const Dock& GetDock(int index) const { return mDocks[index]; }

  void ClearTokenList();
  void VerifyTokenList(CStateManager& mgr);
  void PostConstructArea();
  void FillInStaticGeometry();
  void Validate(CStateManager& mgr);
  bool StartStreamIn(CStateManager& mgr);
  bool StartStreamingMainArea(CStateManager& mgr);
  bool Invalidate(CStateManager* mgr);
  char* AllocNewAreaData(int offset, int size);
  int VerifyHeader() const;
  int GetNumPartSizes() const;
  // Guessed names.
  int GetNumCompressedBlocks() const;
  uint CalculateDependencyListByteCount() const;
  int GetSectionIndex(int section) const;
  void CullDeadAreaRequests();
  void DecompressAreaData();
  void ClearDecompressionRequest(CDvdRequest* request);
  void ReadCompressedLayer(int offset, rstl::auto_ptr< CDvdRequest >& request,
                           rstl::auto_ptr< uchar >& buffer);
  void SortTextureDependencies();
  void SortRelTokens();
  bool UpdateDependencyLoading(CStateManager& mgr);
  void AddLayerTokens(int layer, rstl::vector< CToken >& tokens);
  void FinishDependencyLoading(CStateManager& mgr);
  void PrepareScriptObjects(CStateManager& mgr);
  bool LoadScriptObjects(CStateManager& mgr);
  void FinishScriptObjects(CStateManager& mgr);

  void SetAreaAttributes(CScriptAreaProperties* attributes);
  void SetLoadPauseState(bool paused);
  bool TryTakingOutOfARAM();
  bool TransferARAMTokensOver(EARAMTransfer mode);
  bool TransferTokensToARAM();
  bool UnloadAllloadedTextures();
  bool ReloadAllUnloadedTextures();
  int SetChain(CGameArea* next, int chain);
  void AddStaticGeometry();
  void RemoveStaticGeometry();
  void SetOcclusionState(EOcclusionState state);
  void PingOcclusionState();
  void OtherAreaOcclusionChanged();
  void PreRender();
  void AliveUpdate(float dt);
  bool IsFinishedOccluding() const;
  bool DoesAreaNeedSkyNow() const;
  int DoesAreaNeedEnvFx() const;
  void UpdateFog(float dt);
  void UpdateWeaponWorldLighting(float dt);
  void SetWeaponWorldLighting(float speed, float target);
  void SetXRaySpeedAndTarget(float speed, float target);
  uint Get1stPVSLightFeature(uint index) const;
  uint Get2ndPVSLightFeature(uint index) const;

  // Guessed names for the Echoes dynamic-layer interface.
  int GetTokenCount() const;
  ELayerPhase GetLayerPhase(const TLayerId& layer) const;
  const rstl::vector< CRELFileToken >* GetLayerRelTokens(const TLayerId& layer) const;
  bool IsValidLayerNumber(CStateManager& mgr, const TLayerId& layer) const;
  void LoadLayerDynamic(CStateManager& mgr, const TLayerId& layer);
  void UnloadLayerDynamic(CStateManager& mgr, const TLayerId& layer);
  void ActivateLayerDynamic(CStateManager& mgr, const TLayerId& layer);
  void StartLayerLoad(CStateManager& mgr, const TLayerId& layer);
  void RemoveLayerObjects(CStateManager& mgr, const TLayerId& layer);
  void ClearLayer(CStateManager& mgr, const TLayerId& layer);
  void LoadLayerRelModules(CStateManager& mgr, const TLayerId& layer);
  void UpdateLayerLoading(CStateManager& mgr, const TLayerId& layer);
  int GetLayerRequestCount(const TLayerId& layer) const;
  bool HasPendingLayerLoads() const;
  void UpdateDynamicLayers(CStateManager& mgr);
  int GetLayerScriptSize(const TLayerId& layer) const;
  rstl::pair< const uchar*, int > GetLayerScriptBuffer(const TLayerId& layer) const;
  rstl::pair< const uchar*, int > GetGeneratedScriptBuffer() const;
  void UpdateDocks(CStateManager& mgr);
  void InitializeDocks(CStateManager& mgr);
  void DisableDocks(CStateManager& mgr);
  void EnableDocks();
  void fn_80054F74();

private:
  TAreaId mSelfIdx;
  CAssetId mNameSTRG;
  CTransform4f mTransform;
  CAABox mBounds;
  CAssetId mAreaAssetId;
  int mAreaSaveId;
  rstl::vector< ushort > mAttachedAreaIndices;
  rstl::vector< rstl::pair< CAssetId, uint > > mDependencies1;
  rstl::vector< rstl::pair< CAssetId, uint > > mDependencies2;
  rstl::vector< uint > mLayerDependencyOffsets;
  rstl::vector< Dock > mDocks;
  rstl::vector< rstl::string > mRelModules;
  rstl::vector< uint > mRelOffsets;
  rstl::string mInternalAreaName;
  rstl::vector< ELayerPhase > mLayerPhases;
  uint mSerializedDependencySize;
  void* mDependenciesInAram;
  EPhase mPhase;
  CGameArea* mNext;
  CGameArea* mPrev;
  int mCurrentChain;
  rstl::single_ptr< CPostConstructed > mPostConstructed;
  bool mLoadPaused : 1;
  bool mValidationPaused : 1;
  bool mActive : 1;
  bool x108_3_ : 1;
};
CHECK_SIZEOF(CGameArea, 0x10c)
NESTED_CHECK_SIZEOF(CGameArea, CPostConstructed, 0x2d0)
NESTED_CHECK_SIZEOF(CGameArea, CAreaFog, 0x38)
NESTED_CHECK_SIZEOF(CGameArea, CAreaObjectList, 0x2014)

class CDummyGameArea : public IGameArea {
public:
  CDummyGameArea(CInputStream& in, int index, int mlvlVersion);

  // IGameArea
  const CTransform4f& IGetTM() const override;
  CAssetId IGetStringTableAssetId() const override;
  uint IGetNumAttachedAreas() const override;
  TAreaId IGetAttachedAreaId(int index) const override;
  bool IIsActive() const override;
  CAssetId IGetAreaAssetId() const override;
  int IGetAreaSaveId() const override;
  rstl::string IGetInternalAreaName() const override;

private:
  int mSelfIdx;
  CAssetId mNameSTRG;
  CAssetId mAreaAssetId;
  int mAreaSaveId;
  CTransform4f mTransform;
  rstl::vector< ushort > mAttachedAreaIndices;
  rstl::vector< Dock > mDocks;
  rstl::vector< rstl::string > mRelModules;
  rstl::vector< uint > mRelOffsets;
  rstl::string mInternalAreaName;
};
CHECK_SIZEOF(CDummyGameArea, 0x94)

#endif // _CGAMEAREA
