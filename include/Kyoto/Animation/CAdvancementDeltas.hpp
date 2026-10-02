#ifndef _CADVANCEMENTDELTAS
#define _CADVANCEMENTDELTAS

#include "Kyoto/Math/CQuaternion.hpp"

struct CAdvancementDeltas {
public:
  CAdvancementDeltas(const CVector3f& posDelta, const CQuaternion& rotDelta)
  : mPosDelta(posDelta), mRotDelta(rotDelta) {}

  const CVector3f& GetOffsetDelta() const { return mPosDelta; }
  const CQuaternion& GetOrientationDelta() const { return mRotDelta; }

  static CAdvancementDeltas Interpolate(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
                                        const float startWeight, const float endWeight);
  static CAdvancementDeltas Blend(const CAdvancementDeltas& a, const CAdvancementDeltas& b,
                                  const float t);

private:
  CVector3f mPosDelta;
  CQuaternion mRotDelta;
};
CHECK_SIZEOF(CAdvancementDeltas, 0x1c)

#endif // _CADVANCEMENTDELTAS
