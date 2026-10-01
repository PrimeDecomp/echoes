#include "Kyoto/Animation/CAnimCharacterSet.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CAnimCharacterSet::CAnimCharacterSet(CInputStream& in)
: mVersion(in.Get< ushort >()), mCharacterSet(in), mAnimationSet(in) {}

CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer& xfer) {
  return rs_new CAnimCharacterSet(in);
}
