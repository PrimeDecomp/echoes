#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Animation/CSegId.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
                float weight, int charIdx, int flags, int sfxId, float fallOff, float maxDist,
                const CSegId& segId, ushort x3a, ushort x3c, float x40);
  CSoundPOINode(CInputStream& in);

  uint GetSoundId() const { return mSfxId; }
  float GetFallOff() const { return mFalloff; }
  float GetMaxDistance() const { return mMaxDist; }

  static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
                                              const CCharAnimTime& startTime);

  // Stream nodes above this version carry the extra segment/ushort/float fields.
  static const ushort skExtendedVersion;

private:
  uint mSfxId;
  float mFalloff;
  float mMaxDist;
  CSegId mSegId;
  ushort x3a_;
  ushort x3c_;
  float x40_;
};
CHECK_SIZEOF(CSoundPOINode, 0x44)

#endif // _CSOUNDPOINODE
