#include "MetroidPrime/CFluidPlaneCPU.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/os/OSCache.h"
#include "rstl/math.hpp"
#include <math.h>
#include <string.h>

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

  if (distortionMap != kInvalidAssetId) {
    const TLockedToken< CTexture > texture(gpSimplePool->GetObj(SObjectTag('TXTR', distortionMap)));
    for (int mip = 2; mip < texture->GetNumberOfMipMaps(); ++mip) {
      uchar* data = static_cast< uchar* >(const_cast< void* >(texture->GetConstBitMapData(mip)));
      const int size =
          (texture->GetBitsPerPixel() * texture->GetWidth() * texture->GetHeight() / 8) /
          (1 << (mip * 2));
      for (uchar* end = data + size; data < end; ++data) {
        *data = 0x80;
      }
    }
  }
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
  int width = (*mLightMap)->GetWidth();
  int height = (*mLightMap)->GetHeight();

  CVector3f norm = areaXf.GetRow(kDZ).AsNormalized();
  CTransform4f toLocal = areaXf.GetRotation().GetQuickInverse();
  CAABox areaLocalAABB = bounds.GetTransformedAABox(toLocal);

  float scaleU = areaLocalAABB.GetWidth() / (width * mUnitsPerLightmapTexel);
  float scaleV = areaLocalAABB.GetHeight() / (height * mUnitsPerLightmapTexel);

  float leftBorder = (1.f + fmodf(areaLocalAABB.GetMinPoint().GetX() + xf.GetTranslation().GetX(),
                                  mUnitsPerLightmapTexel)) /
                     width;
  float rightBorder = (2.f - fmodf(areaLocalAABB.GetMaxPoint().GetX() + xf.GetTranslation().GetX(),
                                   mUnitsPerLightmapTexel)) /
                      width;
  float bottomBorder = (1.f + fmodf(areaLocalAABB.GetMinPoint().GetY() + xf.GetTranslation().GetY(),
                                    mUnitsPerLightmapTexel)) /
                       height;
  float topBorder = (2.f - fmodf(areaLocalAABB.GetMaxPoint().GetY() + xf.GetTranslation().GetY(),
                                 mUnitsPerLightmapTexel)) /
                    height;

  CTransform4f texMtx(
      (scaleU - leftBorder - rightBorder) * scale.GetX() / areaLocalAABB.GetWidth(), 0.f, 0.f,
      offset.GetX() +
          (leftBorder + scaleU * -areaLocalAABB.GetMinPoint().GetX() / areaLocalAABB.GetWidth()),
      0.f, (-(scaleV - bottomBorder - topBorder)) * scale.GetY() / areaLocalAABB.GetHeight(), 0.f,
      offset.GetY() +
          (scaleV * areaLocalAABB.GetMinPoint().GetY() / areaLocalAABB.GetHeight() - topBorder),
      0.f, 0.f, 0.f, 0.f);

  CTransform4f result(texMtx * toLocal);
  GXLoadTexMtxImm(result.GetCStyleMatrix(), matrixId, GX_MTX2x4);
}

void CFluidPlaneCPU::ClipPolygonToPlane(const rstl::vector< CVector3f >& polygon,
                                        const CPlane& plane, rstl::vector< CVector3f >& clipped) {
  for (int i = 0; i < polygon.size(); ++i) {
    const CVector3f& a = polygon[i];
    const CVector3f& b = polygon[i + 1 >= polygon.size() ? 0 : i + 1];
    const float da = -CVector3f::Dot(a, plane.GetNormal()) + plane.GetConstant();
    const float db = -CVector3f::Dot(b, plane.GetNormal()) + plane.GetConstant();
    if (da >= 0.f && db >= 0.f) {
      clipped.push_back_unsafe(b);
    } else if (!(da < 0.f && db < 0.f)) {
      const float t = da / (da - db);
      clipped.push_back_unsafe(CVector3f(t * (b.GetX() - a.GetX()) + a.GetX(),
                                         t * (b.GetY() - a.GetY()) + a.GetY(),
                                         t * (b.GetZ() - a.GetZ()) + a.GetZ()));
      if (da < 0.f && db >= 0.f) {
        clipped.push_back_unsafe(b);
      }
    }
  }
}

void CFluidPlaneCPU::RenderDistortion(float time, const CTransform4f& xf,
                                      const CAABox& bounds) const {
  if (!gkWaterEnable) {
    return;
  }
  const CViewport viewport = CGraphics::GetViewport();
  GXFogType fogType;
  float fogStart, fogEnd, fogNear, fogFar;
  GXColor fogColor;
  CGX::GetFog(&fogType, &fogStart, &fogEnd, &fogNear, &fogFar, &fogColor);
  CGX::SetFog(GX_FOG_NONE, 0.f, 0.f, 0.f, 0.f, fogColor);
  void* buffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopySrc(viewport.mLeft, viewport.mTop, viewport.mWidth, viewport.mHeight);
  GXSetTexCopyDst(viewport.mHalfWidth, viewport.mHalfHeight, GX_TF_RGB565, GX_TRUE);
  const bool videoFilter = CGraphics::GetUseVideoFilter();
  CGraphics::SetUseVideoFilter(false);
  GXCopyTex(buffer, GX_FALSE);
  CGraphics::SetUseVideoFilter(videoFilter);
  GXPixModeSync();
  CGraphics::LoadDolphinSpareTexture(viewport.mHalfWidth, viewport.mHalfHeight, GX_TF_RGB565,
                                     nullptr, CGraphics::kSpareBufferTexMapID);

  rstl::vector< CVector3f > polygon;
  polygon.reserve(4);
  const CVector3f position = xf.GetTranslation();
  const float surfaceZ = bounds.GetMaxPoint().GetZ() - (0.5f * bounds.GetDepth() - position.GetZ());
  polygon.push_back(CVector3f(bounds.GetMinPoint().GetX() + position.GetX(),
                              bounds.GetMaxPoint().GetY() + position.GetY(), surfaceZ));
  polygon.push_back(CVector3f(bounds.GetMaxPoint().GetX() + position.GetX(),
                              bounds.GetMaxPoint().GetY() + position.GetY(), surfaceZ));
  polygon.push_back(CVector3f(bounds.GetMaxPoint().GetX() + position.GetX(),
                              bounds.GetMinPoint().GetY() + position.GetY(), surfaceZ));
  polygon.push_back(CVector3f(bounds.GetMinPoint().GetX() + position.GetX(),
                              bounds.GetMinPoint().GetY() + position.GetY(), surfaceZ));
  rstl::vector< CVector3f > clipped;
  clipped.reserve(6);
  const CUnitVector3f normal(
      CGraphics::GetViewMatrix().Rotate(CUnitVector3f(0.f, -1.f, 0.f, CUnitVector3f::kN_Yes)));
  const CPlane nearPlane(CGraphics::GetViewPoint().Magnitude() +
                             1.1f * CGraphics::GetProjectionState().GetNear(),
                         normal);
  ClipPolygonToPlane(polygon, nearPlane, clipped);

  CGX::ResetVtxDescv();
  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  CGX::SetVtxDesc(GX_VA_NRM, GX_DIRECT);
  CGX::SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_NRM, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, CGraphics::kSpareBufferTexMapID, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_ZERO, GX_CC_TEXC);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, GX_FALSE, GX_FALSE, GX_ITBA_OFF);

  float strength = 0.f;
  const float distance = CMath::AbsF(CGraphics::GetViewPoint().GetZ() - xf.GetTranslation().GetZ());
  if (distance < 15.f) {
    const float angle = M_PIF / 2.f * (distance / 15.f);
    strength = 0.03f * (CMath::FastCosR(angle) * CMath::FastCosR(angle));
  }
  float indMtx[2][3] = {};
  indMtx[0][0] = strength;
  indMtx[1][1] = strength;
  GXSetIndTexMtx(GX_ITM_0, indMtx, 0);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
  float offsets[2] = {0.f, 0.f};
  mUvMotion.CalculateFluidLayerOffset(time, 4, offsets, false);
  (*mDistortionMap)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP0);
  CGX::SetNumIndStages(1);
  CGX::SetNumTexGens(2);
  CGX::SetNumTevStages(1);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCullMode(GX_CULL_NONE);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (clipped.size() > 2) {
    CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, clipped.size());
    for (rstl::vector< CVector3f >::const_iterator it = clipped.begin(); it != clipped.end();
         ++it) {
      GXPosition3f32(it->GetX(), it->GetY(), it->GetZ());
      const CVector3f projected = CGraphics::GetPerspectiveProjectionMatrix().MultiplyOneOverW(
          CGraphics::GetViewMatrix().TransposeRotate(*it - CGraphics::GetViewPoint()));
      const float w = CGraphics::GetPerspectiveProjectionMatrix().MultiplyGetW(
          CGraphics::GetViewMatrix().TransposeRotate(*it - CGraphics::GetViewPoint()));
      const float s = 0.5f * projected.GetX() + 0.5f;
      const float t = 0.5f * -projected.GetY() + 0.5f;
      GXNormal3f32(s * w, t * w, w);
      GXTexCoord2f32(w * (s + offsets[0]), w * (t + offsets[1]));
    }
    CGX::End();
  }
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetVtxDesc(GX_VA_TEX0, GX_NONE);
  gpRender->SetModelMatrix(xf);
  CGX::SetFog(fogType, fogStart, fogEnd, fogNear, fogFar, fogColor);
}

void CFluidPlaneCPU::RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                                 const CTransform4f& areaXf, const CAABox& bounds,
                                 const CScriptWater* water) const {
  if (!gkWaterEnable) {
    return;
  }
  const float uvTime = mgr.GetFluidPlaneManager()->GetUVTime();
  const bool hasLightmap = HasLightMap();
  bool hasDoubleLightmap = false;
  const int envMapType =
      mgr.GetCurrentRenderCameraManager()->GetCurrentCamera(mgr, true)->GetFluidCount() != 0
          ? 0
          : HasEnvMap();
  const CAABox transformed =
      CAABox(CVector3f::Zero(), CVector3f(1.f, 1.f, 1.f)).GetTransformedAABox(xf);
  const float width = transformed.GetWidth();
  const float height = transformed.GetHeight();
  gpRender->SetModelMatrix(xf);
  if (HasDistortionMap()) {
    RenderDistortion(uvTime, xf, bounds);
  }

  CGX::ResetVtxDescv();
  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  CGX::SetVtxDesc(GX_VA_NRM, GX_DIRECT);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXNormal3f32(0.f, 0.f, 1.f);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXNormal3f32(0.f, 0.f, 1.f);
  GXPosition3f32(0.f, 0.f, 0.f);
  GXNormal3f32(0.f, 0.f, 1.f);
  CGX::End();

  GXTevColorArg lightColor;
  GXChannelID lightChannel;
  if (mUseDynamicLights) {
    const GXColor black = {0, 0, 0, 0};
    const GXColor white = {255, 255, 255, 255};
    CGX::SetNumChans(1);
    CGX::SetChanCtrl(CGX::Channel0, GX_TRUE, GX_SRC_REG, GX_SRC_REG,
                     static_cast< GXLightID >(CGraphics::GetLightMask()), GX_DF_CLAMP, GX_AF_SPOT);
    CGX::SetChanMatColor(CGX::Channel0, CGraphics::GetLightMask() != 0 ? white : black);
    if (hasLightmap) {
      CGX::SetChanAmbColor(CGX::Channel0, black);
    }
    lightColor = GX_CC_RASC;
    lightChannel = GX_COLOR0A0;
  } else {
    CGX::SetNumChans(0);
    lightColor = GX_CC_ZERO;
    lightChannel = GX_COLOR_NULL;
  }

  GXTexMapID texMapIds[8];
  GXTexCoordID texCoordIds[8];
  const CTexture& zeroTexture = CCubeRenderer::That()->GetBlackTexture();
  (mColorWarpMap.valid() ? **mColorWarpMap : &zeroTexture)->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  texMapIds[1] = GX_TEXMAP0;
  (HasGlossMap() ? **mGlossMap : &zeroTexture)->Load(GX_TEXMAP1, CTexture::kCM_Repeat);
  texMapIds[2] = GX_TEXMAP1;
  (mColorMap.valid() ? **mColorMap : &zeroTexture)->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  texMapIds[0] = GX_TEXMAP2;
  int nextTexMap = 3;
  if (envMapType != 0) {
    texMapIds[6] = static_cast< GXTexMapID >(nextTexMap);
    (*mEnvMap)->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
  }

  float uvOffsets[CFluidUVMotion::kNumLayers][2];
  mUvMotion.CalculateFluidTextureOffset(uvTime, uvOffsets);
  const float glossScale1 = mUvMotion.GetFluidLayerMotion(2).mUvScale;
  float glossMtx1[2][4] = {};
  glossMtx1[0][0] = width * glossScale1;
  glossMtx1[0][3] = uvOffsets[2][0];
  glossMtx1[1][1] = height * glossScale1;
  glossMtx1[1][3] = uvOffsets[2][1];
  texCoordIds[2] = GX_TEXCOORD0;
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
  GXLoadTexMtxImm(glossMtx1, GX_TEXMTX0, GX_MTX2x4);

  const float glossScale2 = mUvMotion.GetFluidLayerMotion(3).mUvScale;
  float glossMtx2[2][4] = {};
  glossMtx2[0][0] = glossScale2 * width;
  glossMtx2[0][3] = uvOffsets[3][0];
  glossMtx2[1][1] = glossScale2 * height;
  glossMtx2[1][3] = uvOffsets[3][1];
  texCoordIds[3] = GX_TEXCOORD1;
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_POS, static_cast< GXTexMtx >(32), GX_FALSE,
                      GX_PTIDENTITY);
  GXLoadTexMtxImm(glossMtx2, 32, GX_MTX2x4);

  const float colorScale = mUvMotion.GetFluidLayerMotion(0).mUvScale;
  float colorMtx[2][4] = {};
  colorMtx[0][0] = colorScale * width;
  colorMtx[0][3] = uvOffsets[0][0];
  colorMtx[1][1] = colorScale * height;
  colorMtx[1][3] = uvOffsets[0][1];
  texCoordIds[0] = GX_TEXCOORD2;
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, static_cast< GXTexMtx >(34), GX_FALSE,
                      GX_PTIDENTITY);
  GXLoadTexMtxImm(colorMtx, 34, GX_MTX2x4);

  const float warpScale = mUvMotion.GetFluidLayerMotion(1).mUvScale;
  float warpMtx[2][4] = {};
  warpMtx[0][0] = warpScale * width;
  warpMtx[0][3] = uvOffsets[1][0];
  warpMtx[1][1] = warpScale * height;
  warpMtx[1][3] = uvOffsets[1][1];
  texCoordIds[1] = GX_TEXCOORD3;
  CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_POS, static_cast< GXTexMtx >(36), GX_FALSE,
                      GX_PTIDENTITY);
  GXLoadTexMtxImm(warpMtx, 36, GX_MTX2x4);
  int nextCoord = 4;
  uint texMtx = 38;
  const float indMtx[2][3] = {{0.25f, 0.f, 0.f}, {0.f, 0.25f, 0.f}};
  GXSetIndTexMtx(GX_ITM_0, indMtx, 1);
  GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

  if (envMapType != 0) {
    const float maxDim = rstl::max_val(bounds.GetWidth(), bounds.GetHeight());
    const float ooMaxDim = 1.f / maxDim;
    float envMtx[3][4] = {};
    envMtx[0][0] = ooMaxDim;
    envMtx[0][3] = 0.5f + -bounds.GetCenterPoint().GetX() / maxDim;
    envMtx[1][1] = ooMaxDim;
    envMtx[1][3] = 0.5f + -bounds.GetCenterPoint().GetY() / maxDim;
    GXLoadTexMtxImm(envMtx, texMtx, GX_MTX2x4);
    texCoordIds[6] = static_cast< GXTexCoordID >(nextCoord);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                        static_cast< GXTexMtx >(texMtx), GX_FALSE, GX_PTIDENTITY);
    texMtx += 2;
  }

  if (hasLightmap) {
    float lightmapAlpha = 1.f;
    const float darkLevel =
        mgr.GetWorld()->GetArea(mgr.GetNextAreaId())->GetPostConstructed()->mWorldLightingLevel;
    const CScriptWater* nextWater = water->GetNextConnectedWater(mgr);
    if (close_enough(water->GetMorphFactor(), 0.f) || nextWater == nullptr ||
        !nextWater->GetFluidPlane().HasLightMap()) {
      texMapIds[4] = static_cast< GXTexMapID >(nextTexMap);
      (*mLightMap)->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
      CalculateLightmapMtx(areaXf, xf, bounds, texMtx, mUVScale, mUVOffset);
      texCoordIds[4] = static_cast< GXTexCoordID >(nextCoord);
      CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                          static_cast< GXTexMtx >(texMtx), GX_FALSE, GX_PTIDENTITY);
    } else if (nextWater != nullptr && nextWater->GetFluidPlane().HasLightMap()) {
      const CFluidPlaneCPU& next = nextWater->GetFluidPlane();
      if (close_enough(water->GetMorphFactor(), 1.f) || mLightMapId == next.mLightMapId) {
        texMapIds[4] = static_cast< GXTexMapID >(nextTexMap);
        (*next.mLightMap)->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
        next.CalculateLightmapMtx(areaXf, xf, bounds, texMtx, mUVScale, mUVOffset);
        texCoordIds[4] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast< GXTexMtx >(texMtx), GX_FALSE, GX_PTIDENTITY);
      } else {
        texMapIds[4] = static_cast< GXTexMapID >(nextTexMap);
        (*mLightMap)->Load(static_cast< GXTexMapID >(nextTexMap++), CTexture::kCM_Repeat);
        CalculateLightmapMtx(areaXf, xf, bounds, texMtx, mUVScale, mUVOffset);
        texCoordIds[4] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast< GXTexMtx >(texMtx), GX_FALSE, GX_PTIDENTITY);
        texMapIds[5] = static_cast< GXTexMapID >(nextTexMap);
        (*next.mLightMap)->Load(static_cast< GXTexMapID >(nextTexMap), CTexture::kCM_Repeat);
        next.CalculateLightmapMtx(areaXf, xf, bounds, texMtx + 2, mUVScale, mUVOffset);
        texCoordIds[5] = static_cast< GXTexCoordID >(nextCoord);
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(nextCoord++), GX_TG_MTX2x4, GX_TG_POS,
                            static_cast< GXTexMtx >(texMtx + 2), GX_FALSE, GX_PTIDENTITY);
        const float morphVal = darkLevel * water->GetMorphFactor();
        lightmapAlpha = (1.f - water->GetMorphFactor()) / (1.f - morphVal);
        CGX::SetTevKColor(GX_KCOLOR3, CColor(morphVal, morphVal, morphVal, 1.f).GetGXColor());
        hasDoubleLightmap = true;
      }
    }
    const float light = lightmapAlpha * darkLevel;
    CGX::SetTevKColor(GX_KCOLOR2, CColor(light, light, light, 1.f).GetGXColor());
  }

  const CVector3f normal = xf.TransposeRotate(CVector3f::Up());
  const CVector3f forward =
      CGraphics::GetViewMatrix().GetQuickInverse().TransposeRotate(CVector3f::Forward());
  const float viewDot = CMath::AbsF(CVector3f::Dot(normal, forward));
  const float gloss = (1.f - viewDot) * (mGlossAngle - mGlossFlat) + mGlossFlat;
  CGX::SetTevKColor(GX_KCOLOR0,
                    CColor(gloss, gloss, gloss, envMapType == 2 ? 1.f : alpha).GetGXColor());
  CGX::SetTevKColor(GX_KCOLOR1, mBaseColor.GetGXColor());
  CGX::SetNumTexGens(nextCoord);

  int stage = 0;
  if (hasLightmap) {
    CGX::SetTevOrder(GX_TEVSTAGE0, texCoordIds[4], texMapIds[4],
                     hasDoubleLightmap ? GX_COLOR_NULL : lightChannel);
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
                       hasDoubleLightmap ? GX_CC_ZERO : lightColor);
    CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K2);
    stage = 1;
    if (hasDoubleLightmap) {
      CGX::SetTevOrder(GX_TEVSTAGE1, texCoordIds[5], texMapIds[5], lightChannel);
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_C2, GX_CC_TEXC, GX_CC_KONST, lightColor);
      CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
      CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K3);
      stage = 2;
    }
  }
  if (HasColorMap()) {
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), texCoordIds[0], texMapIds[0],
                     GX_COLOR_NULL);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage), GX_TEV_KCSEL_K1);
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE,
                       GX_CC_KONST);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVREG1);
    if (HasColorWarpMap()) {
      GXSetIndTexOrder(GX_INDTEXSTAGE0, texCoordIds[1], texMapIds[1]);
      CGX::SetNumIndStages(1);
      CGX::SetTevIndirect(static_cast< GXTevStageID >(stage), GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU,
                          GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    }
    ++stage;
  }
  if (HasGlossMap()) {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage), GX_TEV_KCSEL_K0);
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), texCoordIds[2], texMapIds[2],
                     GX_COLOR_NULL);
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
                       GX_CC_ZERO);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage++), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), texCoordIds[3], texMapIds[2],
                     GX_COLOR_NULL);
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV,
                       envMapType == 0 && HasColorMap() ? GX_CC_C1 : GX_CC_ZERO);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage++), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2,
                       GX_TRUE, GX_TEVPREV);
  }
  if (envMapType != 0) {
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), texCoordIds[6], texMapIds[6],
                     GX_COLOR_NULL);
    const GXTevColorArg color = HasColorMap() ? GX_CC_C1 : GX_CC_ZERO;
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, GX_CC_TEXC,
                       HasGlossMap() ? GX_CC_CPREV : GX_CC_ONE, color);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage++), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
  }
  if (hasLightmap) {
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    const GXTevColorArg color = HasGlossMap() || envMapType != 0 ? GX_CC_CPREV
                                : HasColorMap()                  ? GX_CC_C1
                                                                 : GX_CC_ONE;
    CGX::SetTevColorIn(static_cast< GXTevStageID >(stage), GX_CC_ZERO, color, GX_CC_C2, GX_CC_ZERO);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage++), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
  }
  CGX::SetNumTevStages(stage);
  const GXTevStageID last = static_cast< GXTevStageID >(stage - 1);
  CGX::SetTevAlphaIn(last, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevAlphaOp(last, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetTevKAlphaSel(last, GX_TEV_KASEL_K0_A);
  CGX::SetBlendMode(alpha == 1.f ? GX_BM_NONE : GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                    GX_LO_CLEAR);
  CGX::SetZMode(GX_TRUE, GX_LEQUAL, mFluidType == 2);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCullMode(GX_CULL_NONE);
}

void CFluidPlaneCPU::Render(const CStateManager& mgr, float alpha, const CAABox& bounds,
                            const CTransform4f& xf, const CTransform4f& areaXf, TUniqueId waterId,
                            const char* gridFlags, int gridDimX, int gridDimY) const {
  if (!gkWaterEnable) {
    return;
  }
  const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(waterId));
  if (HasColorMap() || HasLightMap() || HasEnvMap() || HasGlossMap()) {
    RenderSetup(mgr, alpha, xf, areaXf, bounds, water);
    CGX::ResetVtxDescv();
    CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
    if (!mUseDynamicLights) {
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition3f32(bounds.GetMinPoint().GetX(), bounds.GetMinPoint().GetY(), 0.f);
      GXPosition3f32(bounds.GetMinPoint().GetX(), bounds.GetMaxPoint().GetY(), 0.f);
      GXPosition3f32(bounds.GetMaxPoint().GetX(), bounds.GetMinPoint().GetY(), 0.f);
      GXPosition3f32(bounds.GetMaxPoint().GetX(), bounds.GetMaxPoint().GetY(), 0.f);
      CGX::End();
    } else if (mDisplayListSize != 0) {
      GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_U8, 0);
      CGX::CallDisplayList(mDisplayList.get(), mDisplayListSize);
      GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    }
    RenderCleanup();
  }
}

void CFluidPlaneCPU::RenderCleanup() const {
  if (!gkWaterEnable) {
    return;
  }

  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX3x4, GX_TG_TEX2, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX3x4, GX_TG_TEX3, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD4, GX_TG_MTX3x4, GX_TG_TEX4, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD5, GX_TG_MTX3x4, GX_TG_TEX5, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD6, GX_TG_MTX3x4, GX_TG_TEX6, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
  CGX::SetTevDirect(GX_TEVSTAGE2);

  CGX::SetNumIndStages(0);

  CGX::ResetVtxDescv();

  CGX::SetChanCtrl(CGX::Channel1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_CLAMP,
                   GX_AF_SPOT);
  CGX::SetNumChans(1);

  CGraphics::SetLightState(CGraphics::GetLightMask());
  GXSetCullMode(GX_CULL_FRONT);
}

void CFluidPlaneCPU::PreRender(const CStateManager& mgr, const CVector2f& extent) {
  UpdateGridDisplayList(extent);
}

void CFluidPlaneCPU::UpdateGridDisplayList(const CVector2f& extent) {
  if (!mUseDynamicLights) {
    return;
  }
  const CVector2i dimensions(rstl::min_val(254, int(extent.GetX())),
                             rstl::min_val(254, int(extent.GetY())));
  if (dimensions == mGridDimensions) {
    return;
  }
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                        mDisplayList.release());
  mGridDimensions = dimensions;
  if (dimensions.GetX() == 0 || dimensions.GetY() == 0) {
    mDisplayListSize = 0;
    mDisplayList = nullptr;
    return;
  }

  const int width = rstl::min_val(255, dimensions.GetX() + 1);
  const int size = (width * 4 + 3) * dimensions.GetY();
  const uint alignedSize = (size + 31) & ~31;
  mDisplayList = static_cast< uchar* >(CMemory::Alloc(alignedSize, IAllocator::kHI_RoundUpLen));
  uchar* out = mDisplayList.get();
  for (uchar y = 0; y < dimensions.GetY(); ++y) {
    const uchar nextY = y + 1;
    *out++ = GX_TRIANGLESTRIP;
    *reinterpret_cast< ushort* >(out) = width * 2;
    out += 2;
    for (uchar x = 0; x < width; ++x) {
      out[0] = x;
      out[1] = y;
      out[2] = x;
      out[3] = nextY;
      out += 4;
    }
  }
  if (alignedSize - size != 0) {
    memset(out, 0, alignedSize - size);
  }
  DCFlushRange(mDisplayList.get(), alignedSize);
  mDisplayListSize = alignedSize;
}
