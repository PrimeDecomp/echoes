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
  class CParticleResData {
  private:
    rstl::vector< CAssetId > mPart;
    rstl::vector< CAssetId > mSwhc;
    rstl::vector< CAssetId > mElsc0;
    rstl::vector< CAssetId > mElsc1;
  };

  const CPASDatabase& GetPASDatabase() const { return mPasDatabase; }
  const rstl::vector< rstl::pair< rstl::string, CAABox > >& GetAnimBBoxList() const { return mAabbs; }

private:
  ushort mTableCount;
  rstl::string mName;
  CAssetId mCmdl;
  CAssetId mCksr;
  CAssetId mCinf;
  rstl::vector< rstl::pair< int, rstl::pair< rstl::string, rstl::string > > > mAnimInfo;
  CPASDatabase mPasDatabase;
  CParticleResData mPartRes;
  uint mUnk;
  rstl::vector< rstl::pair< rstl::string, CAABox > > mAabbs;
  rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > mEffects;
  uint mCmdlOverlay;
  uint mCksrOverlay;
  rstl::vector< int > mAnimIdxs;
};
CHECK_SIZEOF(CCharacterInfo, 0xc0)

#endif // _CCHARACTERINFO
