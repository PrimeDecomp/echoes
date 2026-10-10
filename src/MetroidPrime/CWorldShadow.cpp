#include "MetroidPrime/CWorldShadow.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"
#include "dolphin/gx/GXFrameBuffer.h"

CWorldShadow::CWorldShadow(uint width, uint height, bool rgba8)
: mTexture(rs_new CTexture(rgba8 ? kTF_RGBA8 : kTF_RGB565, width, height, 1))
, mView(CTransform4f::Identity())
, mModel(CTransform4f::Identity())
, mObjectHalfExtent(1.f)
, mObjectPosition(0.f, 1.f, 0.f)
, mLightPosition(CVector3f::Zero())
, mArea(kInvalidAreaId)
, mLightIndex(-1)
, mBlurReset(true) {}

CWorldShadow::~CWorldShadow() {
  if (mTexture.get())
    mTexture->ScheduleDeletion();
}

// Guessed name
bool CWorldShadow::CanRender(const CStateManager& mgr) {
  if (mgr.IsMultiplayer())
    return false;
  if (!mgr.GetIsDarkWorld()) {
    switch (mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
      return true;
    }
  }
  return false;
}

void CWorldShadow::BuildLightShadowTexture(const CStateManager& mgr, TAreaId areaId,
                                           uint lightIndex, const CAABox& bounds, bool motionBlur,
                                           bool lighten) {
  if (mArea != areaId || mLightIndex != lightIndex) {
    mBlurReset = true;
    mArea = areaId;
    mLightIndex = lightIndex;
  }
  if (areaId == kInvalidAreaId)
    return;

  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
  if (!area.IsLoaded())
    return;

  const CGameArea::CPostConstructed& data = *area.GetPostConstructed();
  const CWorldLight& light = data.mLightsA[lightIndex];
  const CVector3f center = bounds.GetCenterPoint();
  const CPVSAreaSet* pvs = data.mPvs.get();
  CPVSVisSet lightSet(kVSS_OutOfBounds);
  if (pvs && pvs->GetLightIndexCount() > 0 && gkPVSEnabled == 1)
    lightSet = pvs->GetLightSet(lightIndex + pvs->GetNum2ndLights());
  const rstl::pair< int, const CPVSVisSet* > areaSet(areaId.Value(), &lightSet);

  CVector3f lightToPoint = center - light.GetPosition();
  mObjectHalfExtent = (bounds.GetMaxPoint() - center).Magnitude();
  const float distance = lightToPoint.Magnitude();
  const float fov = CMath::Rad2Deg(atan2f(mObjectHalfExtent, distance)) * 2.f;
  if (fov < 0.00001f)
    return;

  lightToPoint.Normalize();
  mView = CTransform4f::LookAt(light.GetPosition(), center, CVector3f(0.f, 0.f, -1.f));
  mObjectPosition = center;
  mLightPosition = light.GetPosition();
  CGraphics::SetViewPointMatrix(mView);
  const CFrustumPlanes frustum(mView, CRelAngle::FromDegrees(fov).AsRadians(), 1.f, 0.1f, true,
                               distance + mObjectHalfExtent);
  gpRender->SetPerspective(fov, mTexture->GetWidth(), mTexture->GetHeight(), 0.1f, 1000.f);
  gpRender->PrepareWorldRendering(&areaSet, 1, frustum, nullptr, rstl::vector< CLight >(), nullptr,
                                  0);

  const float depthNear = CGraphics::GetDepthNear();
  const float depthFar = CGraphics::GetDepthFar();
  CGraphics::SetDepthRange(0.f, 1.f);
  const CViewport viewport = CGraphics::GetViewport();
  gpRender->SetViewport(0, 0, mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
  const float extent = 1.4142f * mObjectHalfExtent;
  mModel = CTransform4f::LookAt(center - CVector3f(0.f, 0.f, 0.1f), light.GetPosition());
  gpRender->SetModelMatrix(mModel);
  gpRender->PrimColor(CColor::White());
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  gpRender->BeginTriangleStrip(4);
  gpRender->PrimVertex(CVector3f(-extent, 0.f, extent));
  gpRender->PrimVertex(CVector3f(extent, 0.f, extent));
  gpRender->PrimVertex(CVector3f(-extent, 0.f, -extent));
  gpRender->PrimVertex(CVector3f(extent, 0.f, -extent));
  gpRender->EndPrimitive();

  gpRender->SetModelMatrix(CTransform4f::Identity());
  CCubeModel::SetRenderModelBlack(true);
  CCubeModel::SetDrawingOccluders(true);
  gpRender->DrawUnsortedGeometry(areaId.Value());
  CCubeModel::SetRenderModelBlack(false);
  CCubeModel::SetDrawingOccluders(false);

  if (lighten) {
    gpRender->SetModelMatrix(mModel);
    CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
    CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(1.f, 1.f, 1.f, 0.25f);
    CGraphics::StreamVertex(CVector3f(-extent, 0.f, extent));
    CGraphics::StreamVertex(CVector3f(extent, 0.f, extent));
    CGraphics::StreamVertex(CVector3f(-extent, 0.f, -extent));
    CGraphics::StreamVertex(CVector3f(extent, 0.f, -extent));
    CGraphics::StreamEnd();
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  }
  if (motionBlur && !mBlurReset) {
    CGraphics::SetDepthWriteMode(false, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
    CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::Render2D(*mTexture, 0, mTexture->GetWidth() * 2, mTexture->GetHeight() * 2,
                        -mTexture->GetWidth() * 2, CColor(1.f, 1.f, 1.f, 0.85f));
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
  }
  mBlurReset = false;

  GXSetTexCopySrc(0, CGraphics::GetRenderMode().xfbHeight - mTexture->GetHeight() * 2,
                  mTexture->GetWidth() * 2, mTexture->GetHeight() * 2);
  GXSetTexCopyDst(mTexture->GetWidth(), mTexture->GetHeight(),
                  mTexture->GetTexelFormat() == kTF_RGB565 ? GX_TF_RGB565 : GX_TF_RGBA8, GX_TRUE);
  mTexture->SetFlag1(true);
  GXCopyTex(mTexture->GetBitMapData(0), GX_TRUE);
  mTexture->UnLock();
  gpRender->SetViewport(viewport.mLeft, viewport.mTop, viewport.mWidth, viewport.mHeight);
  CGraphics::SetDepthRange(depthNear, depthFar);
}

void CWorldShadow::EnableModelProjectedShadow(const CTransform4f& transform, uint lightIndex,
                                              float scale) const {
  static float sqrt2 = sqrt(2.f);
  CTransform4f textureTransform = CTransform4f::LookAt(
      CVector3f::Zero(), mLightPosition - mObjectPosition, CVector3f(0.f, 0.f, 1.f));
  CTransform4f rotation = transform;
  rotation.SetTranslation(CVector3f::Zero());
  textureTransform = rotation.GetInverse() * textureTransform;
  textureTransform *= CTransform4f::Scale(sqrt2 * mObjectHalfExtent * scale);
  textureTransform = textureTransform.GetInverse();
  textureTransform = CTransform4f::Translate(0.5f, 0.f, 0.5f) * textureTransform;
  const uchar lightMask = 1 << lightIndex;
  CCubeModel::EnableShadowMaps(mTexture.get(), textureTransform, lightMask, lightMask);
}

void CWorldShadow::DisableModelProjectedShadow() const { CCubeModel::DisableShadowMaps(); }

void CWorldShadow::ResetBlur() { mBlurReset = true; }
