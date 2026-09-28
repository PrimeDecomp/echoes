#ifndef _CINT32POINODE
#define _CINT32POINODE

#include "Kyoto/Animation/CPOINode.hpp"

#include "rstl/string.hpp"

class CInt32POINode : public CPOINode {
public:
  CInt32POINode(uint nameHash = -1, ushort type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                float weight = 1.f, int charIdx = -1, int flags = 0, int value = 0,
                const rstl::string& locatorName = rstl::string_l("root"))
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
