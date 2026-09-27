#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CSoundPOINode::CSoundPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index,
                             bool unique, float weight, int charIdx, int flags, int sfxId,
                             float fallOff, float maxDist, const CSegId& segId, ushort pitchStart,
                             ushort pitchEnd, float pitchDuration)
: CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags)
, mSfxId(sfxId)
, mFalloff(fallOff)
, mMaxDist(maxDist)
, mSegId(segId)
, mPitchStart(pitchStart)
, mPitchEnd(pitchEnd)
, mPitchDuration(pitchDuration) {}

CSoundPOINode::CSoundPOINode(CInputStream& in)
: CPOINode(in)
, mSfxId(in.Get< int >())
, mFalloff(in.Get< float >())
, mMaxDist(in.Get< float >())
, mSegId(static_cast< uchar >(0))
, mPitchStart(0)
, mPitchEnd(0)
, mPitchDuration(0.f) {
  if (mVersion > skExtendedVersion) {
    mSegId = CSegId(in);
    mPitchStart = in.Get< ushort >();
    mPitchEnd = in.Get< ushort >();
    mPitchDuration = in.Get< float >();
  }
}

CSoundPOINode CSoundPOINode::CopyNodeMinusStartTime(const CSoundPOINode& node,
                                                    const CCharAnimTime& startTime) {
  return CSoundPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                       node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                       node.GetFallOff(), node.GetMaxDistance(), node.mSegId, node.mPitchStart,
                       node.mPitchEnd, node.mPitchDuration);
}
