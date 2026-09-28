#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
  CBoolPOINode(uint nameHash = -1, ushort type = kPT_EmptyBool,
               const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
               float weight = 1.f, int charIdx = -1, int flags = 0, bool value = false)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags), mVal(value) {}

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
  bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
