#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CSoundPOINode::CSoundPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index,
                             bool unique, float weight, int charIdx, int flags, int sfxId,
                             float fallOff, float maxDist, const CSegId& segId, ushort x3a,
                             ushort x3c, float x40)
: CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags)
, mSfxId(sfxId)
, mFalloff(fallOff)
, mMaxDist(maxDist)
, mSegId(segId)
, x3a_(x3a)
, x3c_(x3c)
, x40_(x40) {}

CSoundPOINode::CSoundPOINode(CInputStream& in)
: CPOINode(in)
, mSfxId(in.Get< int >())
, mFalloff(in.Get< float >())
, mMaxDist(in.Get< float >())
, mSegId(static_cast< uchar >(0))
, x3a_(0)
, x3c_(0)
, x40_(0.f) {
  if (mVersion > skExtendedVersion) {
    mSegId = CSegId(in);
    x3a_ = in.Get< ushort >();
    x3c_ = in.Get< ushort >();
    x40_ = in.Get< float >();
  }
}

CSoundPOINode CSoundPOINode::CopyNodeMinusStartTime(const CSoundPOINode& node,
                                                    const CCharAnimTime& startTime) {
  return CSoundPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                       node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                       node.GetFallOff(), node.GetMaxDistance(), node.mSegId, node.x3a_,
                       node.x3c_, node.x40_);
}
