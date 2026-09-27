#ifndef _CBOOLPOINODE
#define _CBOOLPOINODE

#include "Kyoto/Animation/CPOINode.hpp"

class CBoolPOINode : public CPOINode {
public:
  CBoolPOINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
               float weight, int charIdx, int flags, bool value)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags), mVal(value) {}

  CBoolPOINode(CInputStream& in);
  static CBoolPOINode CopyNodeMinusStartTime(const CBoolPOINode& node,
                                             const CCharAnimTime& startTime);
  bool GetValue() const { return mVal; }

private:
  bool mVal;
};

#endif // _CBOOLPOINODE
