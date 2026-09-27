#ifndef _CANIMSOURCEREADERBASE
#define _CANIMSOURCEREADERBASE

#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/IAnimSourceInfo.hpp"
#include "rstl/object_owner.hpp"
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/vector.hpp"

class CAnimPOIData;

class CAnimSourceReaderBase : public IAnimReader {
public:
  CAnimSourceReaderBase(const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo,
                        const CAnimPOIData* poiData)
  : mSourceInfo(sourceInfo), mPOIData(poiData), mCurTime(0.f) {}

  CAnimSourceReaderBase(
      const rstl::ownership_transfer< IAnimSourceInfo >& sourceInfo, const CAnimPOIData* poiData,
      const CCharAnimTime& time, int passedBoolCount, int passedIntCount, int passedParticleCount,
      int passedSoundCount, const rstl::vector< rstl::pair< uint, bool > >& boolStates,
      const rstl::vector< rstl::pair< uint, int > >& intStates,
      const rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > >& particleStates)
  : mSourceInfo(sourceInfo)
  , mPOIData(poiData)
  , mCurTime(time)
  , mPassedBoolCount(passedBoolCount)
  , mPassedIntCount(passedIntCount)
  , mPassedParticleCount(passedParticleCount)
  , mPassedSoundCount(passedSoundCount)
  , mBoolStates(boolStates)
  , mInt32States(intStates)
  , mParticleStates(particleStates) {}

  // IAnimReader
  ~CAnimSourceReaderBase() override {}
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;

  void PostConstruct(const CCharAnimTime& time);
  void UpdatePOIStates();

protected:
  rstl::set< rstl::pair< uint, int > > GetUniqueBoolPOIs() const;
  rstl::set< rstl::pair< uint, int > > GetUniqueInt32POIs() const;
  rstl::set< rstl::pair< uint, int > > GetUniqueParticlePOIs() const;

  rstl::object_owner< IAnimSourceInfo > mSourceInfo;
  const CAnimPOIData* mPOIData;
  CCharAnimTime mCurTime;
  int mPassedBoolCount;
  int mPassedIntCount;
  int mPassedParticleCount;
  int mPassedSoundCount;
  rstl::vector< rstl::pair< uint, bool > > mBoolStates;
  rstl::vector< rstl::pair< uint, int > > mInt32States;
  rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > > mParticleStates;
};
CHECK_SIZEOF(CAnimSourceReaderBase, 0x58)

#endif // _CANIMSOURCEREADERBASE
