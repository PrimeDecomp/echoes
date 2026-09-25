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
                   bool ambChannelOverflow, int useLightSet);
  ~CLightParameters() {}

  const CColor& GetAmbientColor() const { return ambientColor; }
  bool ShouldMakeLights() const { return makeLights; }
  bool GetAmbientChannelOverflow() const { return ambientChannelOverflow; }
  const CVector3f& GetLightingPositionOffset() const { return lightingPositionOffset; }
  int GetMaxDynamicLights() const { return maxDynamicLights; }
  int GetMaxAreaLights() const { return maxAreaLights; }

  static CLightParameters None();

  static uint GetFramesBetweenRecalculation(ELightRecalculationOptions opts);
  rstl::auto_ptr< CActorLights > MakeActorLights() const;

private:
  bool castShadow; // x0
  float shadowScale;  // x4
  EShadowTessellation shadowTesselation; // x8
  float shadowAlpha; // xc
  float maxShadowHeight; // x10
  CColor ambientColor; // x14
  bool makeLights;
  bool ambientChannelOverflow;
  EWorldLightingOptions useWorldLighting; // x1c
  ELightRecalculationOptions lightRecalculation; // x20
  int useLightSet; // x24
  CVector3f lightingPositionOffset; // x28
  int maxDynamicLights; // x34
  int maxAreaLights; // x38
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
  CVisorParameters(uchar mask, bool b1, bool scanPassthrough)
  : mMask(mask), mB1(b1), mScanPassthrough(scanPassthrough) {}

  uchar GetMask() const { return mMask; }
  // TODO: GetIsBlockXRay__16CVisorParametersCFv?
  bool GetBool1() const { return mB1; }
  bool GetScanPassthrough() const { return mScanPassthrough; }

  static CVisorParameters None() { return CVisorParameters(0xF, false, false); }

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
                   const CVisorParameters& visorParms, bool globalTimeProvider, bool thermalHeat,
                   bool renderUnsorted, bool noSortThermal, float fadeInTime, float fadeOutTime,
                   float thermalMag);
  CActorParameters(const CActorParameters&);
  ~CActorParameters() {}

  CActorParameters Scannable(const CScannableParameters& sParms) const;
  CActorParameters HotInThermal(bool hot) const;
  CActorParameters MakeDamageableTriggerActorParms(const CVisorParameters& visorParam) const;

  const CLightParameters& GetLighting() const { return lighting; }
  const CScannableParameters& GetScannable() const { return scannable; }
  const rstl::pair< CAssetId, CAssetId >& GetXRay() const { return echoAssets; }
  const rstl::pair< CAssetId, CAssetId >& GetInfra() const { return darkAssets; }
  const CVisorParameters& GetVisorParameters() const { return visor; }
  // float GetThermalMag() const { return x64_thermalMag; }
  bool UseGlobalRenderTime() const { return useGlobalRenderTime; }
  bool IsHotInThermal() const { return thermalHeat; }
  bool ForceRenderUnsorted() const { return forceRenderUnsorted; }
  bool NoSortThermal() const { return noSortThermal; }
  float GetFadeInTime() const { return fadeInTime; }
  float GetFadeOutTime() const { return fadeOutTime; }

  static CActorParameters None();

private:
  CLightParameters lighting; // x0
  CScannableParameters scannable; // x3c
  rstl::pair< CAssetId, CAssetId > echoAssets; // x40, model/skin
  rstl::pair< CAssetId, CAssetId > darkAssets; // x48, model/skin
  CVisorParameters visor; // x50
  uchar maxVolume;  // x54
  uchar maxEchoVolume;  // x55
  uchar useGlobalRenderTime : 1; // x56
  uchar thermalHeat : 1;
  uchar forceRenderUnsorted : 1;
  uchar noSortThermal : 1;
  float fadeInTime; // x58
  float fadeOutTime; // x5c
};
CHECK_SIZEOF(CActorParameters, 0x60)

#endif // _CACTORPARAMETERS
