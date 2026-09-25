#include "Kyoto/Graphics/CCubeMaterial.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGX_Impl.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include <dolphin/mtx.h>
#include <string.h>

extern "C" int fn_8033B70C(uint vtxDesc, int attr);
extern "C" const float (*fn_8033A44C())[4];

typedef void (*TTevHandler)(const uint*& materialData, uint firstTev, uint& tevCount,
                            uint& tcgCount);

static void HandleThermalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                              uint& tcgCount);
static void HandleNormalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                             uint& tcgCount);
static void SetupAlphaMaskVtxDesc(uint vtxDesc);

static const float gkEpsilon32 = FLT_EPSILON;

static CVector3f sPlayerPosition(CVector3f::Zero());
CVector3f CCubeMaterial::sViewingFrom(0.f, 0.f, 0.f);
static CTransform4f sTextureProjectionTransform(CTransform4f::Identity());
int sLastMaterialUnique = -1;
static float sThrobX = 1.f;
static float sThrobY = 1.f;
static int sReflectionStage = -1;
static int sAlphaMaskPostTexMtx = GX_PTIDENTITY;
static int sAlphaMaskTexCoord = GX_TEXCOORD_NULL;
static int sTransparencyKColor = -1;
static TTevHandler sTevHandler = HandleNormalTevs;
const CCubeModel* CCubeMaterial::sLastModelCached = nullptr;
const CCubeModel* CCubeMaterial::sRenderingModel = nullptr;
static int sMaterialCachedState = 0;
const uchar* sLastMaterialCached = nullptr;
static bool sKColorModulated = false;
static float sReflectionAlpha = 0.f;
static float sLastTime = 0.f;
static bool sbRenderModelBlack = false;
static bool sbRenderModelBlackKonst = false;
static bool sbRenderModelShadow = false;
static CTexture* spShadowTexture = nullptr;
static uchar sChannel0DisableLightMask = 0;
static uchar sChannel1EnableLightMask = 0;
static const GXColor sGXBlack = {0, 0, 0, 255};
static const GXColor sGXWhite = {0xFF, 0xFF, 0xFF, 0xFF};

static const Mtx sEnvPostMtx = {
    {0.5f, 0.0f, 0.0f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};
static Mtx sScrollTexMtx = {
    {1.f, 0.f, 0.f, 0.f},
    {0.f, 1.f, 0.f, 0.f},
    {0.f, 0.f, 1.f, 0.f},
};

struct SMtx {
  Mtx m;
};

union scanner_t {
  const uint* words;
  const uchar* bytes;
};

void CCubeMaterial::SetupBlendMode(const uint blendFactors, const CModelFlags& flags,
                                   bool alphaTest) {
  GXBlendFactor newSrcFactor = static_cast< GXBlendFactor >(blendFactors & 0xFFFF);
  GXBlendFactor newDstFactor = static_cast< GXBlendFactor >(blendFactors >> 0x10);
  CModelFlags::ETrans blendMode = flags.GetTrans();

  GXCompare alphaCompare;
  if (alphaTest) {
    alphaCompare = GX_GEQUAL;
    newSrcFactor = GX_BL_ONE;
    newDstFactor = GX_BL_ZERO;
  } else {
    alphaCompare = GX_ALWAYS;
  }
  CGX::SetAlphaCompare(alphaCompare, 64, GX_AOP_AND, GX_ALWAYS, 0);

  if (blendMode > 4 && newSrcFactor == GX_BL_ONE) {
    newSrcFactor = GX_BL_SRCALPHA;
    if (newDstFactor == GX_BL_ZERO) {
      newDstFactor = blendMode > 6 ? GX_BL_ONE : GX_BL_INVSRCALPHA;
    }
  }
  CGX::SetBlendMode(GX_BM_BLEND, newSrcFactor, newDstFactor, GX_LO_CLEAR);
}

static void HandleTev(int tevCur, const uint* materialDataCur, const uint* texMapTexCoordFlags,
                      bool shadowMapsEnabled) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
  const uint colorArgs = shadowMapsEnabled ? 0x7a04f : materialDataCur[0];
  const uint alphaArgs = materialDataCur[1];
  const uint colorOps = materialDataCur[2];
  const uint alphaOps = materialDataCur[3];

  CGX::SetStandardDirectTev_Compressed(stage, colorArgs, alphaArgs, colorOps, alphaOps);

  uint tmtcFlags = *texMapTexCoordFlags;
  uint matFlags = materialDataCur[4];
  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(tmtcFlags & 0xFF),
                   static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   static_cast< GXChannelID >(matFlags & 0xFF));
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
}

static uint HandleThermalTev(int tevCur, const uint* materialDataCur,
                             const uint* texMapTexCoordFlags) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
  switch (materialDataCur[0]) {
  case 0x7b84f:
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ONE, GX_CC_KONST, GX_CC_ZERO);
    break;
  default:
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_TEXC, GX_CC_ZERO);
    break;
  }
  CGX::SetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
  CGX::SetTevAlphaIn_Compressed(stage, materialDataCur[1]);
  CGX::SetTevAlphaOp_Compressed(stage, materialDataCur[3]);

  uint tmtcFlags = *texMapTexCoordFlags;
  uint matFlags = materialDataCur[4];
  CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   GX_COLOR_NULL);
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
  return tmtcFlags & 0xFF;
}

static uint HandleUVInvModelView(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
      CGraphics::GetModelMatrix());
  xf.SetTranslation(CVector3f::Zero());
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
  CGX::LoadTexMtxImm(sEnvPostMtx, ptTexMtx, GX_MTX3x4);
  return 1;
}

static uint HandleUVInvModelViewTranslated(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
  CGX::LoadTexMtxImm(sEnvPostMtx, ptTexMtx, GX_MTX3x4);
  return 1;
}

static uint HandleUVScroll(const float* params, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
  const float f3 = params[2];
  const float f4 = params[3];
  const float seconds = CGraphics::GetSecondsMod900();
  sScrollTexMtx[0][3] = f1 + seconds * f3;
  sScrollTexMtx[1][3] = f2 + seconds * f4;
  CGX::LoadTexMtxImm(sScrollTexMtx, texMtx, GX_MTX3x4);
  return 5;
}

static uint HandleUVRotation(const float* params, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
  const float seconds = CGraphics::GetSecondsMod900();
  const float angle = f1 + seconds * f2;
  const float asin = CMath::FastSinR(angle);
  const float acos = CMath::FastCosR(angle);
  Mtx mtx = {
      {acos, -asin, 0.f, 0.5f * (1.f - (acos - asin))},
      {asin, acos, 0.f, 0.5f * (1.f - (asin + acos))},
      {0.f, 0.f, 1.f, 0.f},
  };
  CGX::LoadTexMtxImm(mtx, texMtx, GX_MTX3x4);
  return 3;
}

static uint HandleUVFilmstrip(const float* params, uint type, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
  const float f3 = params[2];
  const float f4 = params[3];
  const float value = (f4 + CGraphics::GetSecondsMod900()) * f1 * f3;
  const float fmod = CMath::FastFmod(value, 1.f);
  const float fs = CCast::FtoS(fmod * f2);
  const float v2 = fs * f3;
  if (type == 4) {
    sScrollTexMtx[0][3] = v2;
    sScrollTexMtx[1][3] = 0.f;
  } else {
    sScrollTexMtx[0][3] = 0.f;
    sScrollTexMtx[1][3] = v2;
  }
  CGX::LoadTexMtxImm(sScrollTexMtx, texMtx, GX_MTX3x4);
  return 5;
}

static uint HandleUVModelMatrix(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const SMtx sPtMtx = {{
      {0.5f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.5f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  }};
  CTransform4f xf = CGraphics::GetModelMatrix();
  xf.SetTranslation(CVector3f::Zero());
  SMtx tmpPtMtx = sPtMtx;
  tmpPtMtx.m[0][3] = CGraphics::GetModelMatrix().Get03() * 0.05f;
  tmpPtMtx.m[1][3] = CGraphics::GetModelMatrix().Get13() * 0.05f;
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
  CGX::LoadTexMtxImm(tmpPtMtx.m, ptTexMtx, GX_MTX3x4);
  return 1;
}

static uint HandleUVCylinder(const float* params, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const SMtx sPtMtx = {{
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  }};
  const CTransform4f& vm = CGraphics::GetViewMatrix();
  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
      CGraphics::GetModelMatrix());
  xf.SetTranslation(CVector3f::Zero());
  SMtx tmpPtMtx = sPtMtx;
  float scale = params[0];
  scale = 0.5f * scale;
  tmpPtMtx.m[0][0] = scale;
  tmpPtMtx.m[0][3] = CMath::FastFmod(0.025f * (vm.Get03() + vm.Get13()) * params[1], 1.f);
  tmpPtMtx.m[1][2] = scale;
  tmpPtMtx.m[1][3] = CMath::FastFmod(0.05f * vm.Get23() * params[1], 1.f);
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
  CGX::LoadTexMtxImm(tmpPtMtx.m, ptTexMtx, GX_MTX3x4);
  return 3;
}

static uint HandleAnimatedUV(const uint* uvAnim, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  const uint type = *uvAnim;
  switch (type) {
  case 0:
    return HandleUVInvModelView(texMtx, ptTexMtx);
  case 1:
    return HandleUVInvModelViewTranslated(texMtx, ptTexMtx);
  case 2:
    return HandleUVScroll(reinterpret_cast< const float* >(uvAnim + 1), texMtx);
  case 3:
    return HandleUVRotation(reinterpret_cast< const float* >(uvAnim + 1), texMtx);
  case 4:
  case 5:
    return HandleUVFilmstrip(reinterpret_cast< const float* >(uvAnim + 1), type, texMtx);
  case 6:
    return HandleUVModelMatrix(texMtx, ptTexMtx);
  case 7:
    return HandleUVCylinder(reinterpret_cast< const float* >(uvAnim + 1), texMtx, ptTexMtx);
  default:
    return 0;
  }
}

static void HandleAlphaMask(uint vtxDesc, uint& tevCount, uint& texCount, uint& tcgCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
  sAlphaMaskTexCoord = tcgCount;
  const CTexture& texture = CCubeRenderer::That()->GetAlphaMaskRamp();
  sAlphaMaskPostTexMtx = tcgCount * 3 + GX_PTTEXMTX0;

  GXTexObj texObj;
  GXInitTexObj(&texObj, const_cast< void* >(texture.GetConstBitMapData(0)), texture.GetWidth(),
               texture.GetHeight(), GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_FALSE, GX_FALSE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texMap);
  CTexture::InvalidateTexmap(texMap);

  CGX::SetNumTevStages(tevCount + 1);
  CGX::SetNumTexGens(tcgCount + 1);
  CGX::SetStandardTevColorAlphaOp(stage);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(sAlphaMaskTexCoord), texMap, GX_COLOR_NULL);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGX::LoadTexMtxImm(fn_8033A44C(), sAlphaMaskPostTexMtx, GX_MTX3x4);
  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(sAlphaMaskTexCoord), GX_TG_MTX3x4, GX_TG_POS,
                      static_cast< GXTexMtx >(GX_PNMTX0), GX_FALSE,
                      static_cast< GXPTTexMtx >(sAlphaMaskPostTexMtx));
  SetupAlphaMaskVtxDesc(vtxDesc);

  ++tevCount;
  ++texCount;
  ++tcgCount;
}

static void SetupAlphaMaskVtxDesc(uint vtxDesc) {
  CGX::SetVtxDescv_Compressed(vtxDesc);
  if (fn_8033B70C(vtxDesc, GX_VA_TEX6MTXIDX) == GX_DIRECT && sAlphaMaskTexCoord < 8) {
    CGX::SetVtxDesc(GX_VA_TEX6MTXIDX, GX_NONE);
    CGX::SetVtxDesc(static_cast< GXAttr >(sAlphaMaskTexCoord + GX_VA_TEX0MTXIDX), GX_DIRECT);
  }
}

static void ModulateKColor(const CModelFlags& flags) {
  CGX::SetTevKColor(GX_KCOLOR0,
                    CColor::Modulate(flags.GetColor(), reinterpret_cast< const CColor& >(
                                                           CGX::GetTevKColor(GX_KCOLOR0)))
                        .GetGXColor());
}

static bool TryModulateKColor(uint tevCount, uint& kColorCount, const CModelFlags& flags) {
  const CModelFlags::ETrans blendMode = flags.GetTrans();
  if (blendMode != CModelFlags::kT_Additive && blendMode != CModelFlags::kT_Blend) {
    return false;
  }
  if (tevCount != 1) {
    return false;
  }
  if (kColorCount == 1) {
    const CGX::STevState& state = CGX::GetTevState(GX_TEVSTAGE0);
    if (state.mColorInArgs == 0x7b94f &&
        (state.mAlphaInArgs == 0x390c7 || state.mAlphaInArgs == 0x31ce7)) {
      ModulateKColor(flags);
      sKColorModulated = true;
      return true;
    }
  }
  return false;
}

static void HandleTransparency(uint& finalTevCount, uint& finalKColorCount,
                               const CModelFlags& modelFlags, uint blendFactors) {
  if ((modelFlags.GetOtherFlags() & CModelFlags::kF_Unknown400) != 0 &&
      TryModulateKColor(finalTevCount, finalKColorCount, modelFlags)) {
    return;
  }

  const CModelFlags::ETrans blendMode = modelFlags.GetTrans();
  const CColor color = modelFlags.GetColor();

  if (blendMode == 2) {
    if (static_cast< GXBlendFactor >(blendFactors >> 16) == 1) {
      return;
    }
  }

  if (blendMode == 3) {
    const uint stage = finalTevCount;
    const uint stage2 = stage + 1;
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), 0x73def);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage), 0x1ce7);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVREG0);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0_A));
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));

    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage2), 0x7b840);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage2), 0x1ce7);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage2));
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage2));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage2), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());

    finalKColorCount += 1;
    finalTevCount += 2;
  } else {
    uint alphaArgs = 0x380c7;
    if (blendMode == 8) {
      alphaArgs = 0x31ce7;
    }

    uint colorArgs = 0x781cf;
    if (blendMode == 2) {
      colorArgs = 0x7018f;
    }

    const uint stage = finalTevCount;
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), colorArgs);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage), alphaArgs);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage));
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    sTransparencyKColor = finalKColorCount;
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKAlphaSel >(finalKColorCount + GX_TEV_KASEL_K0_A));

    finalTevCount += 1;
    finalKColorCount += 1;
  }
}

static void DoModelShadow(uint texCount, uint tcgCount) {
  static const SMtx identity2D = {{
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  }};

  spShadowTexture->Load(static_cast< GXTexMapID >(texCount), CTexture::kCM_Repeat);

  SMtx mtx = identity2D;
  mtx.m[0][0] = sTextureProjectionTransform.Get00();
  mtx.m[0][1] = sTextureProjectionTransform.Get01();
  mtx.m[0][2] = sTextureProjectionTransform.Get02();
  mtx.m[0][3] = sTextureProjectionTransform.Get03();
  mtx.m[1][0] = sTextureProjectionTransform.Get20();
  mtx.m[1][1] = sTextureProjectionTransform.Get21();
  mtx.m[1][2] = sTextureProjectionTransform.Get22();
  mtx.m[1][3] = sTextureProjectionTransform.Get23();
  CGX::LoadTexMtxImm(mtx.m, GX_TEXMTX5, GX_MTX3x4);

  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX5,
                      GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetTevOrder(GX_TEVSTAGE0, static_cast< GXTexCoordID >(tcgCount),
                   static_cast< GXTexMapID >(texCount), GX_COLOR1A1);

  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_C0);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
}

uint CCubeMaterial::HandleReflection(const GXTexMapID indTexSlot, const int indMtxScaleExp,
                                     const uint tevCount, const uint texCount,
                                     const uint tcgCount, const uint kColorCount) {
  bool usesTevReg2 = false;
  for (uint i = 0; i < tevCount; ++i) {
    if ((CGX::GetTevState(static_cast< GXTevStageID >(i)).mColorOps >> 9 & 3) == GX_TEVREG2) {
      usesTevReg2 = true;
      break;
    }
  }

  uint out = 0;
  GXTevStageID finalTevCount = static_cast< GXTevStageID >(tevCount);
  GXTevColorArg colorArg;
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
  if (usesTevReg2) {
    colorArg = GX_CC_C2;
    CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, GX_CC_C2, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    CGX::SetTevColorOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevOrder(finalTevCount, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_ZERO);
    out = 1;
  } else {
    colorArg = GX_CC_KONST;
  }

  CColor reflectionColor;
  reflectionColor.Set(sReflectionAlpha, sReflectionAlpha, sReflectionAlpha, sReflectionAlpha);
  CGX::SetTevKColor(finalKColorCount, reflectionColor.GetGXColor());
  CGX::SetTevKColorSel(finalTevCount,
                       static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

  finalTevCount = static_cast< GXTevStageID >(finalTevCount + out);

  CCubeRenderer::That()->GetRealReflection()->Load(texMap, CTexture::kCM_Clamp);

  GXTexCoordID texCoord;
  if (indTexSlot != GX_TEXMAP_NULL) {
    texCoord = static_cast< GXTexCoordID >(tcgCount + 1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, static_cast< GXTexCoordID >(tcgCount), indTexSlot);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX6,
                        GX_TRUE, GX_PTTEXMTX6);
    CGX::SetTevIndWarp(finalTevCount, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    CGX::SetIndTexMtxSTPointFive(GX_ITM_0, static_cast< s8 >(indMtxScaleExp));
    if (sReflectionStage > static_cast< int >(finalTevCount)) {
      CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
    }
    sReflectionStage = finalTevCount;
  } else {
    texCoord = static_cast< GXTexCoordID >(tcgCount);
  }

  CGX::SetTexCoordGen(texCoord, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX7, GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, colorArg, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(finalTevCount);
  CGX::SetTevOrder(finalTevCount, texCoord, texMap, GX_COLOR_NULL);
  return out + 1;
}

void CCubeMaterial::EnsureTevsDirect() {
  if (sReflectionStage == -1) {
    return;
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
  sReflectionStage = -1;
}

void CCubeMaterial::SetCurrentBlack() const {
  const uint* data = reinterpret_cast< const uint* >(mData);
  const uint texCount = data[1];
  const uint flags = data[0];
  const uint vertexDesc = data[texCount + 2];

  if ((flags & (kStateFlag_DepthSorting | kStateFlag_AlphaTest)) != 0) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }

  CGX::SetVtxDescv_Compressed(vertexDesc);

  GXTevColorArg colorArg = GX_CC_ZERO;
  if (sbRenderModelBlackKonst) {
    colorArg = GX_CC_ONE;
  }
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, colorArg);

  GXTevAlphaArg alphaArg = GX_CA_ZERO;
  if (sbRenderModelBlackKonst) {
    alphaArg = GX_CA_KONST;
  }
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, alphaArg);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumIndStages(0);
}

static uint HandleColorChannels(uint chanCount, uint firstChan) {
  if (sbRenderModelShadow) {
    if (chanCount != 0) {
      CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
      CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
      CGX::SetChanCtrl(CGX::Channel1, true, GX_SRC_REG, GX_SRC_REG,
                       static_cast< GXLightID >(sChannel1EnableLightMask), GX_DF_CLAMP, GX_AF_SPOT);

      const uchar chan0Lights = CGraphics::GetLightMask() & ~sChannel0DisableLightMask;
      CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(chan0Lights), firstChan);
      if (chan0Lights != 0) {
        CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
      } else {
        CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
      }
    }
    return 2;
  }

  if (chanCount == 2) {
    CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
    CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
  } else {
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  if (chanCount >= 1) {
    const uchar lightMask = CGraphics::GetLightMask();
    CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(lightMask), firstChan);
    if (lightMask != 0) {
      CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
    } else {
      CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
    }
  } else {
    CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  return chanCount;
}

static void HandleDepth(uint modelFlags, uint matFlags) {
  GXCompare func;
  if ((modelFlags & CModelFlags::kF_DepthCompare) == 0) {
    func = GX_ALWAYS;
  } else if ((modelFlags & CModelFlags::kF_Unknown200) == 0) {
    func = GX_LEQUAL;
  } else if ((modelFlags & CModelFlags::kF_DepthGreater) != 0) {
    if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
      func = GX_GREATER;
    } else {
      func = GX_GEQUAL;
    }
  } else if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
    func = GX_LESS;
  } else {
    func = GX_EQUAL;
  }
  CGX::SetZMode(true, func,
                (modelFlags & CModelFlags::kF_DepthUpdate) == CModelFlags::kF_DepthUpdate &&
                    (matFlags & kStateFlag_DepthWrite) != 0);
}

static void DoPassthru(const uint finalTevCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(finalTevCount);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevDirect(stage);
  CGX::SetStandardTevColorAlphaOp(stage);
}

static void HandleNormalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                             uint& tcgCount) {
  scanner_t materialDataCur;
  materialDataCur.words = materialData;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += tevCount * 20;
  const uint finalTevCount = tevCount + firstTev;
  for (uint i = firstTev; i < finalTevCount; ++i) {
    HandleTev(i, materialDataCur.words, texMapTexCoordFlags.words,
              sbRenderModelShadow && i == firstTev);
    materialDataCur.words += 5;
    texMapTexCoordFlags.words += 1;
  }

  scanner_t uvAnim;
  uvAnim = texMapTexCoordFlags;
  const uint finalTcgCount = *uvAnim.words;
  for (uint i = 0; i < finalTcgCount; ++i) {
    CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i), uvAnim.words[i + 1]);
  }
  uvAnim.words += finalTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + *uvAnim.words + 4;
  const uint animCount = uvAnim.words[1];
  uvAnim.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(uvAnim.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    uvAnim.words += size;
  }

  materialData = end.words;
  tevCount = finalTevCount;
  tcgCount = finalTcgCount;
}

static void HandleThermalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                              uint& tcgCount) {
  scanner_t materialDataCur;
  materialDataCur.words = materialData;
  const uint matTevCount = tevCount;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += matTevCount * 20;
  scanner_t savedTexMapTexCoordFlags;
  savedTexMapTexCoordFlags = texMapTexCoordFlags;
  if ((materialDataCur.words[2] >> 9 & 3) == GX_TEVREG0) {
    materialDataCur.words += 5;
    texMapTexCoordFlags.words += 1;
  }
  const int texCoord =
      HandleThermalTev(firstTev, materialDataCur.words, texMapTexCoordFlags.words);

  scanner_t uvAnim;
  uvAnim.words = savedTexMapTexCoordFlags.words + matTevCount;
  const uint fullTcgCount = *uvAnim.words;
  if (texCoord != GX_TEXCOORD_NULL) {
    for (int i = 0; i < fullTcgCount; ++i) {
      if (texCoord == i) {
        CGX::SetTexCoordGen_Compressed(GX_TEXCOORD0, uvAnim.words[i + 1]);
        break;
      }
    }
  }
  uvAnim.words += fullTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + *uvAnim.words + 4;
  const uint animCount = uvAnim.words[1];
  uvAnim.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(uvAnim.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    uvAnim.words += size;
  }

  materialData = end.words;
  tevCount = firstTev + 1;
  tcgCount = 1;
}

void CCubeMaterial::SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                               const CCubeModel& model) const {
  if (mData == sLastMaterialCached) {
    switch (sMaterialCachedState) {
    case 1:
      if (sLastModelCached == sRenderingModel) {
        return;
      }
      break;
    case 2:
      break;
    default:
      return;
    }
  }

  if (sbRenderModelBlack) {
    SetCurrentBlack();
    return;
  }

  const uint* materialDataCur = reinterpret_cast< const uint* >(GetData());
  uint numIndStages = 0;
  sLastMaterialCached = GetData();
  GXTexMapID indTexSlot = GX_TEXMAP0;
  const uint matFlags = materialDataCur[0];
  uint texCount = materialDataCur[1];
  const bool reflection =
      (matFlags & (kStateFlag_Reflection | kStateFlag_ReflectionSurfaceEye)) != 0;
  sRenderingModel = &model;
  if (reflection) {
    EnsureViewDepStateCached((matFlags & kStateFlag_ReflectionSurfaceEye) != 0 ? &surface
                                                                               : nullptr);
  } else {
    sMaterialCachedState = 0;
  }

  if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) == 0) {
    const rstl::vector< TCachedToken< CTexture > >& textures = model.GetTextures();
    materialDataCur += 2;
    for (uint i = 0; i < texCount; ++i) {
      textures[*materialDataCur].GetObject()->Load(static_cast< GXTexMapID >(i),
                                                   CTexture::kCM_Repeat);
      ++materialDataCur;
    }
  } else {
    materialDataCur += texCount + 2;
  }

  const uint vertexDesc = *materialDataCur++;
  materialDataCur += 2;
  const int groupIdx = static_cast< int >(*materialDataCur++);

  uint finalKColorCount = 0;
  if ((matFlags & kStateFlag_KonstValues) != 0) {
    finalKColorCount = materialDataCur[0];
    for (uint i = 0; i < finalKColorCount; ++i) {
      CGX::SetTevKColor(static_cast< GXTevKColorID >(i), CColor::ToGX(materialDataCur[i + 1]));
    }
    materialDataCur += finalKColorCount + 1;
  }

  if (sLastMaterialUnique != -1 && sLastMaterialUnique == groupIdx && sMaterialCachedState == 0) {
    if (sKColorModulated) {
      ModulateKColor(flags);
    }
    return;
  }
  sLastMaterialUnique = groupIdx;
  sKColorModulated = false;

  CGX::SetVtxDescv_Compressed(vertexDesc);

  const bool packedLightmaps = (matFlags & kStateFlag_LightmapUvArray) != 0;
  if (packedLightmaps != CCubeModel::IsUsingPackedLightmaps()) {
    model.SetUsingPackedLightmaps(packedLightmaps);
  }

  const uint blendFactors = *materialDataCur;
  SetupBlendMode(blendFactors, flags, (matFlags & kStateFlag_AlphaTest) != 0);

  ++materialDataCur;
  const bool indTex = (matFlags & kStateFlag_ReflectionIndirectTexture) != 0;
  if (indTex) {
    indTexSlot = static_cast< GXTexMapID >(*materialDataCur++);
  }

  HandleDepth(flags.GetOtherFlags(), matFlags);

  const uint chanCount = materialDataCur[0];
  const uint firstChan = materialDataCur[1];
  materialDataCur += chanCount + 1;
  const uint finalNumColorChans = HandleColorChannels(chanCount, firstChan);

  uint firstTev = 0;
  if (sbRenderModelShadow) {
    firstTev = 2;
  }

  uint finalTevCount = *materialDataCur++;
  uint tcgCount = 0;
  sTevHandler(materialDataCur, firstTev, finalTevCount, tcgCount);

  if (flags.GetTrans() != CModelFlags::kT_Opaque) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors);
  }

  if ((flags.GetOtherFlags() & CModelFlags::kF_Unknown80) != 0) {
    HandleAlphaMask(vertexDesc, finalTevCount, texCount, tcgCount);
  }

  if (reflection) {
    if (sReflectionAlpha > 0.f) {
      uint addedTevs = 0;
      if (indTex) {
        addedTevs = HandleReflection(static_cast< GXTexMapID >(indTexSlot & 7), 0, finalTevCount,
                                     texCount, tcgCount, finalKColorCount);
        numIndStages = 1;
        tcgCount += 2;
      } else {
        addedTevs = HandleReflection(GX_TEXMAP_NULL, 0, finalTevCount, texCount, tcgCount,
                                     finalKColorCount);
        tcgCount += 1;
      }
      finalTevCount += addedTevs;
      texCount += 1;
      finalKColorCount += 1;
    } else if (finalTevCount != 0 &&
               (CGX::GetTevState(static_cast< GXTevStageID >(finalTevCount - 1)).mColorOps >>
                    9 &
                3) != 0) {
      DoPassthru(finalTevCount);
      finalTevCount += 1;
    }
  }

  if (sbRenderModelShadow) {
    DoModelShadow(texCount, tcgCount);
    texCount += 1;
    tcgCount += 1;
  }

  CGX::SetNumIndStages(numIndStages);
  CGX::SetNumTevStages(finalTevCount);
  CGX::SetNumTexGens(tcgCount);
  CGX::SetNumChans(finalNumColorChans);
}

void CCubeMaterial::EnsureViewDepStateCached(const CCubeSurface* surface) {
  static const Mtx texMtx1 = {
      {0.5f, 0.f, 0.f, 0.5f},
      {0.f, 0.f, 0.5f, 0.5f},
      {0.f, 0.f, 0.f, 1.f},
  };
  static const SMtx texMtx2 = {{
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  }};

  if ((surface == nullptr && sLastModelCached == sRenderingModel) || sRenderingModel == nullptr) {
    return;
  }

  const CTransform4f& modelMtx = CGraphics::GetModelMatrix();
  const CVector3f& playerPos =
      modelMtx.TransposeRotate(sPlayerPosition - modelMtx.GetTranslation());
  CVector3f points[2];
  points[1] = playerPos;
  sLastModelCached = sRenderingModel;
  points[0] = CVector3f::Zero();
  CVector3f& modelPoint = points[0];
  CVector3f& playerPoint = points[1];
  float radius = 0.f;

  if (surface != nullptr) {
    sMaterialCachedState = 2;

    const CPlane plane(surface->GetCenter(), surface->GetNormalHint());
    modelPoint = playerPoint - plane.GetHeight(playerPoint) * plane.GetNormal();
  } else {
    sMaterialCachedState = 1;

    const CAABox& bounds = sRenderingModel->GetBoundingBox();
    modelPoint = bounds.GetCenterPoint();
    modelPoint.SetZ(playerPoint.GetZ());
    radius = 0.5f * (bounds.GetWidth() + bounds.GetHeight());
  }

  CCubeRenderer* renderer = CCubeRenderer::That();
  if (renderer->GetReflectionFlag()) {
    const CVector3f& oldDelta = sViewingFrom - sPlayerPosition;
    const CVector3f& newDelta = modelPoint - sPlayerPosition;
    const float oldMag = oldDelta.MagSquared();
    const float newMag = newDelta.MagSquared();
    if (newMag < oldMag) {
      sViewingFrom = modelPoint;
    }
  } else {
    sViewingFrom = modelPoint;
    renderer->SetReflectionFlag();
  }

  const CVector3f distVec = modelPoint - playerPoint;
  const float dist = distVec.Magnitude();
  const float reflDist = CMath::Max(gkEpsilon32, dist - 0.5f * radius);

  if (reflDist >= 5.f) {
    sReflectionAlpha = 0.f;
    return;
  }

  sReflectionAlpha = (5.f - reflDist) / 5.f;

  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), GX_TEXMTX6, GX_MTX3x4);
  CGX::LoadTexMtxImm(texMtx1, GX_PTTEXMTX6, GX_MTX3x4);

  CVector3f dir = distVec / reflDist;
  CVector3f right = CVector3f::Cross(dir, CVector3f(0.f, 0.f, 1.f));
  float xScale = 0.32258067f;
  float yScale = 0.32258067f;
  if (right.CanBeNormalized()) {
    right.Normalize();
  } else {
    dir = CVector3f::Forward();
    right = CVector3f::Right();
  }

  const float scale = 0.02f * reflDist + 1.f;
  xScale *= scale * sThrobX;
  yScale *= scale * sThrobY;

  SMtx texMtx = texMtx2;
  texMtx.m[0][0] = xScale * right.GetX();
  texMtx.m[0][1] = xScale * right.GetY();
  texMtx.m[0][3] = -CVector3f::Dot(modelPoint, right) * xScale + 0.5f;
  texMtx.m[1][2] = yScale;
  texMtx.m[1][3] = -playerPoint.GetZ() * yScale;
  CGX::LoadTexMtxImm(texMtx.m, GX_TEXMTX7, GX_MTX3x4);
}

void CCubeMaterial::KillCachedViewDepState() {
  sLastModelCached = nullptr;
  sMaterialCachedState = 0;
}

void CCubeMaterial::ResetCachedMaterials() {
  KillCachedViewDepState();
  sRenderingModel = nullptr;
  sLastMaterialCached = nullptr;
  sLastMaterialUnique = -1;
  sKColorModulated = false;
}

uint CCubeMaterial::GetCompressedBlend() const {
  const uint* ptr = reinterpret_cast< const uint* >(mData);
  const uint flags = ptr[0];
  const uint texCount = ptr[1];
  const uint* blend = ptr + texCount + 6;
  if ((flags & kStateFlag_KonstValues) != 0) {
    blend += *blend + 1;
  }
  return *blend;
}

uint CCubeMaterial::GetExtraPostTexMtx() { return sAlphaMaskPostTexMtx; }

uint CCubeMaterial::GetExtraTexCoord() { return sAlphaMaskTexCoord; }

void CCubeMaterial::ResetExtraTexCoord() { sAlphaMaskTexCoord = GX_TEXCOORD_NULL; }

void CCubeModel::SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch) {
  sPlayerPosition = pos;
  CCubeMaterial::KillCachedViewDepState();

  s64 millis = stopwatch.GetCurrMicros() / 1000;

  float frequency = 1.5f;
  float timeWrapScale = 100000.f;
  float period = M_2PIF / frequency;
  const float time = static_cast< float >(static_cast< uint >(
                         millis % static_cast< uint >(timeWrapScale * period))) /
                     1000.f;
  sLastTime = time;

  float throbAmplitudeX = 0.05f;
  float throbAmplitudeY = 0.015f;
  float phaseX = 0.f;
  float phaseY = 1.f;
  sThrobX = 1. / (1. - throbAmplitudeX * sin(time * frequency + phaseX));
  sThrobY = 1. / (1. - throbAmplitudeY * sin(sLastTime * frequency + phaseY));
}

void CCubeModel::SetRenderModelBlack(bool v) {
  sbRenderModelBlack = v;
  sbRenderModelBlackKonst = false;
}

void CCubeModel::EnableShadowMaps(const CTexture* shadowTex, const CTransform4f& textureProjXf,
                                  unsigned char chan0DisableMask,
                                  unsigned char chan1EnableLightMask) {
  if (CCubeRenderer::That()->GetMaterialMode() != 0) {
    return;
  }
  sbRenderModelShadow = true;
  spShadowTexture = const_cast< CTexture* >(shadowTex);
  sTextureProjectionTransform = textureProjXf;
  sChannel0DisableLightMask = chan0DisableMask;
  sChannel1EnableLightMask = chan1EnableLightMask;
}

void CCubeModel::DisableShadowMaps() { sbRenderModelShadow = false; }

int CCubeMaterial::GetTransparencyKColor() { return sTransparencyKColor; }

void CCubeMaterial::ResetTransparencyKColor() { sTransparencyKColor = -1; }

void CCubeMaterial::UseNormalTevs() { sTevHandler = HandleNormalTevs; }

void CCubeMaterial::UseThermalTevs() { sTevHandler = HandleThermalTevs; }
