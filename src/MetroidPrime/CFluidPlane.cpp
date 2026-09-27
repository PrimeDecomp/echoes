#include "MetroidPrime/CFluidPlane.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/SObjectTag.hpp"

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
