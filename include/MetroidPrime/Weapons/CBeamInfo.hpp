#ifndef _CBEAMINFO
#define _CBEAMINFO

#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CBeamInfo {
public:
  CBeamInfo(int beamAttributes, CAssetId contactFxId, CAssetId pulseFxId, CAssetId textureId,
            CAssetId glowTextureId, float length, float radius, float expansionSpeed,
            float lifeTime, float pulseSpeed, float shutdownTime, float contactFxScale,
            float pulseFxScale, const CColor& innerColor, const CColor& outerColor,
            float travelSpeed, CAssetId muzzleFxId);

  int GetBeamAttributes() const { return mBeamAttributes; }
  CAssetId GetContactFXId() const { return mContactFxId; }
  CAssetId GetPulseFXId() const { return mPulseFxId; }
  CAssetId GetTextureId() const { return mTextureId; }
  CAssetId GetGlowTextureId() const { return mGlowTextureId; }
  float GetLength() const { return mLength; }
  float GetRadius() const { return mRadius; }
  float GetExpansionSpeed() const { return mExpansionSpeed; }
  float GetLifeTime() const { return mLifeTime; }
  float GetPulseSpeed() const { return mPulseSpeed; }
  float GetShutdownTime() const { return mShutdownTime; }
  float GetContactFxScale() const { return mContactFxScale; }
  float GetPulseFxScale() const { return mPulseFxScale; }
  float GetTravelSpeed() const { return mTravelSpeed; }
  const CColor& GetInnerColor() const { return mInnerColor; }
  const CColor& GetOuterColor() const { return mOuterColor; }
  CAssetId GetMuzzleFXId() const { return mMuzzleFxId; }

private:
  uint x0_;
  int mBeamAttributes;
  CAssetId mContactFxId;
  CAssetId mPulseFxId;
  CAssetId mTextureId;
  CAssetId mGlowTextureId;
  float mLength;
  float mRadius;
  float mExpansionSpeed;
  float mLifeTime;
  float mPulseSpeed;
  float mShutdownTime;
  float mContactFxScale;
  float mPulseFxScale;
  float mTravelSpeed;
  CColor mInnerColor;
  CColor mOuterColor;
  CAssetId mMuzzleFxId; // Guessed name: particle effect attached to the beam origin.
};
CHECK_SIZEOF(CBeamInfo, 0x48)

#endif // _CBEAMINFO
