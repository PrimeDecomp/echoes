#ifndef _CWORLD
#define _CWORLD

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/IWorld.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CDvdRequest;
class CInputStream;
class CModel;
class CStateManager;
class CResFactory;
class IFactory;
class IObjectStore;

class CRelay {
public:
  explicit CRelay(CInputStream& in);
  const TEditorId& GetRelayId() const { return mRelay; }
  const TEditorId& GetTargetId() const { return mTarget; }
  const ushort& GetMessage() const { return mMsg; }
  bool GetActive() const { return mActive; }

private:
  TEditorId mRelay;
  TEditorId mTarget;
  ushort mMsg;
  bool mActive;
};
CHECK_SIZEOF(CRelay, 0xc)

class CWorld : public IWorld {
public:
  enum EChain {
    kC_Invalid = -1,
    kC_ToDeallocate,
    kC_Deallocated,
    kC_Loading,
    kC_Alive,
    kC_AliveJudgement,
  };

  enum EAreaTravelType { kATT_LoadAdjacent, kATT_SkipAdjacent };

  CWorld(IObjectStore& objStore, CResFactory& resFactory, CAssetId mlvlId);

  // IWorld
  ~CWorld() override;
  CAssetId IGetWorldAssetId() const override;
  CAssetId IGetStringTableAssetId() const override;
  CAssetId IGetDarkStringTableAssetId() const override;
  CAssetId IGetSaveWorldAssetId() const override;
  const CMapWorld* IGetMapWorld() const override;
  CMapWorld* IMapWorld() override;
  const IGameArea* IGetAreaAlways(TAreaId id) const override;
  TAreaId IGetCurrentAreaId() const override;
  TAreaId IGetAreaId(CAssetId id) const override;
  bool ICheckWorldComplete() override;
  rstl::string IGetDefaultAudioTrack() const override;
  int IGetAreaCount() const override;
  uint IGetTempleKeyWorldIndex() const override;
  bool ICancelLoad() override;

  bool CheckWorldComplete(CStateManager* mgr, TAreaId aid, CAssetId mreaId);
  void SetLoadPauseState(bool);
  void PauseAndUnpauseAreaLoading();
  void TouchSky() const;
  void DrawSky(const CTransform4f& xf, bool noFog) const;
  void StopSounds();
  bool ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr);
  void MoveToChain(CGameArea* area, EChain chain);
  void MoveAreaToChain3(TAreaId aid);
  void TravelToArea(const TAreaId& aid, CStateManager& mgr, EAreaTravelType travelType);
  bool fn_80050BC4(CStateManager& mgr, const TAreaId& aid);
  void fn_8004F6E8(const TAreaId& aid, const TLayerId& layer);
  void Update(float dt);
  void PreRender();
  CMapWorld* MapWorld() { return GetMapWorld(); }
  TAreaId GetAreaIdForSaveId(uint saveId) const;
  TAreaId GetAreaId(CAssetId assetId) const;
  bool AreSkyNeedsMet() const;
  void StopGlobalSound(ushort soundId);
  void AddGlobalSound(ushort soundId, CSfxHandle handle);
  bool HasGlobalSound(ushort soundId) const;

  CGameArea::CChainIterator ChainHead(EChain chain) const {
    return CGameArea::CChainIterator(mChainHeads[size_t(chain)]);
  }
  CGameArea::CConstChainIterator GetChainHead(EChain chain) const {
    return CGameArea::CConstChainIterator(mChainHeads[size_t(chain)]);
  }
  static CGameArea::CConstChainIterator skGlobalEnd;

  const CGameArea& GetAreaAlways(TAreaId id) const { return *mAreas[id.Value()]; }
  CGameArea* Area(TAreaId id) { return mAreas[id.Value()].get(); }
  const CGameArea* GetArea(TAreaId id) const { return mAreas[id.Value()].get(); }
  bool IsAreaValid(TAreaId id) const { return mAreas[id.Value()]->IsLoaded(); }
  bool DoesAreaExist(TAreaId id) const { return id.Value() >= 0 && id.Value() < mAreas.size(); }
  CAssetId GetWorldAssetId() const { return mMlvlId; }
  TAreaId GetCurrentAreaId() const { return mCurAreaId; }
  int GetNeededEnvFx() const { return mNeededEnvFx; }
  CMapWorld* GetMapWorld() const;


  static void PropogateAreaChain(CGameArea::EOcclusionState occlusionState, CGameArea* area,
                                 CWorld* world);

private:
  static CGameArea::CChainIterator skGlobalNonConstEnd;

  enum EPhase {
    kP_Loading,
    kP_LoadingMap,
    kP_LoadingMapAreas,
    kP_LoadingSkyBox,
    kP_Done,
  };

  // Guessed name
  struct SLayerRelUnload {
    TAreaId mAreaId;
    TLayerId mLayerId;
    uchar mFramesLeft;
  };

  EPhase mLoadPhase;
  CAssetId mMlvlId;
  CAssetId mStrgId;
  CAssetId mDarkStrgId;
  CAssetId mSavwId;
  rstl::vector< rstl::auto_ptr< CGameArea > > mAreas;
  CAssetId mMapwId;
  rstl::single_ptr< TCachedToken< CMapWorld > > mMapWorld;
  rstl::single_ptr< CDvdRequest > mLoadToken;
  rstl::single_ptr< char > mLoadBuf;
  uint mBufSize;
  rstl::reserved_vector< CGameArea*, 5 > mChainHeads;
  IObjectStore* mObjectStore;
  IFactory* mResFactory;
  TAreaId mCurAreaId;
  bool mCurrentAreaNeedsAllocation : 1;
  bool mLoadPaused : 1;
  bool mSkyboxActive : 1;
  bool mSkyboxVisible : 1;
  rstl::string mDefAudioTrack;
  rstl::optional_object< TCachedToken< CModel > > mSkyboxWorld;
  rstl::optional_object< TLockedToken< CModel > > mSkyboxWorldLoaded;
  rstl::optional_object< TLockedToken< CModel > > mSkyboxOverride;
  // Guessed names
  ERglFogMode mSkyboxFogMode;
  float mSkyboxFogStart;
  float mSkyboxFogEnd;
  CColor mSkyboxFogColor;
  int mNeededEnvFx;
  rstl::reserved_vector< rstl::pair< ushort, CSfxHandle >, 10 > mGlobalSfxHandles;
  rstl::list< SLayerRelUnload > mPendingLayerRelUnloads; // Guessed name
  float mSkyboxLightingLevel;                            // Guessed name
  uint mTempleKeyWorldIndex;                             // Guessed name
};
CHECK_SIZEOF(CWorld, 0x12c)

#endif // _CWORLD
