#ifndef _CFLUIDPLANECPU
#define _CFLUIDPLANECPU

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "MetroidPrime/CFluidPlane.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CPlane;
class CScriptWater;
class CVector3f;

class CFluidPlaneCPU : public CFluidPlane {
public:
  CFluidPlaneCPU(const CVector2f& extent, CAssetId colorMap, const CColor& baseColor,
                 CAssetId colorWarpMap, CAssetId glossMap, CAssetId lightMap, CAssetId envMap,
                 CAssetId distortionMap, bool useDynamicLights, int fluidType,
                 const CFluidUVMotion& motion, const CVector2f& uvScale, const CVector2f& uvOffset,
                 float unitsPerLightmapTexel, float alpha, float glossFlat, float glossAngle,
                 float unknown2, float unknown3, float envMapSize, float viscosity);

  // CFluidPlane
  ~CFluidPlaneCPU() override {}
  void Render(const CStateManager& mgr, float alpha, const CAABox& bounds, const CTransform4f& xf,
              const CTransform4f& areaXf, TUniqueId waterId, const char* gridFlags, int gridDimX,
              int gridDimY) const override;
  void PreRender(const CStateManager& mgr, const CVector2f& extent) override;

  void RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                   const CTransform4f& areaXf, const CAABox& bounds,
                   const CScriptWater* water) const;
  void RenderCleanup() const;
  void CalculateLightmapMtx(const CTransform4f& areaXf, const CTransform4f& xf,
                            const CAABox& bounds, uint matrixId, const CVector2f& scale,
                            const CVector2f& offset) const;
  // Guessed names
  static void ClipPolygonToPlane(const rstl::vector< CVector3f >& polygon, const CPlane& plane,
                                 rstl::vector< CVector3f >& clipped);
  void RenderDistortion(float time, const CTransform4f& xf, const CAABox& bounds) const;
  void UpdateGridDisplayList(const CVector2f& extent);

  bool HasEnvMap() const;
  bool HasGlossMap() const;
  bool HasColorWarpMap() const;
  bool HasColorMap() const;
  bool HasLightMap() const;
  bool HasDistortionMap() const;

private:
  CAssetId mLightMapId;
  CAssetId mEnvMapId;
  CAssetId mDistortionMapId; // Guessed name
  rstl::optional_object< TLockedToken< CTexture > > mLightMap;
  rstl::optional_object< TLockedToken< CTexture > > mEnvMap;
  rstl::optional_object< TLockedToken< CTexture > > mDistortionMap; // Guessed name
  CColor mBaseColor;
  float mGlossFlat;
  float mGlossAngle; // Guessed name: grazing-angle endpoint of the gloss interpolation.
  float x118_;
  float x11c_;
  float mEnvMapSize;
  float mUnitsPerLightmapTexel;
  CVector2f mUVScale;
  CVector2f mUVOffset;
  rstl::single_ptr< uchar > mDisplayList;
  uint mDisplayListSize;
  CVector2i mGridDimensions;
  bool mHasDistortionMap : 1;
  bool mHasLightMap : 1;
  bool mHasColorMap : 1;
  bool mHasColorWarpMap : 1;
  bool mHasGlossMap : 1;
  bool mHasEnvMap : 1;
  bool mUseDynamicLights : 1;
};
CHECK_SIZEOF(CFluidPlaneCPU, 0x14c)

#endif // _CFLUIDPLANECPU
