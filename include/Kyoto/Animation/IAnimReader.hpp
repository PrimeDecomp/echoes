#ifndef _IANIMREADER
#define _IANIMREADER

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/ownership_transfer.hpp"
#include "rstl/string.hpp"

struct SAdvancementDeltas {
  CVector3f mPosDelta;
  CQuaternion mRotDelta;

  static SAdvancementDeltas Interpolate(const SAdvancementDeltas& a, const SAdvancementDeltas& b,
                                        float oldWeight, float newWeight);
  static SAdvancementDeltas Blend(const SAdvancementDeltas& a, const SAdvancementDeltas& b,
                                  float w);
};

struct SAdvancementResults {
  CCharAnimTime mRemTime;
  SAdvancementDeltas mDeltas;

  SAdvancementResults() {}
  explicit SAdvancementResults(const CCharAnimTime& time) : mRemTime(time) {
    mDeltas.mPosDelta = CVector3f::Zero();
    mDeltas.mRotDelta = CQuaternion::NoRotation();
  }
};
CHECK_SIZEOF(SAdvancementResults, 0x24)

class CSteadyStateAnimInfo {
  CCharAnimTime mDuration;
  CVector3f mOffset;
  bool mLooping;

public:
  CSteadyStateAnimInfo(bool looping, const CCharAnimTime& duration, const CVector3f& offset)
  : mDuration(duration), mOffset(offset), mLooping(looping) {}

  CCharAnimTime GetDuration() const { return mDuration; }
  CVector3f GetOffset() const { return mOffset; }
  const bool IsLooping() const { return mLooping; }
};
CHECK_SIZEOF(CSteadyStateAnimInfo, 0x18)

struct CAnimTreeEffectiveContribution {
  float mContributionWeight;
  rstl::string mName;
  CSteadyStateAnimInfo mSsInfo;
  CCharAnimTime mRemTime;
  u32 mDbIdx;

public:
  CAnimTreeEffectiveContribution(float cweight, const rstl::string& name,
                                 const CSteadyStateAnimInfo& ssInfo, const CCharAnimTime& remTime,
                                 u32 dbIdx)
  : mContributionWeight(cweight), mName(name), mSsInfo(ssInfo), mRemTime(remTime), mDbIdx(dbIdx) {}
  float GetContributionWeight() const { return mContributionWeight; }
  const rstl::string& GetPrimitiveName() const { return mName; }
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSsInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return mRemTime; }
  u32 GetAnimDatabaseIndex() const { return mDbIdx; }
  float GetPhase() const {
    return rstl::min_val(rstl::max_val(1.f - mRemTime / mSsInfo.GetDuration(), 0.f), 1.f);
  }
};

class CSegId;
class CSegIdList;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegStatementSet;
class CCharLayoutInfo;
class CJointData_LinearStorage;

class IAnimReader {
public:
  virtual ~IAnimReader();
  virtual bool IsCAnimTreeNode() const;
  virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                               uint iterator, int additive) const = 0;
  virtual uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                uint iterator, int additive) const = 0;
  virtual uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                   uint capacity, uint iterator, int additive) const = 0;
  virtual uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                                uint iterator, int additive) const = 0;
  virtual bool VGetBoolPOIState(uint nameHash) const = 0;
  virtual s32 VGetInt32POIState(uint nameHash) const = 0;
  virtual CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
  // Guessed names.
  virtual void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                           const CCharAnimTime& time) const;
  virtual void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data) const;
  virtual rstl::ownership_transfer< IAnimReader > VClone() const = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified();
  virtual void VSetPhase(float phase) = 0;
  virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                                     const CCharAnimTime& startOffset) const;

  CCharAnimTime GetTimeRemaining() const { return VGetTimeRemaining(); }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const { return VGetSteadyStateAnimInfo(); }

  uint GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                      uint iterator, int additive) const;
  uint GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                       uint iterator, int additive) const;
  uint GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                          uint iterator, int additive) const;
  uint GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                       uint iterator, int additive) const;
};
CHECK_SIZEOF(IAnimReader, 0x4)

#endif // _IANIMREADER
