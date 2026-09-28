#include "MetroidPrime/CSimpleShadow.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"

CSimpleShadow::CSimpleShadow(float scale, float userAlpha, float maxObjHeight, float displacement)
: mXf(CTransform4f::Identity())
, mScale(scale)
, mRadius(1.f)
, mUserAlpha(userAlpha)
, mHeightAlpha(1.f)
, mMaxObjHeight(maxObjHeight)
, mDisplacement(displacement)
, mCollision(false)
, mAlwaysCalculateRadius(true)
, mRadiusCalculated(false) {}

void CSimpleShadow::Calculate(const CAABox& bounds, const CTransform4f& xf,
                              const CStateManager& mgr) {
  mCollision = false;
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const float halfHeight = extent.GetZ() / 2.f;
  const CVector3f position = xf.GetTranslation() + CVector3f(0.f, 0.f, halfHeight);
  const CVector3f direction(0.f, 0.f, -1.f);
  CRayCastResult result =
      mgr.RayStaticIntersection(position, direction, mMaxObjHeight,
                                CMaterialFilter::MakeExclude(CMaterialList(kMT_SeeThrough)));
  float height = mMaxObjHeight;
  if (result.IsValid()) {
    mCollision = true;
    height = result.GetTime();
  }

  CRayCastResult closestResult = result;
  if (height > 0.1f + halfHeight) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, position, direction, mMaxObjHeight,
                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Platform)), nullptr);
    TUniqueId id = kInvalidUniqueId;
    CRayCastResult dynamicResult =
        CGameCollision::RayDynamicIntersection(mgr, id, position, direction, mMaxObjHeight,
                                               CMaterialFilter::GetPassEverything(), nearList);
    if (dynamicResult.IsValid() && dynamicResult.GetTime() < height) {
      closestResult = dynamicResult;
      mCollision = true;
      height = dynamicResult.GetTime();
    }
  }

  if (mCollision) {
    mHeightAlpha = 1.f - height / mMaxObjHeight;
    const CVector3f normal = closestResult.GetPlane().GetNormal();
    mXf = CTransform4f::LookAt(normal, CVector3f::Zero());
    mXf.SetTranslation(closestResult.GetPoint() + mDisplacement * normal);
    if (mAlwaysCalculateRadius || !mRadiusCalculated) {
      mRadius = sqrtf(extent.GetX() * extent.GetX() + extent.GetY() * extent.GetY()) / 2.f;
      mRadiusCalculated = true;
    }
  }
}

void CSimpleShadow::Render(const CTexture* tex) const {
  if (!mCollision)
    return;

  CGraphics::DisableAllLights();
  gpRender->SetModelMatrix(mXf);
  tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  const float radius = mRadius * mScale;
  CGraphics::StreamBegin(kP_Quads);
  CGraphics::StreamColor(CColor(1.f, 1.f, 1.f, mHeightAlpha * mUserAlpha));
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, -radius));
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, -radius));
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(CVector3f(radius, 0.f, radius));
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(CVector3f(-radius, 0.f, radius));
  CGraphics::StreamEnd();
}

const CTransform4f& CSimpleShadow::GetTransform() const { return mXf; }

void CSimpleShadow::SetUserAlpha(float alpha) { mUserAlpha = alpha; }

float CSimpleShadow::GetMaxObjectHeight() const { return mMaxObjHeight; }

void CSimpleShadow::SetAlwaysCalculateRadius(bool value) { mAlwaysCalculateRadius = value; }

CAABox CSimpleShadow::GetBounds() const {
  const CVector3f translation = mXf.GetTranslation();
  const float extent = mRadius * mScale;
  return CAABox(translation - CVector3f(extent, extent, extent),
                translation + CVector3f(extent, extent, extent));
}

CAABox CSimpleShadow::GetMaxShadowBox(const CAABox& bounds) const {
  const float extent = mRadius * mScale;
  const CVector3f center = bounds.GetCenterPoint();
  CAABox expanded = bounds;
  expanded.AccumulateBounds(center + CVector3f(extent, extent, -GetMaxObjectHeight()));
  expanded.AccumulateBounds(center + CVector3f(-extent, -extent, -GetMaxObjectHeight()));
  return expanded;
}

bool CSimpleShadow::Valid() const { return mCollision; }
