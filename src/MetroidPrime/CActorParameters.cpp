#include "MetroidPrime/CActorParameters.hpp"

#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/SEchoParameters.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"

CActorParameters::CActorParameters()
: mLighting(CLightParameters::None())
, mScannable(kInvalidAssetId)
, mEchoAssets(0, 0)
, mDarkAssets(0, 0)
, mVisor(CVisorParameters::None())
, mMaxVolume(CAudioSys::kMaxVolume)
, mMaxEchoVolume(CAudioSys::kMaxVolume)
, mUseGlobalRenderTime(true)
, mForceRenderUnsorted(false)
, mHighlightedInDarkVisor(false)
, mTakesProjectedShadow(true)
, mAlphaSorted(false)
, mRenderFullEchoModel(false)
, mFadeInTime(0.f)
, mFadeOutTime(0.f) {}

CActorParameters::CActorParameters(const CLightParameters& lightParms,
                                   const CScannableParameters& scanParms,
                                   const rstl::pair< CAssetId, CAssetId >& echoAssets,
                                   const rstl::pair< CAssetId, CAssetId >& darkAssets,
                                   const CVisorParameters& visorParms,
                                   const bool globalTimeProvider, const bool renderUnsorted,
                                   const bool highlightedInDarkVisor,
                                   const bool takesProjectedShadow, const bool alphaSorted,
                                   const bool renderFullEchoModel, const uchar maxVolume,
                                   const uchar maxEchoVolume, const float fadeInTime,
                                   const float fadeOutTime)
: mLighting(lightParms)
, mScannable(scanParms)
, mEchoAssets(echoAssets)
, mDarkAssets(darkAssets)
, mVisor(visorParms)
, mMaxVolume(maxVolume)
, mMaxEchoVolume(maxEchoVolume)
, mUseGlobalRenderTime(globalTimeProvider)
, mForceRenderUnsorted(renderUnsorted)
, mHighlightedInDarkVisor(highlightedInDarkVisor)
, mTakesProjectedShadow(takesProjectedShadow)
, mAlphaSorted(alphaSorted)
, mRenderFullEchoModel(renderFullEchoModel)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime) {}

CActorParameters CActorParameters::Scannable(const CScannableParameters& sParms) const {
  CActorParameters result(*this);
  result.mScannable = sParms;
  return result;
}

CActorParameters
CActorParameters::MakeDamageableTriggerActorParms(const CVisorParameters& visorParam) const {
  CActorParameters result(*this);
  result.mVisor = visorParam;
  return result;
}

CActorParameters CActorParameters::HotInThermal(bool hot) const {
  CActorParameters result(*this);
  result.mHighlightedInDarkVisor = hot;
  return result;
}

CActorParameters CActorParameters::WithAlphaSorting(bool enabled) const {
  CActorParameters result(*this);
  result.mAlphaSorted = enabled;
  return result;
}

CLightParameters::CLightParameters(bool castShadow, float shadowScale,
                                   CLightParameters::EShadowTessellation shadowTess,
                                   float shadowAlpha, float maxShadowHeight,
                                   const CColor& ambientColor, bool makeLights,
                                   CLightParameters::EWorldLightingOptions useWorldLighting,
                                   CLightParameters::ELightRecalculationOptions lightRecalculation,
                                   const CVector3f& lightingPositionOffset, int maxDynamicLights,
                                   int maxAreaLights, bool ambChannelOverflow, int useLightSet,
                                   bool disableAmbientLights)
: mCastShadow(castShadow)
, mShadowScale(shadowScale)
, mShadowTesselation(shadowTess)
, mShadowAlpha(shadowAlpha)
, mMaxShadowHeight(maxShadowHeight)
, mAmbientColor(ambientColor)
, mMakeLights(makeLights)
, mAmbientChannelOverflow(ambChannelOverflow)
, mDisableAmbientLights(disableAmbientLights)
, mUseWorldLighting(useWorldLighting)
, mLightRecalculation(lightRecalculation)
, mUseLightSet(useLightSet)
, mLightingPositionOffset(lightingPositionOffset)
, mMaxDynamicLights(maxDynamicLights)
, mMaxAreaLights(maxAreaLights) {
  if (mMaxDynamicLights > 4 || mMaxDynamicLights == -1)
    mMaxDynamicLights = 4;
  if (mMaxAreaLights > 4 || mMaxAreaLights == -1)
    mMaxAreaLights = 4;
}

CLightParameters::CLightParameters()
: mCastShadow(false)
, mShadowScale(0.f)
, mShadowTesselation(kST_Zero)
, mShadowAlpha(0.f)
, mMaxShadowHeight(0.f)
, mAmbientColor(CColor::White())
, mMakeLights(false)
, mAmbientChannelOverflow(false)
, mDisableAmbientLights(false)
, mUseWorldLighting(kLO_Zero)
, mLightRecalculation(kLR_EightFrames)
, mUseLightSet(0)
, mLightingPositionOffset(CVector3f::Zero())
, mMaxDynamicLights(4)
, mMaxAreaLights(4) {}

uint CLightParameters::GetFramesBetweenRecalculation(ELightRecalculationOptions opts) {
  switch (opts) {
  case kLR_Never:
    return 0x3FFFFFFF;
  case kLR_EightFrames:
    return 8;
  case kLR_FourFrames:
    return 4;
  case kLR_OneFrame:
    return 1;
  default:
    return 8;
  }
}

rstl::auto_ptr< CActorLights > CLightParameters::MakeActorLights() const {
  rstl::auto_ptr< CActorLights > result;
  if (mMakeLights) {
    result = rs_new CActorLights(
        GetFramesBetweenRecalculation(mLightRecalculation), mLightingPositionOffset,
        mMaxDynamicLights, mMaxAreaLights, CActorLights::kDefaultMinPosChange,
        mAmbientChannelOverflow, mUseLightSet == 1, mUseWorldLighting == kLO_DisableWorld,
        mDisableAmbientLights);
    if (mUseWorldLighting == kLO_NoShadowCast) {
      result->SetCastShadows(false);
    }
    if (mMaxAreaLights == 0 && mMakeLights) {
      result->SetAmbientColor(mAmbientColor);
    }
  }
  return result;
}

SEchoParameters::SEchoParameters(bool isEchoEmitter, bool onlyEmitDamage, uint numSoundWaves,
                                 float spaceBetweenWaves, float waveLineSize,
                                 float visibilityDecayTime)
: mIsEchoEmitter(isEchoEmitter)
, mOnlyEmitDamage(onlyEmitDamage)
, mNumSoundWaves(numSoundWaves)
, mSpaceBetweenWaves(spaceBetweenWaves)
, mWaveLineSize(waveLineSize)
, mVisibilityDecayTime(visibilityDecayTime) {}

SEchoParameters SEchoParameters::None() { return SEchoParameters(false, false, 0, 0.f, 0.f, 0.f); }
