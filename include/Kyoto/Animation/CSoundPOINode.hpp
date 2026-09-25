#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(rstl::string name, ushort type, const CCharAnimTime& time, int index, bool unique,
                float weight, int charIdx, int flags, int sfxId, float fallOff, float maxDist);
  /* : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , x38_sfxId(sfxId)
  , x3c_falloff(fallOff)
  , x40_maxDist(maxDist) {}
  */

  CSoundPOINode(CInputStream& in);
  /*: CPOINode(in)
  , x38_sfxId(in.ReadInt32())
  , x3c_falloff(in.ReadFloat())
  , x40_maxDist(in.ReadFloat()) {}
  */

  uint GetSoundId() const { return mSfxId; }
  float GetFallOff() const { return mFalloff; }
  float GetMaxDistance() const { return mMaxDist; }

  static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
                                              const CCharAnimTime& startTime) {
    return CSoundPOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
                         node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                         node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                         node.GetFallOff(), node.GetMaxDistance());
  }

private:
  uint mSfxId;
  float mFalloff;
  float mMaxDist;
};

#endif // _CSOUNDPOINODE
