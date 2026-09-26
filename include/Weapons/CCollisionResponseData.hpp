#ifndef _CCOLLISIONRESPONSEDATA
#define _CCOLLISIONRESPONSEDATA

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CSimplePool;
class CGenDescription;
class CDecalDescription;

class CCollisionResponseData {
public:
  typedef rstl::vector< rstl::optional_object< TLockedToken< CGenDescription > > >
      GeneratorListType;
  typedef rstl::vector< int > SoundEffectIdListType;
  typedef rstl::vector< rstl::optional_object< TLockedToken< CDecalDescription > > > DecalListType;

  CCollisionResponseData(CInputStream& in, CSimplePool* pool);
  ~CCollisionResponseData();

  rstl::optional_object< TLockedToken< CGenDescription > >
  GetParticleDescription(EWeaponCollisionResponseTypes type) const;
  uint GetSoundEffectId(EWeaponCollisionResponseTypes type) const;
  rstl::optional_object< TLockedToken< CDecalDescription > >
  GetDecalDescription(EWeaponCollisionResponseTypes type) const;
  float GetAudibleRange() const;
  float GetAudibleFallOff() const;

private:
  GeneratorListType mGeneratorTokens;
  SoundEffectIdListType mSoundEffectIds;
  DecalListType mDecalTokens;
  float mAudibleRange;
  float mAudibleFallOff;
};
CHECK_SIZEOF(CCollisionResponseData, 0x38)

#endif // _CCOLLISIONRESPONSEDATA
