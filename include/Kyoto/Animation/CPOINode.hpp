#ifndef _CPOINODE
#define _CPOINODE

#include "types.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "rstl/string.hpp"

enum EPOIType {
  kPT_Loop = 0,
  kPT_EmptyBool = 1,
  kPT_EmptyInt32 = 2,
  kPT_SoundInt32 = 4,
  kPT_Particle = 5,
  kPT_UserEvent = 6,
  kPT_RandRate = 7,
  kPT_Sound = 8,
};

class CInputStream;
// Echoes replaces Prime's node name string with a CRC32 hash of it.
class CPOINode {
public:
  CPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
           float weight, int charIdx, int flags);
  CPOINode(CInputStream& in);
  virtual ~CPOINode() {}

  uint GetNameHash() const { return mNameHash; }
  const EPOIType GetPoiType() const { return static_cast< EPOIType >(mType); }
  const CCharAnimTime& GetTime() const { return mTime; }
  const int GetIndex() const { return mIndex; }
  const bool GetSaveState() const { return mUnique; }
  const float GetWeight() const { return mWeight; }
  const int GetCharacterIndex() const { return mCharIdx; }
  const int GetFlags() const { return mFlags; }

  bool operator>(const CPOINode& other) const { return mTime > other.mTime; }
  bool operator<(const CPOINode& other) const { return mTime < other.mTime; }
  static int compare(const void* a, const void* b);
  static uint GetHashForString(const char* str);

protected:
  ushort mVersion;
  uint mNameHash;
  ushort mType;
  CCharAnimTime mTime;
  int mIndex;
  bool mUnique;
  float mWeight;
  int mCharIdx;
  int mFlags;
};
CHECK_SIZEOF(CPOINode, 0x2c)

#endif // _CPOINODE
