#ifndef _CCECHARACTERINFO
#define _CCECHARACTERINFO

#include "types.h"

#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"

#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCECharacterInfo {
public:
  typedef rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > > TEffectList;
  class CParticleResData {
  public:
    CParticleResData(CInputStream& in, ushort tableCount);
    CParticleResData(const rstl::vector< CAssetId >& part, const rstl::vector< CAssetId >& swhc,
                     const rstl::vector< CAssetId >& elscA, const rstl::vector< CAssetId >& spsc,
                     const rstl::vector< CAssetId >& srsc, const rstl::vector< CAssetId >& elscB)
    : mPart(part), mSwhc(swhc), mElscA(elscA), mSpsc(spsc), mSrsc(srsc), mElscB(elscB) {}
    const rstl::vector< CAssetId >& GetParts() const { return mPart; }
    const rstl::vector< CAssetId >& GetSwooshes() const { return mSwhc; }
    const rstl::vector< CAssetId >& GetElectrics() const { return mElscA; }
    const rstl::vector< CAssetId >& GetSpawnSystems() const { return mSpsc; }
    const rstl::vector< CAssetId >& GetSortedSystems() const { return mSrsc; }

  private:
    rstl::vector< CAssetId > mPart;
    rstl::vector< CAssetId > mSwhc;
    rstl::vector< CAssetId > mElscA;
    rstl::vector< CAssetId > mSpsc;
    rstl::vector< CAssetId > mSrsc;
    rstl::vector< CAssetId > mElscB;
  };

  explicit CCECharacterInfo(CInputStream& in);

  CAssetId GetModelId() const { return mCmdl; }
  CAssetId GetSkinRulesId() const { return mCksr; }
  CAssetId GetCharLayoutInfoId() const { return mCinf; }
  CAssetId GetIceModelId() const { return mCmdlOverlay; }
  CAssetId GetIceSkinRulesId() const { return mCksrOverlay; }
  CAssetId GetSpatialPrimitiveId() const { return mSpatialPrimitiveId; }
  bool GetAnimatedScale() const { return mAnimatedScale; }
  uint GetDefaultAnimation() const { return mDefaultAnimation; }
  const CPASDatabase& GetPASDatabase() const { return mPasDatabase; }
  const CParticleResData& GetParticleResData() const { return mPartRes; }
  const TEffectList& GetEffects() const { return mEffects; }
  const rstl::vector< uint >& GetAnimationIndexList() const { return mAnimIdxs; }
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
  rstl::vector< rstl::pair< int, rstl::string > > mAnimInfo;
  CPASDatabase mPasDatabase;
  CParticleResData mPartRes;
  uint mDefaultAnimation;
  rstl::vector< rstl::pair< rstl::string, CAABox > > mAabbs;
  TEffectList mEffects;
  CAssetId mCmdlOverlay;
  CAssetId mCksrOverlay;
  rstl::vector< uint > mAnimIdxs;
  CAssetId mSpatialPrimitiveId; // Guessed name: CSPP resource.
  bool mAnimatedScale;          // Guessed name.
  rstl::vector< rstl::pair< uint, CAABox > > mAnimBoundsById;
};
CHECK_SIZEOF(CCECharacterInfo, 0xf8)
NESTED_CHECK_SIZEOF(CCECharacterInfo, CParticleResData, 0x60)

#endif // _CCECHARACTERINFO
