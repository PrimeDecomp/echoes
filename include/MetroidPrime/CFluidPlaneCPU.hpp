#ifndef _CFLUIDPLANECPU
#define _CFLUIDPLANECPU

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "MetroidPrime/CFluidPlane.hpp"
#include "rstl/single_ptr.hpp"

class CFluidPlaneCPU : public CFluidPlane {
public:
  CFluidPlaneCPU(const CVector2f& extent, CAssetId colorMap, const CColor& baseColor,
                 CAssetId colorWarpMap, CAssetId glossMap, CAssetId lightMap, CAssetId envMap,
                 CAssetId texture, bool useDynamicLights, int fluidType,
                 const CFluidUVMotion& motion, const CVector2f& uvScale, const CVector2f& uvOffset,
                 float unknownScale, float alpha, float glossFlat, float unknown1, float unknown2,
                 float unknown3, float envMapSize, float viscosity);

  // CFluidPlane
  ~CFluidPlaneCPU() override;

private:
  CAssetId mLightMapId;
  CAssetId mEnvMapId;
  CAssetId xd8_;
  rstl::optional_object< TLockedToken< CTexture > > mLightMap;
  rstl::optional_object< TLockedToken< CTexture > > mEnvMap;
  rstl::optional_object< TLockedToken< CTexture > > xfc_;
  CColor mBaseColor;
  float mGlossFlat;
  float x114_;
  float x118_;
  float x11c_;
  float mEnvMapSize;
  float x124_;
  CVector2f mUVScale;  // Guessed name
  CVector2f mUVOffset; // Guessed name
  rstl::single_ptr< uchar > mDisplayList;
  uint mDisplayListSize;
  CVector2i mGridDimensions; // Guessed name
  bool mHasTexture : 1;      // Guessed name
  bool mHasLightMap : 1;
  bool mHasColorMap : 1;
  bool mHasColorWarpMap : 1;
  bool mHasGlossMap : 1;
  bool mHasEnvMap : 1;
  bool mUseDynamicLights : 1;
};
CHECK_SIZEOF(CFluidPlaneCPU, 0x14c)

#endif // _CFLUIDPLANECPU
