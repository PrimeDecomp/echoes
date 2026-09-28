#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Animation/CSegId.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
                float weight, int charIdx, int flags, int sfxId, float fallOff, float maxDist,
                const CSegId& segId, ushort pitchStart, ushort pitchEnd, float pitchDuration);
  CSoundPOINode(CInputStream& in);
  ~CSoundPOINode() override;

  uint GetSoundId() const { return mSfxId; }
  float GetFallOff() const { return mFalloff; }
  float GetMaxDistance() const { return mMaxDist; }
  const CSegId& GetLocator() const { return mSegId; }
  ushort GetPitchStart() const { return mPitchStart; }
  ushort GetPitchEnd() const { return mPitchEnd; }
  float GetPitchDuration() const { return mPitchDuration; }

  static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
                                              const CCharAnimTime& startTime);

  // Stream nodes above this version carry the extra segment/ushort/float fields.
  static const ushort skExtendedVersion;

private:
  uint mSfxId;
  float mFalloff;
  float mMaxDist;
  CSegId mSegId;
  ushort mPitchStart;
  ushort mPitchEnd;
  float mPitchDuration;
};
CHECK_SIZEOF(CSoundPOINode, 0x44)

#endif // _CSOUNDPOINODE
