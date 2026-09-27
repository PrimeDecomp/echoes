#include "MetroidPrime/CFluidUVMotion.hpp"

#include "Kyoto/Math/CMath.hpp"

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation, const SFluidLayerMotion& layer0,
                               const SFluidLayerMotion& layer1, const SFluidLayerMotion& layer2,
                               const SFluidLayerMotion& layer3, const SFluidLayerMotion& layer4)
: mFluidLayers(), mOoTimeToWrap(1.f / timeToWrap), mOrientation(orientation) {
  mFluidLayers.resize(kNumLayers);
  mFluidLayers[0] = layer0;
  mFluidLayers[1] = layer1;
  mFluidLayers[2] = layer2;
  mFluidLayers[3] = layer3;
  mFluidLayers[4] = layer4;
}

void CFluidUVMotion::CalculateFluidLayerOffset(float t, int layerIndex, float offset[2],
                                               bool includeGlobal) const {
  const SFluidLayerMotion& layer = mFluidLayers[layerIndex];
  const float speedT = t * layer.mOoTimeToWrap;
  const float cycleT = speedT - floorf(speedT);
  float localX = 0.f;
  float localY = 0.f;
  switch (layer.mMotion) {
  case kFM_Linear:
    localX = cycleT;
    break;
  case kFM_Circular:
    localY = layer.mMagnitude * CMath::FastSinR(M_2PIF * cycleT);
    localX = layer.mMagnitude * CMath::FastCosR(M_2PIF * cycleT);
    break;
  case kFM_Oscillate:
    localX = layer.mMagnitude * CMath::FastCosR(M_2PIF * cycleT);
    break;
  }

  float x =
      CMath::FastCosR(layer.mOrientation) * localY + CMath::FastSinR(layer.mOrientation) * localX;
  float y =
      CMath::FastCosR(layer.mOrientation) * localX + CMath::FastSinR(layer.mOrientation) * localY;
  if (includeGlobal) {
    const float distance = t * mOoTimeToWrap;
    x += distance * CMath::FastSinR(mOrientation);
    y += distance * CMath::FastCosR(mOrientation);
  }

  x += 0.5f;
  y += 0.5f;
  offset[0] = x - floorf(x);
  offset[1] = y - floorf(y);
}

void CFluidUVMotion::CalculateFluidTextureOffset(float t, float offsets[kNumLayers][2]) const {
  for (int i = 0; i < mFluidLayers.size(); ++i) {
    CalculateFluidLayerOffset(t, i, offsets[i], true);
  }
}
