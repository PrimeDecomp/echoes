#ifndef _CACTORPARAMETERS
#define _CACTORPARAMETERS

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"

class CActorLights;

class CLightParameters {
public:
  enum EShadowTessellation {
    kST_Invalid = -1,
    kST_Zero,
  };

  enum EWorldLightingOptions {
    kLO_Zero,
    kLO_NormalWorld,
    kLO_NoShadowCast,
    kLO_DisableWorld,
  };

  enum ELightRecalculationOptions {
    kLR_Never,
    kLR_EightFrames,
    kLR_FourFrames,
    kLR_OneFrame,
  };

  CLightParameters();
  CLightParameters(bool castShadow, float shadowScale,
                   CLightParameters::EShadowTessellation shadowTess, float shadowAlpha,
                   float maxShadowHeight, const CColor& ambientColor, bool makeLights,
                   CLightParameters::EWorldLightingOptions useWorldLighting,
                   CLightParameters::ELightRecalculationOptions lightRecalcOpts,
                   const CVector3f& lightingPositionOffset, int maxDynamicLights, int maxAreaLights,
                   bool ambChannelOverflow, int useLightSet, bool disableAmbientLights);

  const CColor& GetAmbientColor() const { return mAmbientColor; }
  bool ShouldMakeLights() const { return mMakeLights; }
  bool GetAmbientChannelOverflow() const { return mAmbientChannelOverflow; }
  const CVector3f& GetLightingPositionOffset() const { return mLightingPositionOffset; }
  int GetMaxDynamicLights() const { return mMaxDynamicLights; }
  int GetMaxAreaLights() const { return mMaxAreaLights; }

  static CLightParameters None() { return CLightParameters(); }

  static uint GetFramesBetweenRecalculation(ELightRecalculationOptions opts);
  rstl::auto_ptr< CActorLights > MakeActorLights() const;

private:
  bool mCastShadow;                       // x0
  float mShadowScale;                     // x4
  EShadowTessellation mShadowTesselation; // x8
  float mShadowAlpha;                     // xc
  float mMaxShadowHeight;                 // x10
  CColor mAmbientColor;                   // x14
  bool mMakeLights : 1;                   // x18
  bool mAmbientChannelOverflow : 1;
  bool mDisableAmbientLights : 1;                 // Guessed name
  EWorldLightingOptions mUseWorldLighting;        // x1c
  ELightRecalculationOptions mLightRecalculation; // x20
  int mUseLightSet;                               // x24
  CVector3f mLightingPositionOffset;              // x28
  int mMaxDynamicLights;                          // x34
  int mMaxAreaLights;                             // x38
};
CHECK_SIZEOF(CLightParameters, 0x3c)

class CScannableParameters {
public:
  CScannableParameters() {}
  CScannableParameters(CAssetId scanId) : mScanId(scanId) {}

  CAssetId GetScannableObject0() const { return mScanId; }

private:
  CAssetId mScanId;
};
CHECK_SIZEOF(CScannableParameters, 0x4)

class CVisorParameters {
public:
  // Original enum type; individual flag names are inferred from visor order.
  enum EVisorOrbitableFlags {
    kVOF_Combat = 1,
    kVOF_Echo = 2,
    kVOF_Scan = 4,
    kVOF_Dark = 8,
  };
  CVisorParameters(uchar mask, bool scanPassthrough)
  : mMask(mask), mScanPassthrough(scanPassthrough) {}

  uchar GetMask() const { return mMask; }
  // TODO: GetIsBlockXRay__16CVisorParametersCFv?
  bool GetBool1() const { return mB1; }
  bool GetScanPassthrough() const { return mScanPassthrough; }

  static CVisorParameters None() { return CVisorParameters(0xF, false); }

private:
  uint mMask : 4;
  uint mScanPassthrough : 1;
  uint mB1 : 1;
};
CHECK_SIZEOF(CVisorParameters, 0x4)

class CActorParameters {
public:
  CActorParameters();
  CActorParameters(const CLightParameters& lightParms, const CScannableParameters& scanParms,
                   const rstl::pair< CAssetId, CAssetId >& xrayAssets,
                   const rstl::pair< CAssetId, CAssetId >& thermalAssets,
                   const CVisorParameters& visorParms, bool globalTimeProvider, bool renderUnsorted,
                   bool highlightedInDarkVisor, bool takesProjectedShadow, bool alphaSorted,
                   bool renderFullEchoModel, uchar maxVolume, uchar maxEchoVolume, float fadeInTime,
                   float fadeOutTime);

  CActorParameters Scannable(const CScannableParameters& sParms) const;
  CActorParameters HotInThermal(bool hot) const;
  CActorParameters WithAlphaSorting(bool enabled) const; // Guessed name.
  CActorParameters MakeDamageableTriggerActorParms(const CVisorParameters& visorParam) const;

  const CLightParameters& GetLighting() const { return mLighting; }
  const CScannableParameters& GetScannable() const { return mScannable; }
  const rstl::pair< CAssetId, CAssetId >& GetXRay() const { return mEchoAssets; }
  const rstl::pair< CAssetId, CAssetId >& GetInfra() const { return mDarkAssets; }
  const CVisorParameters& GetVisorParameters() const { return mVisor; }
  // float GetThermalMag() const { return x64_thermalMag; }
  bool UseGlobalRenderTime() const { return mUseGlobalRenderTime; }
  bool ForceRenderUnsorted() const { return mForceRenderUnsorted; }
  bool IsHighlightedInDarkVisor() const { return mHighlightedInDarkVisor; }
  bool TakesProjectedShadow() const { return mTakesProjectedShadow; }
  bool UseAlphaSorting() const { return mAlphaSorted; } // Guessed name
  // Guessed name: controls whether Echo rendering includes sorted surfaces.
  bool RenderFullEchoModel() const { return mRenderFullEchoModel; }
  float GetFadeInTime() const { return mFadeInTime; }
  float GetFadeOutTime() const { return mFadeOutTime; }
  uchar GetMaxVolume() const { return mMaxVolume; }
  uchar GetMaxEchoVolume() const { return mMaxEchoVolume; }

  static CActorParameters None() { return CActorParameters(); }

private:
  CLightParameters mLighting;                   // x0
  CScannableParameters mScannable;              // x3c
  rstl::pair< CAssetId, CAssetId > mEchoAssets; // x40, model/skin
  rstl::pair< CAssetId, CAssetId > mDarkAssets; // x48, model/skin
  CVisorParameters mVisor;                      // x50
  uchar mMaxVolume;                             // x54
  uchar mMaxEchoVolume;                         // x55
  bool mUseGlobalRenderTime : 1;                // x56
  bool mForceRenderUnsorted : 1;
  bool mHighlightedInDarkVisor : 1;
  bool mTakesProjectedShadow : 1;
  bool mAlphaSorted : 1;
  bool mRenderFullEchoModel : 1; // Guessed name.
  float mFadeInTime;             // x58
  float mFadeOutTime;            // x5c
};
CHECK_SIZEOF(CActorParameters, 0x60)

#endif // _CACTORPARAMETERS
