#ifndef _IANIMREADER
#define _IANIMREADER

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"

#include "rstl/string.hpp"

struct SAdvancementDeltas {
  CVector3f mPosDelta;
  CQuaternion mRotDelta;

  static SAdvancementDeltas Interpolate(const SAdvancementDeltas& a, const SAdvancementDeltas& b, float oldWeight,
                                        float newWeight);
  static SAdvancementDeltas Blend(const SAdvancementDeltas& a, const SAdvancementDeltas& b, float w);
};

struct SAdvancementResults {
  CCharAnimTime mRemTime;
  SAdvancementDeltas mDeltas;
};


class CSteadyStateAnimInfo {
  CCharAnimTime mDuration;
  CVector3f mOffset;
  bool mLooping;

public:
  CSteadyStateAnimInfo(bool looping, const CCharAnimTime& duration, const CVector3f& offset)
  : mDuration(duration), mOffset(offset), mLooping(looping) {}

  const CCharAnimTime& GetDuration() const { return mDuration; }
  const CVector3f& GetOffset() const { return mOffset; }
  bool IsLooping() const { return mLooping; }
};

struct CAnimTreeEffectiveContribution {
  float mContributionWeight;
  rstl::string mName;
  CSteadyStateAnimInfo mSsInfo;
  CCharAnimTime mRemTime;
  u32 mDbIdx;

public:
  CAnimTreeEffectiveContribution(float cweight, const rstl::string& name, const CSteadyStateAnimInfo& ssInfo,
                                 const CCharAnimTime& remTime, u32 dbIdx)
  : mContributionWeight(cweight), mName(name), mSsInfo(ssInfo), mRemTime(remTime), mDbIdx(dbIdx) {}
  float GetContributionWeight() const { return mContributionWeight; }
  const rstl::string& GetPrimitiveName() const { return mName; }
  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSsInfo; }
  const CCharAnimTime& GetTimeRemaining() const { return mRemTime; }
  u32 GetAnimDatabaseIndex() const { return mDbIdx; }
};

class CSegId;
class CSegIdList;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegStatementSet;

class IAnimReader {
public:
  virtual ~IAnimReader();
  virtual bool IsCAnimTreeNode() const { return false; }
  virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a) = 0;
  virtual CCharAnimTime VGetTimeRemaining() const = 0;
  virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const = 0;
  virtual bool VHasOffset(const CSegId& seg) const = 0;
  virtual CVector3f VGetOffset(const CSegId& seg) const = 0;
  virtual CQuaternion VGetRotation(const CSegId& seg) const = 0;
  virtual size_t VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity, size_t iterator,
                                 u32) const = 0;
  virtual size_t VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity, size_t iterator,
                                  u32) const = 0;
  virtual size_t VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                     size_t iterator, u32) const = 0;
  virtual size_t VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity, size_t iterator,
                                  u32) const = 0;
  virtual bool VGetBoolPOIState(const rstl::string& name) const = 0;
  virtual s32 VGetInt32POIState(const rstl::string& name) const = 0;
  // virtual CParticleData::EParentedMode VGetParticlePOIState(const rstl::string& name) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const = 0;
  virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                   const CCharAnimTime& time) const = 0;
  // virtual std::unique_ptr<IAnimReader> VClone() const = 0;
};


#endif // _IANIMREADER
