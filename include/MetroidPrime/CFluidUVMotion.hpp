#ifndef _CFLUIDUVMOTION
#define _CFLUIDUVMOTION

#include "rstl/reserved_vector.hpp"

class CFluidUVMotion {
public:
  enum EFluidMotion { kFM_Linear, kFM_Circular, kFM_Oscillate };
  enum { kNumLayers = 5 };

  struct SFluidLayerMotion {
    EFluidMotion mMotion;
    float mOoTimeToWrap;
    float mOrientation;
    float mMagnitude;
    float mUvMul;
    float mUvScale;

    SFluidLayerMotion(EFluidMotion motion = kFM_Linear, float timeToWrap = 6.f,
                      float orientation = 0.f, float magnitude = 1.f, float uvMul = 5.f)
    : mMotion(motion)
    , mOoTimeToWrap(1.f / timeToWrap)
    , mOrientation(orientation)
    , mMagnitude(magnitude)
    , mUvMul(uvMul)
    , mUvScale(1.f / uvMul) {}
  };

  CFluidUVMotion(float timeToWrap, float orientation, const SFluidLayerMotion& layer0,
                 const SFluidLayerMotion& layer1, const SFluidLayerMotion& layer2,
                 const SFluidLayerMotion& layer3, const SFluidLayerMotion& layer4);

  void CalculateFluidTextureOffset(float t, float offsets[kNumLayers][2]) const;
  // Guessed name: the single-layer counterpart also controls global motion.
  void CalculateFluidLayerOffset(float t, int layer, float offset[2], bool includeGlobal) const;

  float GetOOTimeToWrapTexPage() const { return mOoTimeToWrap; }
  float GetOrientation() const { return mOrientation; }
  const SFluidLayerMotion& GetFluidLayerMotion(int layer) const { return mFluidLayers[layer]; }
  const rstl::reserved_vector< SFluidLayerMotion, kNumLayers >& GetFluidLayers() const {
    return mFluidLayers;
  }

private:
  rstl::reserved_vector< SFluidLayerMotion, kNumLayers > mFluidLayers;
  float mOoTimeToWrap;
  float mOrientation;
};
CHECK_SIZEOF(CFluidUVMotion, 0x84)
extern int SFluidLayerMotionCheck[check_sizeof< CFluidUVMotion::SFluidLayerMotion, 0x18 >::value];

#endif // _CFLUIDUVMOTION
