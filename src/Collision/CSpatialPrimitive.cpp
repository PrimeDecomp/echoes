#include "Collision/CSpatialPrimitive.hpp"

#include "Kyoto/CFactoryMgr.hpp"

CSpatialPrimitive::CSpatialPrimitive(CInputStream& in) : mSpheres(in), mBoxes(in) {}

CFactoryFnReturn FSpatialPrimitivesFactory(const SObjectTag& tag, CInputStream& in,
                                           const CVParamTransfer& xfer) {
  return rs_new CSpatialPrimitive(in);
}
