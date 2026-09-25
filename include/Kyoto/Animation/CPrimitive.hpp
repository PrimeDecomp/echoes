#ifndef _CPRIMITIVE
#define _CPRIMITIVE

#include "Kyoto/SObjectTag.hpp"

#include "rstl/string.hpp"

class CInputStream;
class COutputStream;
class CPrimitive {
public:
  CPrimitive(CInputStream& in);
  void PutTo(COutputStream& out) const;
  CAssetId GetAnimResId() const { return mAnimId; }

private:
  CAssetId mAnimId;
  int mAnimIdx;
  rstl::string mAnimName;
};

#endif // _CPRIMITIVE
