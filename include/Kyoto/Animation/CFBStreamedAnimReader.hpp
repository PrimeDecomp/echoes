#ifndef _CFBSTREAMEDANIMREADER
#define _CFBSTREAMEDANIMREADER

#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/CTimeRemainderAndFraction.hpp"
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#include "rstl/math.hpp"

class CMemoryInputToBitLevelLoader;
class CSegStatement;
template < typename T >
class CBitLevelLoader;

template < typename T >
class TAnimSourceInfo : public IAnimSourceInfo {
public:
  explicit TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : mSource(source) {}

  // IAnimSourceInfo
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }
  bool HasScaleData() const override { return mSource->HasScaleData(); }
  ~TAnimSourceInfo() override {}

private:
  TSubAnimTypeToken< T > mSource;
};

class CFBStreamedAnimReaderTotals {
public:
  explicit CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
  ~CFBStreamedAnimReaderTotals();
  void CalculateDown();
  void SetToReadStart(const CFBStreamedCompression& source);
  void IncrementInto(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                     const CFBStreamedCompression& source, CFBStreamedAnimReaderTotals& out);
  uint GetFrameNumber() const { return mCurKey; }
  uint NumEntries() const { return mBoneChanCount; }
  uint GetSegId(uint index) const { return mSegIds[index]; }
  bool HasRotation(uint index) const { return mHasRotation[index]; }
  bool HasOffset(uint index) const { return mHasOffset[index]; }
  bool HasScale(uint index) const { return mHasScale[index]; }
  bool HasOffsetData() const { return mHasOffsetData; }
  bool HasScaleData() const { return mHasScaleData; }
  bool AmCalculatedDown() const { return mCalculated; }
  const CQuaternion& GetQuat(uint index) const {
    return *reinterpret_cast< const CQuaternion* >(mComputedFloats + index * mValuesPerChannel);
  }
  const CVector3f& GetVector(uint index) const {
    return *reinterpret_cast< const CVector3f* >(mComputedFloats + index * mValuesPerChannel + 4);
  }
  const CVector3f& GetScale(uint index) const {
    uint offset = index * mValuesPerChannel + 4 + (mHasOffsetData ? 4 : 0);
    return *reinterpret_cast< const CVector3f* >(mComputedFloats + offset);
  }

private:
  void Allocate(uint channelCount);
  uchar GetValuesPerChannel() const; // Guessed name.

  uchar* mBuffer;
  uint mBufferSize;
  short* mCumulativeInts;
  bool* mHasRotation;
  bool* mHasOffset;
  bool* mHasScale;
  short* mSegIds;
  float* mComputedFloats;
  uint mRotDiv;
  float mOffsetMult;
  float mScaleMult;
  uint mCurKey;
  bool mCalculated;
  uint mBoneChanCount;
  bool mHasOffsetData;
  bool mHasScaleData;
  uchar mValuesPerChannel;
};
CHECK_SIZEOF(CFBStreamedAnimReaderTotals, 0x3c)

class CFBFullBodyAspectsForStream {
public:
  CFBFullBodyAspectsForStream(const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
                              const CTimeRemainderAndFraction& time, const CCharAnimTime& duration);
  void SetTime(const CTimeRemainderAndFraction& time);
  uint GetPrevIndex() const { return mPriorKey; }
  float GetT() const { return mT; }

private:
  const CFBKeyFrameReductionPerChannel_HeaderForAll* mHeader;
  uint mPriorFrame;
  uint mNextFrame;
  uint mLastFrame;
  float mSampleTime;
  float mT;
  uint mPriorKey;
  uint mNextKey;
};
CHECK_SIZEOF(CFBFullBodyAspectsForStream, 0x20)

class CFBStreamedPairOfTotals {
public:
  explicit CFBStreamedPairOfTotals(const TSubAnimTypeToken< CFBStreamedCompression >& source);
  void SetTime(CMemoryInputToBitLevelLoader& input,
               CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CCharAnimTime& time);
  void DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader);
  float GetT() const;
  CFBStreamedAnimReaderTotals& Prior() { return mNextSel ? mA : mB; }
  CFBStreamedAnimReaderTotals& Next() { return mNextSel ? mB : mA; }

private:
  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  bool mNextSel;
  CFBStreamedAnimReaderTotals mA;
  CFBStreamedAnimReaderTotals mB;
  CFBFullBodyAspectsForStream mAspects;
  uint mCurKey;
};
CHECK_SIZEOF(CFBStreamedPairOfTotals, 0xb0)

class CMemoryInputToBitLevelLoader {
  friend class CBitLevelLoader< CMemoryInputToBitLevelLoader >;

public:
  explicit CMemoryInputToBitLevelLoader(const uint* data)
  : mData(reinterpret_cast< const uchar* >(data) - sizeof(uint)) {}

private:
  const uchar* mData;
};
CHECK_SIZEOF(CMemoryInputToBitLevelLoader, 0x4)

template < typename T >
class CBitLevelLoader {
public:
  explicit CBitLevelLoader(T& input) : mInput(&input), mWord(Input(*mInput)), mBit(0) {}
  uint LoadUnsigned(uint bits) {
    uint result = 0;
    uint shift = 0;
    while (bits != 0) {
      uint count = rstl::min_val(32 - mBit, bits);
      uint highShift = 32 - count;
      result |= ((mWord >> mBit) << highShift) >> (highShift - shift);
      mBit += count;
      shift += count;
      bits -= count;
      if (mBit == 32) {
        mBit = 0;
        mWord = Input(*mInput);
      }
    }
    return result;
  }
  int LoadSigned(uint bits) {
    if (bits == 0) {
      return 0;
    }
    uint value = LoadUnsigned(bits);
    if (value & (1u << (bits - 1))) {
      value |= ~0u << bits;
    }
    return value;
  }

private:
  static uint Input(T& input);
  T* mInput;
  uint mWord;
  uint mBit;
};

template <>
inline uint
CBitLevelLoader< CMemoryInputToBitLevelLoader >::Input(CMemoryInputToBitLevelLoader& input) {
  input.mData += sizeof(uint);
  return TLoadedVal< uint >::Read(input.mData);
}

class CSegIdToIndexConverter {
public:
  explicit CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals) {
    for (uint i = 0; i < 100; ++i) {
      mIndices[i] = ~0u;
    }
    for (uint i = 0; i < totals.NumEntries(); ++i) {
      mIndices[totals.GetSegId(i)] = i;
    }
    CCharAnimMemoryMetrics::AddToTotalSize(sizeof(mIndices), CCharAnimMemoryMetrics::kASS_Two);
  }
  ~CSegIdToIndexConverter() {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(sizeof(mIndices),
                                                  CCharAnimMemoryMetrics::kASS_Two);
  }
  uint SegIdToIndex(uint seg) const { return mIndices[seg]; }

private:
  uint mIndices[100];
};
CHECK_SIZEOF(CSegIdToIndexConverter, 0x190)

class CFBStreamedAnimReader : public CAnimSourceReaderBase {
public:
  CFBStreamedAnimReader(const TSubAnimTypeToken< CFBStreamedCompression >& source,
                        CCharAnimTime time, const CAnimPOIData* poiData);

  // IAnimReader
  ~CFBStreamedAnimReader() override;
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

  virtual bool VSupportsReverseView() const;
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time);

private:
  void SetReadTime(const CCharAnimTime& time) const;
  void GetSegStatement(CSegStatement& statement, const CSegId& seg) const;

  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
  mutable CFBStreamedPairOfTotals mTotals;
  mutable CMemoryInputToBitLevelLoader mInput;
  mutable CBitLevelLoader< CMemoryInputToBitLevelLoader > mBitLoader;
  CSegIdToIndexConverter mSegIdToIndex;
};
CHECK_SIZEOF(CFBStreamedAnimReader, 0x2d0)

#endif // _CFBSTREAMEDANIMREADER
