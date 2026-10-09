#ifndef _CANIMCHARACTERSET
#define _CANIMCHARACTERSET

#include "Kyoto/Animation/CCEAnimationSet.hpp"
#include "Kyoto/Animation/CCharacterSet.hpp"

class CFactoryFnReturn;
class CVParamTransfer;
struct SObjectTag;

class CAnimCharacterSet {
public:
  explicit CAnimCharacterSet(CInputStream& in);

  const CCharacterSet& GetCharacterSet() const { return mCharacterSet; }
  const CCEAnimationSet& GetAnimationSet() const { return mAnimationSet; }

private:
  ushort mVersion;
  CCharacterSet mCharacterSet;
  CCEAnimationSet mAnimationSet;
};
CHECK_SIZEOF(CAnimCharacterSet, 0x7c)

CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& xfer);

#endif // _CANIMCHARACTERSET
