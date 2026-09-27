#ifndef _CFLUIDUVMOTION
#define _CFLUIDUVMOTION

#include "rstl/reserved_vector.hpp"

class CFluidUVMotion {
public:
  struct SFluidLayerMotion {
    int mMotion;
    float mOoTimeToWrap;
    float mOrientation;
    float mMagnitude;
    float mUvMul;
    float mUvScale;
  };

private:
  rstl::reserved_vector< SFluidLayerMotion, 5 > mFluidLayers;
  float mOoTimeToWrap;
  float mOrientation;
};
CHECK_SIZEOF(CFluidUVMotion, 0x84)

#endif // _CFLUIDUVMOTION
