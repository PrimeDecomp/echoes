#ifndef _CCHARACTERINFO
#define _CCHARACTERINFO

#include "types.h"

#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"

#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCharacterInfo {
public:
  typedef rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > TEffectList;
  class CParticleResData {
  public:
    CParticleResData(CInputStream& in, ushort tableCount);

  private:
    rstl::vector< CAssetId > mPart;
    rstl::vector< CAssetId > mSwhc;
    rstl::vector< CAssetId > mElscA;
    rstl::vector< CAssetId > mSpsc;
    rstl::vector< CAssetId > mSrsc;
    rstl::vector< CAssetId > mElscB;
  };

  explicit CCharacterInfo(CInputStream& in);

  const CPASDatabase& GetPASDatabase() const { return mPasDatabase; }
  const CParticleResData& GetParticleResData() const { return mPartRes; }
  const TEffectList& GetEffects() const { return mEffects; }
  const rstl::vector< int >& GetAnimationIndexList() const { return mAnimIdxs; }
  const rstl::vector< rstl::pair< rstl::string, CAABox > >& GetAnimBBoxList() const {
    return mAabbs;
  }
  const rstl::vector< rstl::pair< uint, CAABox > >& GetAnimBoundsById() const {
    return mAnimBoundsById;
  }

private:
  ushort mTableCount;
  rstl::string mName;
  CAssetId mCmdl;
  CAssetId mCksr;
  CAssetId mCinf;
  rstl::vector< rstl::pair< int, rstl::pair< rstl::string, rstl::string > > > mAnimInfo;
  CPASDatabase mPasDatabase;
  CParticleResData mPartRes;
  uint xa4_;
  rstl::vector< rstl::pair< rstl::string, CAABox > > mAabbs;
  TEffectList mEffects;
  CAssetId mCmdlOverlay;
  CAssetId mCksrOverlay;
  rstl::vector< int > mAnimIdxs;
  CAssetId mSpatialPrimitiveId; // Guessed name: CSPP resource.
  bool mAnimatedScale;          // Guessed name.
  rstl::vector< rstl::pair< uint, CAABox > > mAnimBoundsById;
};
CHECK_SIZEOF(CCharacterInfo, 0xf8)
NESTED_CHECK_SIZEOF(CCharacterInfo, CParticleResData, 0x60)

#endif // _CCHARACTERINFO
