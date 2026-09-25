#ifndef _CINT32POINODE
#define _CINT32POINODE

#include "Kyoto/Animation/CPOINode.hpp"

#include "rstl/string.hpp"

class CInt32POINode : public CPOINode {
public:
  CInt32POINode(uint nameHash, ushort type, const CCharAnimTime& time, int index, bool unique,
                float weight, int charIdx, int flags, int value, const rstl::string& locatorName)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags)
  , mVal(value)
  , mLctrName(locatorName) {}

  explicit CInt32POINode(CInputStream& in);

  static CInt32POINode CopyNodeMinusStartTime(const CInt32POINode& node,
                                              const CCharAnimTime& startTime);

  int GetValue() const { return mVal; }
  const rstl::string& GetLocatorName() const { return mLctrName; }

private:
  int mVal;
  rstl::string mLctrName;
};
CHECK_SIZEOF(CInt32POINode, 0x40)

#endif // _CINT32POINODE
