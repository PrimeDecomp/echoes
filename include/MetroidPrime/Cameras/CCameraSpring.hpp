#ifndef _CCAMERASPRING
#define _CCAMERASPRING

#include "Kyoto/Math/CMath.hpp"

class CCameraSpring {
public:
  CCameraSpring(float k, float max, float tardis)
  : mK(k), mK2Sqrt(2.f * CMath::SqrtF(k)), mMax(max), mTardis(tardis), mDx(0.f) {}

  float ApplyDistanceSpring(float target, float current, float dt);
  void Reset();

private:
  float mK;
  float mK2Sqrt;
  float mMax;
  float mTardis;
  float mDx;
};
CHECK_SIZEOF(CCameraSpring, 0x14)

#endif // _CCAMERASPRING
