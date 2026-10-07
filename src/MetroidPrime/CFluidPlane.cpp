#include "MetroidPrime/CFluidPlane.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/SObjectTag.hpp"

// These globals own .sdata2 0x8041B7C8..0x8041B7D4. The original also holds an unreferenced 0.0f
// between the margin and the flags (offset 4), which is not reproduced here.
const float gkWaterGridRayMargin = 0.8f;
const bool gkWaterEnable = true;
const bool gkWaterFog = true;

CFluidPlane::CFluidPlane(CAssetId colorMap, CAssetId colorWarpMap, CAssetId glossMap, float alpha,
                         int fluidType, float viscosity, const CFluidUVMotion& motion)
: mColorMapId(colorMap)
, mColorWarpMapId(colorWarpMap)
, mGlossMapId(glossMap)
, mAlpha(alpha)
, mFluidType(fluidType)
, mViscosity(viscosity)
, mUvMotion(motion) {
  if (gpResourceFactory->GetResourceTypeById(mColorMapId) == FourCC('TXTR')) {
    mColorMap = TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mColorMapId)));
  }
  if (gpResourceFactory->GetResourceTypeById(mColorWarpMapId) == FourCC('TXTR')) {
    mColorWarpMap =
        TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mColorWarpMapId)));
  }
  if (gpResourceFactory->GetResourceTypeById(mGlossMapId) == FourCC('TXTR')) {
    mGlossMap = TLockedToken< CTexture >(gpSimplePool->GetObj(SObjectTag('TXTR', mGlossMapId)));
  }
}

CFluidPlane::~CFluidPlane() {}

void CFluidPlane::Render(const CStateManager& mgr, float alpha, const CAABox& bounds,
                         const CTransform4f& xf, const CTransform4f& areaXf, TUniqueId waterId,
                         const char* gridFlags, int gridDimX, int gridDimY) const {}

void CFluidPlane::UnkVtable0C() {}
