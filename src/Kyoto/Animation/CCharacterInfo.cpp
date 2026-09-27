#include "Kyoto/Animation/CCharacterInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CCharacterInfo::CParticleResData::CParticleResData(CInputStream& in, ushort tableCount)
: mPart(in), mSwhc(in), mElscB(in) {
  if (tableCount > 5) {
    mElscA = rstl::vector< CAssetId >(in);
  }
  if (tableCount > 8) {
    mSpsc = rstl::vector< CAssetId >(in);
    mSrsc = rstl::vector< CAssetId >(in);
  }
}

CCharacterInfo::CCharacterInfo(CInputStream& in)
: mTableCount(in.Get< ushort >())
, mName(in)
, mCmdl(in.Get< CAssetId >())
, mCksr(in.Get< CAssetId >())
, mCinf(in.Get< CAssetId >())
, mAnimInfo(in)
, mPasDatabase(in.Get< CPASDatabase >())
, mPartRes(in, mTableCount)
, xa4_(in.Get< uint >())
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
    mAnimIdxs = rstl::vector< int >(in);
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
