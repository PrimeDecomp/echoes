#ifndef _CANIMSOURCEREADER
#define _CANIMSOURCEREADER

#include "Kyoto/Animation/CAnimSource.hpp"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"

class CAnimSourceInfo : public IAnimSourceInfo {
public:
  explicit CAnimSourceInfo(const TSubAnimTypeToken< CAnimSource >& source) : mSource(source) {}

  // IAnimSourceInfo
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }

  bool HasScaleData() const override { return mSource->HasScaleData(); }

  ~CAnimSourceInfo() override {}

private:
  TSubAnimTypeToken< CAnimSource > mSource;
};
CHECK_SIZEOF(CAnimSourceInfo, 0x14)

class CAnimSourceReader : public CAnimSourceReaderBase {
public:
  CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source, const CCharAnimTime& time,
                    const CAnimPOIData* poiData);

  // IAnimReader
  ~CAnimSourceReader() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                           const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                   const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  virtual CVector3f VGetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  virtual bool VSupportsReverseView() const;
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time);

private:
  CAnimSourceReader(
      const TSubAnimTypeToken< CAnimSource >& source, const CAnimPOIData* poiData,
      const CCharAnimTime& time, const CSteadyStateAnimInfo& steadyStateInfo, int passedBoolCount,
      int passedIntCount, int passedParticleCount, int passedSoundCount,
      const rstl::vector< rstl::pair< uint, bool > >& boolStates,
      const rstl::vector< rstl::pair< uint, int > >& intStates,
      const rstl::vector< rstl::pair< uint, CParticleData::EParentedMode > >& particleStates)
  : CAnimSourceReaderBase(rs_new CAnimSourceInfo(source), poiData, time, passedBoolCount,
                          passedIntCount, passedParticleCount, passedSoundCount, boolStates,
                          intStates, particleStates)
  , mSource(source)
  , mSteadyStateInfo(steadyStateInfo) {}

  TSubAnimTypeToken< CAnimSource > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
};
CHECK_SIZEOF(CAnimSourceReader, 0x80)

#endif // _CANIMSOURCEREADER
