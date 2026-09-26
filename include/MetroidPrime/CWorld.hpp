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
class CModel;
class CResFactory;
class IFactory;
class IObjectStore;

class CWorld : public IWorld {
public:
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

  const CGameArea& GetAreaAlways(TAreaId id) const { return *mAreas[id.Value()]; }
  CGameArea* Area(TAreaId id) { return mAreas[id.Value()].get(); }
  const CGameArea* GetArea(TAreaId id) const { return mAreas[id.Value()].get(); }
  bool IsAreaValid(TAreaId id) const { return mAreas[id.Value()]->IsLoaded(); }
  CAssetId GetWorldAssetId() const { return mMlvlId; }
  TAreaId GetCurrentAreaId() const { return mCurAreaId; }
  CMapWorld* GetMapWorld() const;

  void SetLoadPauseState(bool);

  static void PropogateAreaChain(CGameArea::EOcclusionState occlusionState, CGameArea* area,
                                 CWorld* world);

private:
  // Guessed name; recover the complete payload before defining world cleanup.
  struct SLayerRelUnload;

  uint mLoadPhase;
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
