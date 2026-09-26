#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CSpawnSystemKeyframeData::CSpawnSystemKeyframeData(CInputStream& in)
: x0_(in.Get< int >())
, x4_(in.Get< int >())
, mEndFrame(in.Get< int >())
, xc_(in.Get< int >())
, mFrames(in) {}

void CSpawnSystemKeyframeData::LoadAllSpawnedSystemTokens(CSimplePool* pool) {
  for (int i = 0; i < mFrames.size(); ++i) {
    rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > >& frame = mFrames[i];
    for (int j = 0; j < frame.second.size(); ++j) {
      frame.second[j].LoadToken(pool);
    }
  }
}

CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::CSpawnSystemKeyframeInfo(CInputStream& in)
: mId(in.Get< uint >())
, mType(in.Get< uint >())
, x8_(in.Get< uint >())
, xc_(in.Get< uint >())
, mToken() {
  if (mType == 0) {
    mType = 'PART';
  }
}

rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >&
CSpawnSystemKeyframeData::GetSpawnedSystemsAtFrame(uint frame) {
  static rstl::vector< CSpawnSystemKeyframeInfo > emptyList =
      rstl::vector< CSpawnSystemKeyframeInfo >();
  if (frame >= mEndFrame) {
    return emptyList;
  }
  for (int i = 0; i < mFrames.size(); ++i) {
    rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > >& keyframe = mFrames[i];
    if (keyframe.first == frame) {
      return keyframe.second;
    }
  }
  return emptyList;
}

void CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::LoadToken(CSimplePool* pool) {
  mToken = pool->GetObj(SObjectTag(mType, mId));
  mToken->Lock();
}
