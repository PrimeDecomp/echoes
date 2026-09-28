#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Animation/CSegId.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(uint nameHash = -1, ushort type = kPT_Sound,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                float weight = 1.f, int charIdx = -1, int flags = 0, int sfxId = 0,
                float fallOff = 0.f, float maxDist = 0.f, const CSegId& segId = CSegId(0),
                ushort pitchStart = 0, ushort pitchEnd = 0, float pitchDuration = 0.f);
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
