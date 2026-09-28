#include "Weapons/CDecal.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"

#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXVert.h"

CRandom16 CDecal::mDecalRandom(99);
bool CDecal::mMoveRedToAlphaBuffer = false;
bool CDecal::mDisableAlphaUpdate = false;
bool CDecal::mEnableClippedGeometry = true;

void CDecal::SetGlobalSeed(ushort seed) { mDecalRandom.SetSeed(seed); }

CDecal::CQuadDecal::CQuadDecal(int lifetime)
: mUseClippedGeometry(true)
, mLifetime(lifetime)
, mHalfSize(1.f)
, mRotation(0.f)
, mOffset(CVector3f::Zero())
, mInitialUV(CVector2f::Zero()) {}

void CDecal::InitQuad(CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc, int flag,
                      const CUnitVector3f& direction,
                      const rstl::vector< CCollisionSurface >& surfaces) {
  if (!desc.mTEX.null()) {
    if (!desc.mLFT.null()) {
      desc.mLFT->GetValue(0, quad.mLifetime);
    } else {
      quad.mLifetime = 0x7fffff;
    }
    if (!desc.mROT.null()) {
      quad.mUseClippedGeometry &= desc.mROT->IsConstant();
    }
    if (!desc.mSZE.null()) {
      quad.mUseClippedGeometry &= desc.mSZE->IsConstant();
      if (quad.mUseClippedGeometry) {
        float size = 1.f;
        desc.mSZE->GetValue(0, size);
        quad.mUseClippedGeometry = size <= 5.f;
      }
    }
    if (!desc.mOFF.null()) {
      quad.mUseClippedGeometry &= desc.mOFF->IsFastConstant();
    }
  } else {
    quad.mUseClippedGeometry = false;
    mFlags |= flag;
  }
  quad.mUseClippedGeometry &= mEnableClippedGeometry;
  if (quad.mUseClippedGeometry) {
    BuildClippedGeometry(quad, desc, direction, surfaces);
  }
}

CDecal::CDecal(const TToken< CDecalDescription >& desc, const CTransform4f& xf,
               const CUnitVector3f& direction, const rstl::vector< CCollisionSurface >& surfaces)
: mDescription(desc)
, mTransform(xf)
, mModelLifetime(0)
, mFrameIdx(0)
, mFlags(0)
, mRotation(CVector3f::Zero()) {
  CGlobalRandom gr(mDecalRandom);

  InitQuad(mQuad1, mDescription->mQuad1, 1, direction, surfaces);
  InitQuad(mQuad2, mDescription->mQuad2, 2, direction, surfaces);

  if (mDescription->mDMDL) {
    if (!mDescription->mDLFT.null()) {
      mDescription->mDLFT->GetValue(0, mModelLifetime);
    } else {
      mModelLifetime = 0x7FFFFF;
    }

    if (!mDescription->mDMRT.null()) {
      mDescription->mDMRT->GetValue(0, mRotation);
    }
  } else {
    mFlags |= 4;
  }
}

void CDecal::RenderQuad(CQuadDecal& decal, const CDecalDescription::SQuadDescr& desc) const {
  CColor color = CColor::White();
  if (CColorElement* clr = desc.mCLR.get()) {
    clr->GetValue(mFrameIdx, color);
  }
  if (CRealElement* sze = desc.mSZE.get()) {
    sze->GetValue(mFrameIdx, decal.mHalfSize);
    decal.mHalfSize *= 0.5f;
  }
  if (CRealElement* rot = desc.mROT.get()) {
    rot->GetValue(mFrameIdx, decal.mRotation);
  }
  if (CVectorElement* off = desc.mOFF.get()) {
    off->GetValue(mFrameIdx, decal.mOffset);
    decal.mOffset.SetY(0.f);
  }

  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  bool redToAlpha = CDecal::mMoveRedToAlphaBuffer && desc.mADD && !desc.mTEX.null();
  if (mDisableAlphaUpdate) {
    CGX::SetAlphaUpdate(false);
  }
  if (desc.mADD) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    if (redToAlpha) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    }
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  SUVElementSet uvSet;
  uvSet.xMin = 0.f;
  uvSet.xMax = 1.f;
  uvSet.yMin = 0.f;
  uvSet.yMax = 1.f;
  if (!desc.mTEX.null()) {
    TToken< CTexture > tex = desc.mTEX->GetValueTexture(mFrameIdx);
    if (!tex.IsLoaded()) {
      return;
    }
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    desc.mTEX->GetValueUV(mFrameIdx, uvSet);
    if (redToAlpha) {
      CGX::SetNumTevStages(2);
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_NEVER, 0);
      CGX::SetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(1);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  static const GXVtxDescList vtxDesc[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  if (decal.mUseClippedGeometry) {
    for (int i = 0; i < decal.mPolygons.size(); ++i) {
      decal.mPolygons[i].Render(color, CVector2f(uvSet.xMin, uvSet.yMin) - decal.mInitialUV);
    }
  } else {
    CTransform4f modXf = mTransform;
    modXf.AddTranslation(decal.mOffset);
    CGraphics::SetModelMatrix(modXf);
    CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    if (decal.mRotation == 0.f) {
      // Vertex 0
      GXPosition3f32(-decal.mHalfSize, 0.001f, decal.mHalfSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
      // Vertex 1
      GXPosition3f32(decal.mHalfSize, 0.001f, decal.mHalfSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
      // Vertex 2
      GXPosition3f32(-decal.mHalfSize, 0.001f, -decal.mHalfSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
      // Vertex 3
      GXPosition3f32(decal.mHalfSize, 0.001f, -decal.mHalfSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
    } else {
      const CRelAngle ang = CRelAngle::FromDegrees(decal.mRotation);
      const float sinSize = sine(ang) * decal.mHalfSize;
      const float cosSize = cosine(ang) * decal.mHalfSize;
      // Vertex 0
      GXPosition3f32(sinSize - cosSize, 0.001f, cosSize + sinSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
      // Vertex 1
      GXPosition3f32(sinSize + cosSize, 0.001f, cosSize - sinSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
      // Vertex 2
      GXPosition3f32(-(sinSize + cosSize), 0.001f, -(cosSize - sinSize));
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
      // Vertex 3
      GXPosition3f32(-sinSize + cosSize, 0.001f, -cosSize - sinSize);
      GXColor1u32(color.GetColor_u32());
      GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
    }

    CGX::End();
  }
  if (redToAlpha) {
    CGX::SetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  }
  if (mDisableAlphaUpdate) {
    CGX::SetAlphaUpdate(true);
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CDecal::RenderMdl() const {
  CColor color = CColor::White();
  CVector3f offset = CVector3f::Zero();
  CTransform4f rotXf(CTransform4f::Identity());
  if (!mDescription->mDMOO) {
    rotXf = mTransform.GetRotation();
  }

  bool dmrtIsConst = false;
  if (CVectorElement* off = mDescription->mDMRT.get()) {
    if (off->IsFastConstant()) {
      dmrtIsConst = true;
    }
  }

  CTransform4f dmrtXf = CTransform4f::Identity();
  if (dmrtIsConst) {
    mDescription->mDMRT->GetValue(mFrameIdx, mRotation);
    dmrtXf = CTransform4f::RotateZ(CRelAngle::FromDegrees(mRotation.GetZ()));
    dmrtXf.RotateLocalY(CRelAngle::FromDegrees(mRotation.GetY()));
    dmrtXf.RotateLocalX(CRelAngle::FromDegrees(mRotation.GetX()));
  }
  dmrtXf = rotXf * dmrtXf;

  if (CVectorElement* off = mDescription->mDMOP.get()) {
    off->GetValue(mFrameIdx, offset);
  }

  CTransform4f worldXf = CTransform4f::Translate(mTransform.GetTranslation() + rotXf * offset);
  if (dmrtIsConst) {
    worldXf *= dmrtXf;
  } else if (CVectorElement* dmrt = mDescription->mDMRT.get()) {
    CVector3f rotation = CVector3f::Zero();
    dmrt->GetValue(mFrameIdx, rotation);
    dmrtXf = CTransform4f::RotateZ(CRelAngle::FromDegrees(rotation.GetZ()));
    dmrtXf.RotateLocalY(CRelAngle::FromDegrees(rotation.GetY()));
    dmrtXf.RotateLocalX(CRelAngle::FromDegrees(rotation.GetX()));
    worldXf *= rotXf * dmrtXf;
  } else {
    worldXf *= dmrtXf;
  }

  if (CVectorElement* dmsc = mDescription->mDMSC.get()) {
    CVector3f scale = CVector3f::Zero();
    dmsc->GetValue(mFrameIdx, scale);
    worldXf *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
  }

  if (CColorElement* dmcl = mDescription->mDMCL.get()) {
    dmcl->GetValue(mFrameIdx, color);
  }

  CGraphics::SetModelMatrix(worldXf);

  if (mDescription->mDMAB) {
    const CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(true, false);
    (*mDescription->mDMDL)->Draw(flags);
  } else if (color.GetAlpha() == 1.f) {
    (*mDescription->mDMDL)->Draw(CModelFlags::Normal());
  } else {
    (*mDescription->mDMDL)->Draw(CModelFlags::AlphaBlended(color).DepthCompareUpdate(true, false));
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
}

void CDecal::Render() const {
  CGlobalRandom gr(mDecalRandom);
  if (IsDone()) {
    return;
  }

  CGraphics::DisableAllLights();
  CParticleGlobals::SetEmitterTime(mFrameIdx);

  if (!mDescription->mQuad1.mTEX.null() && !(mFlags & 1)) {
    CParticleGlobals::SetParticleLifetime(mQuad1.mLifetime);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(mFrameIdx);
    RenderQuad(mQuad1, mDescription->mQuad1);
  }
  if (!mDescription->mQuad2.mTEX.null() && !(mFlags & 2)) {
    CParticleGlobals::SetParticleLifetime(mQuad2.mLifetime);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(mFrameIdx);
    RenderQuad(mQuad2, mDescription->mQuad2);
  }
  if (mDescription->mDMDL && (mFlags & 4) == 0) {
    CParticleGlobals::SetParticleLifetime(mModelLifetime);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(mFrameIdx);
    RenderMdl();
  }
}

void CDecal::Update(float dt) {
  if (mFrameIdx >= mQuad1.mLifetime) {
    mFlags |= 1;
  }

  if (mFrameIdx >= mQuad2.mLifetime) {
    mFlags |= 2;
  }

  if (mFrameIdx >= mModelLifetime) {
    mFlags |= 4;
  }

  ++mFrameIdx;
}

void CDecal::CDecalPolygon::Render(const CColor& color, const CVector2f& uvOffset) const {
  if (mVertices.empty()) {
    return;
  }
  CGraphics::SetCullMode(kCM_None);
  const float near = CGraphics::GetDepthNear();
  const float far = CGraphics::GetDepthFar();
  const CGraphics::CProjectionState& projection = CGraphics::GetProjectionState();
  CGraphics::SetDepthRange(near, far - 0.1f / (projection.GetFar() - projection.GetNear()));
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT0, mVertices.size());
  for (int i = 0; i < mVertices.size(); ++i) {
    const SDecalVertex& vertex = mVertices[i];
    GXPosition3f32(vertex.mPosition.GetX(), vertex.mPosition.GetY(), vertex.mPosition.GetZ());
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(vertex.mUV.GetX() + uvOffset.GetX(), vertex.mUV.GetY() + uvOffset.GetY());
  }
  CGX::End();
  CGraphics::SetDepthRange(near, far);
}

void CDecal::BuildClippedGeometry(CQuadDecal& quad, const CDecalDescription::SQuadDescr& desc,
                                  const CUnitVector3f& direction,
                                  const rstl::vector< CCollisionSurface >& surfaces) {
  if (!desc.mSZE.null()) {
    desc.mSZE->GetValue(0, quad.mHalfSize);
    quad.mHalfSize *= 0.5f;
  }
  if (!desc.mROT.null()) {
    desc.mROT->GetValue(0, quad.mRotation);
  }
  if (!desc.mOFF.null()) {
    desc.mOFF->GetValue(0, quad.mOffset);
    quad.mOffset.SetY(0.f);
  }
  SUVElementSet uv = {0.f, 0.f, 1.f, 1.f};
  if (!desc.mTEX.null()) {
    desc.mTEX->GetValueUV(0, uv);
    quad.mInitialUV = CVector2f(uv.xMin, uv.yMin);
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  CTransform4f xf = mTransform;
  xf.AddTranslation(quad.mOffset);
  const CRelAngle angle = CRelAngle::FromDegrees(quad.mRotation);
  const float sinSize = sine(angle) * quad.mHalfSize;
  const float cosSize = cosine(angle) * quad.mHalfSize;
  rstl::vector< SDecalVertex > vertices;
  vertices.reserve(4);
  vertices.push_back(SDecalVertex(xf * CVector3f(-(sinSize + cosSize), 0.f, -(cosSize - sinSize)),
                                  CVector2f(uv.xMin, uv.yMin)));
  vertices.push_back(SDecalVertex(xf * CVector3f(sinSize - cosSize, 0.f, sinSize + cosSize),
                                  CVector2f(uv.xMin, uv.yMax)));
  vertices.push_back(SDecalVertex(xf * CVector3f(sinSize + cosSize, 0.f, cosSize - sinSize),
                                  CVector2f(uv.xMax, uv.yMax)));
  vertices.push_back(SDecalVertex(xf * CVector3f(cosSize - sinSize, 0.f, -cosSize - sinSize),
                                  CVector2f(uv.xMax, uv.yMin)));
  quad.mPolygons.clear();
  quad.mPolygons.reserve(surfaces.empty() ? 1 : surfaces.size());
  if (surfaces.empty()) {
    quad.mPolygons.push_back(CDecalPolygon(vertices));
    return;
  }

  const CPlane decalPlane(vertices[0].mPosition, vertices[1].mPosition, vertices[2].mPosition);
  const CUnitVector3f projectionDirection(direction - decalPlane.GetNormal());
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCollisionSurface& surface = surfaces[i];
    if (!(CVector3f::Dot(surface.GetNormal(), decalPlane.GetNormal()) > 0.3f ||
          CVector3f::Dot(surface.GetNormal(), -direction) > 0.6f)) {
      continue;
    }
    rstl::vector< SDecalVertex > projected;
    projected.reserve(4);
    for (int j = 0; j < 4; ++j) {
      const CVector3f displacement = 500.f * projectionDirection;
      CVector3f point = CVector3f::Zero();
      CollisionUtil::RayPlaneIntersection(vertices[j].mPosition - displacement,
                                          vertices[j].mPosition + displacement, surface.GetPlane(),
                                          point);
      projected.push_back(SDecalVertex(point, vertices[j].mUV));
    }
    rstl::vector< SDecalVertex > clipped;
    clipped.reserve(projected.size() + 3);
    for (int edge = 0; edge < 3 && !projected.empty(); ++edge) {
      const CPlane edgePlane = surface.GetEdgePlane(edge);
      const CPlane plane(-edgePlane.GetConstant(), -edgePlane.GetNormal());
      SDecalVertex previous = projected.back();
      float previousDistance = plane.GetHeight(previous.mPosition);
      for (int j = 0; j < projected.size(); ++j) {
        const SDecalVertex& current = projected[j];
        const float distance = plane.GetHeight(current.mPosition);
        if ((distance < 0.f && previousDistance > 0.f) ||
            (distance > 0.f && previousDistance < 0.f)) {
          const CVector3f delta = current.mPosition - previous.mPosition;
          const float t =
              -plane.GetHeight(previous.mPosition) / CVector3f::Dot(plane.GetNormal(), delta);
          clipped.push_back(SDecalVertex(previous.mPosition + t * delta,
                                         previous.mUV + (current.mUV - previous.mUV) * t));
        }
        if (!(distance > 0.f)) {
          clipped.push_back(current);
        }
        previous = current;
        previousDistance = distance;
      }
      projected = clipped;
      clipped.clear();
    }
    quad.mPolygons.push_back(CDecalPolygon(projected));
  }
}
