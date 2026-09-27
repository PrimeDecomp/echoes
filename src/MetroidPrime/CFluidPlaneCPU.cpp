#include "MetroidPrime/CFluidPlaneCPU.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"

CFluidPlaneCPU::CFluidPlaneCPU(const CVector2f& extent, CAssetId colorMap, const CColor& baseColor,
                               CAssetId colorWarpMap, CAssetId glossMap, CAssetId lightMap,
                               CAssetId envMap, CAssetId distortionMap, bool useDynamicLights,
                               int fluidType, const CFluidUVMotion& motion,
                               const CVector2f& uvScale, const CVector2f& uvOffset,
                               float unitsPerLightmapTexel, float alpha, float glossFlat,
                               float glossAngle, float unknown2, float unknown3, float envMapSize,
                               float viscosity)
: CFluidPlane(colorMap, colorWarpMap, glossMap, alpha, fluidType, viscosity, motion)
, mLightMapId(lightMap)
, mEnvMapId(envMap)
, mDistortionMapId(distortionMap)
, mBaseColor(baseColor)
, mGlossFlat(glossFlat)
, mGlossAngle(glossAngle)
, x118_(unknown2)
, x11c_(unknown3)
, mEnvMapSize(envMapSize)
, mUnitsPerLightmapTexel(unitsPerLightmapTexel)
, mUVScale(uvScale)
, mUVOffset(uvOffset)
, mDisplayList(nullptr)
, mDisplayListSize(0)
, mGridDimensions(0, 0)
, mHasDistortionMap(false)
, mHasLightMap(false)
, mHasColorMap(false)
, mHasColorWarpMap(false)
, mHasGlossMap(false)
, mHasEnvMap(false)
, mUseDynamicLights(useDynamicLights) {
  if (gpResourceFactory->GetResourceTypeById(mLightMapId) == FourCC('TXTR')) {
    mLightMap = TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mLightMapId)));
    mHasLightMap = mLightMap.valid();
  }
  if (gpResourceFactory->GetResourceTypeById(mEnvMapId) == FourCC('TXTR')) {
    mEnvMap = TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mEnvMapId)));
    mHasEnvMap = mEnvMap.valid();
  }
  if (gpResourceFactory->GetResourceTypeById(mDistortionMapId) == FourCC('TXTR')) {
    mDistortionMap =
        TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mDistortionMapId)));
    mHasDistortionMap = mDistortionMap.valid();
  }
  mHasColorMap = mColorMap.valid();
  mHasColorWarpMap = mColorWarpMap.valid();
  mHasGlossMap = mGlossMap.valid();

  // TODO: initialize the distortion texture's lower mip levels.
  UpdateGridDisplayList(extent);
}

bool CFluidPlaneCPU::HasDistortionMap() const { return mHasDistortionMap; }

bool CFluidPlaneCPU::HasLightMap() const { return mHasLightMap; }

bool CFluidPlaneCPU::HasColorMap() const { return mHasColorMap; }

bool CFluidPlaneCPU::HasColorWarpMap() const { return mHasColorWarpMap; }

bool CFluidPlaneCPU::HasGlossMap() const { return mHasGlossMap; }

bool CFluidPlaneCPU::HasEnvMap() const { return mHasEnvMap; }

void CFluidPlaneCPU::CalculateLightmapMtx(const CTransform4f& areaXf, const CTransform4f& xf,
                                          const CAABox& bounds, uint matrixId,
                                          const CVector2f& scale, const CVector2f& offset) const {
  // TODO: construct and load the area-relative, texel-aligned lightmap matrix.
}

void CFluidPlaneCPU::ClipPolygonToPlane(const rstl::vector< CVector3f >& polygon,
                                        const CPlane& plane, rstl::vector< CVector3f >& clipped) {
  for (int i = 0; i < polygon.size(); ++i) {
    const CVector3f& a = polygon[i];
    const CVector3f& b = polygon[(i + 1) % polygon.size()];
    const float da = -plane.GetHeight(a);
    const float db = -plane.GetHeight(b);
    if (da < 0.f || db < 0.f) {
      if (da >= 0.f || db >= 0.f) {
        clipped.push_back(a + (da / (da - db)) * (b - a));
        if (da < 0.f && db >= 0.f) {
          clipped.push_back(b);
        }
      }
    } else {
      clipped.push_back(b);
    }
  }
}

void CFluidPlaneCPU::RenderDistortion(float time, const CTransform4f& xf,
                                      const CAABox& bounds) const {
  // TODO: capture the framebuffer and draw the clipped surface with the distortion texture.
}

void CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                                 const CTransform4f& areaXf, const CAABox& bounds,
                                 const CScriptWater* water) const {
  // TODO: configure animated texture matrices, lighting, connected-water blending and TEV stages.
}

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const CAABox& bounds,
                            const CTransform4f& xf, const CTransform4f& areaXf, TUniqueId waterId,
                            const char* gridFlags, int gridDimX, int gridDimY) const {
  // TODO: resolve the water actor and draw either the surface quad or the lit grid display list.
}

void CFluidPlaneCPU::RenderCleanup() const {
  // TODO: restore texture-coordinate generation, lighting and culling after rendering.
}

void CFluidPlaneCPU::PreRender(const CStateManager& mgr, const CVector2f& extent) {
  UpdateGridDisplayList(extent);
}

void CFluidPlaneCPU::UpdateGridDisplayList(const CVector2f& extent) {
  // TODO: rebuild the aligned triangle-strip display list when the lit grid dimensions change.
}
