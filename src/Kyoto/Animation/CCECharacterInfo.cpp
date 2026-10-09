#include "Kyoto/Animation/CCECharacterInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CCECharacterInfo::CParticleResData::CParticleResData(CInputStream& in, ushort tableCount)
: mPart(in), mSwhc(in), mElscB(in) {
  if (tableCount > 5) {
    const rstl::vector< CAssetId > resources(in);
    mElscA = rstl::vector< CAssetId >(resources.begin(), resources.end());
  }

  if (tableCount > 8) {
    const rstl::vector< CAssetId > spawnResources(in);
    mSpsc = rstl::vector< CAssetId >(spawnResources.begin(), spawnResources.end());
    const rstl::vector< CAssetId > sortedResources(in);
    mSrsc = rstl::vector< CAssetId >(sortedResources.begin(), sortedResources.end());
  }
}

CCECharacterInfo::CCECharacterInfo(CInputStream& in)
: mTableCount(in.Get< ushort >())
, mName(in)
, mCmdl(in.Get< CAssetId >())
, mCksr(in.Get< CAssetId >())
, mCinf(in.Get< CAssetId >())
, mAnimInfo(in)
, mPasDatabase(in.Get(TGetType(mPasDatabase)))
, mPartRes(in, mTableCount)
, mDefaultAnimation(in.Get< uint >())
, mCmdlOverlay(kInvalidAssetId)
, mCksrOverlay(kInvalidAssetId)
, mSpatialPrimitiveId(kInvalidAssetId)
, mAnimatedScale(false) {
  if (mTableCount > 1) {
    mAabbs = rstl::vector< rstl::pair< rstl::string, CAABox > >(in);
  }
  if (mTableCount > 2) {
    mEffects = TEffectList(in);
  }
  if (mTableCount > 3) {
    mCmdlOverlay = in.Get< CAssetId >();
    mCksrOverlay = in.Get< CAssetId >();
  } else {
    mCmdlOverlay = 0;
    mCksrOverlay = 0;
  }
  if (mTableCount > 4) {
    mAnimIdxs = rstl::vector< uint >(in);
  }
  if (mTableCount > 6) {
    mSpatialPrimitiveId = in.Get< CAssetId >();
  }
  if (mTableCount > 7) {
    mAnimatedScale = in.Get< bool >();
  }
  if (mTableCount > 9) {
    mAnimBoundsById = rstl::vector< rstl::pair< uint, CAABox > >(in);
  }
}
