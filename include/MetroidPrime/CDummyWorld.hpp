#ifndef _CDUMMYWORLD
#define _CDUMMYWORLD

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/IWorld.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CDummyGameArea;
class CDvdRequest;

class CDummyWorld : public IWorld {
public:
  CDummyWorld(CAssetId mlvlId, bool loadMap);

  // IWorld
  ~CDummyWorld() override;
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

private:
  enum EPhase { kP_Loading, kP_LoadingMap, kP_LoadingMapAreas, kP_Done };

  bool mLoadMap;
  EPhase mPhase;
  CAssetId mMlvlId;
  CAssetId mStrgId;
  CAssetId mDarkStrgId;
  CAssetId mSavwId;
  rstl::vector< rstl::auto_ptr< CDummyGameArea > > mAreas;
  CAssetId mMapWorldId;
  rstl::single_ptr< TCachedToken< CMapWorld > > mMapWorld;
  rstl::single_ptr< CDvdRequest > mLoadToken;
  rstl::single_ptr< char > mLoadBuf;
  uint mBufSize;
  TAreaId mCurrentAreaId;
  uint mTempleKeyWorldIndex;
};
CHECK_SIZEOF(CDummyWorld, 0x48)

#endif // _CDUMMYWORLD
