#include "Kyoto/Animation/CFBStreamedCompression.hpp"

#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

rstl::auto_ptr< uint > CFBStreamedCompression::GetRotationsAndOffsets(uint words,
                                                                      CInputStream& in) {
  rstl::auto_ptr< uint > data(rs_new uint[words]);
  CStandardMultiFormatHeader* header = reinterpret_cast< CStandardMultiFormatHeader* >(data.get());
  new (header) CStandardMultiFormatHeader(in);
  CFBStreamedCompressionTimeHeader* timeHeader =
      static_cast< CFBStreamedCompressionTimeHeader* >(const_cast< void* >(header->AfterEnd()));
  new (timeHeader) CFBStreamedCompressionTimeHeader(in);
  CFBStreamedPerChannelHeaderList* channels =
      static_cast< CFBStreamedPerChannelHeaderList* >(const_cast< void* >(timeHeader->AfterEnd()));
  new (channels) CFBStreamedPerChannelHeaderList(in);
  const uint wordCount = static_cast< uint >(
      static_cast< float >(channels->GetSumOfBitCounts() *
                               channels->begin()->GetRotationBitStorage().GetWidth() +
                           31) /
      32.f);
  uchar* cursor = const_cast< uchar* >(channels->AfterEnd());
  for (uint i = 0; i < wordCount; ++i) {
    TLoadedVal< uint >::Write(cursor, in.ReadInt32());
    cursor += sizeof(uint);
  }
  return data;
}

CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore&)
: mScratchSize(in.ReadInt32())
, x4_(in.ReadInt8())
, mRotsAndOffs(GetRotationsAndOffsets(mScratchSize / 4 + 1, in).release())
, mRootOffset(CVector3f::Zero()) {
  {
    const CFBStreamedPerChannelHeaderList& channels =
        GetPerChannelHeaderList(TimeHeader(MainHeader()));
    CMemoryInputToBitLevelLoader input(GetBytes(channels));
    CBitLevelLoader< CMemoryInputToBitLevelLoader > loader(input);
    uint rootIndex = 0;
    for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin();
         it != channels.end(); ++it) {
      if (it->GetSegId() == CSegId(0)) {
        break;
      }
      ++rootIndex;
    }
    CFBStreamedAnimReaderTotals totals(*this);
    totals.CalculateDown();
    CVector3f previous = totals.GetVector(rootIndex);
    float distance = 0.f;
    const uint keyframes = GetNumKeyframes();
    for (uint i = 0; i < keyframes; ++i) {
      totals.IncrementInto(loader, *this, totals);
      totals.CalculateDown();
      const CVector3f current = totals.GetVector(rootIndex);
      const float delta = (current - previous).Magnitude();
      previous = current;
      if (!close_enough(delta, 0.f)) {
        distance += delta;
      }
    }
    mAverageVelocity = distance / GetAnimationDuration().GetSeconds();
  }
  CCharAnimMemoryMetrics::AddToTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
}

CFBStreamedCompression::~CFBStreamedCompression() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(mScratchSize, CCharAnimMemoryMetrics::kASS_Two);
}
