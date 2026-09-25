#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#include <limits.h>

CGX::SGXState CGX::sGXState;
CGX::SGXState* CGX::gpGXState = &CGX::sGXState;

#if NONMATCHING
// Doesn't need to be so big
static GXVtxDescList sVtxDescList[12];
#else
static GXVtxDescList sVtxDescList[30];
#endif

void CGX::SetNumChans(uchar num) {
  gpGXState->mNumChans = num;
  gpGXState->mFlags.numDirty = num != gpGXState->mPrevNumChans;
}

void CGX::SetNumTexGens(uchar num) {
  if (num != gpGXState->mNumTexGens) {
    gpGXState->mNumTexGens = num;
    GXSetNumTexGens(num);
  }
}

void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->mChanAmbColors[channel])) {
    CopyGXColor(gpGXState->mChanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

GXColor CGX::GetChanAmbColor(EChannelId channel) { return gpGXState->mChanAmbColors[channel]; }

void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->mChanMatColors[channel])) {
    CopyGXColor(gpGXState->mChanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

void CGX::SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc,
                      GXLightID lights, GXDiffuseFn diffFn, GXAttnFn attnFn) {
  ushort& state = gpGXState->mChanCtrls[channel];
  ushort prevFlags = gpGXState->mPrevChanCtrls[channel];
  if (lights == GX_LIGHT_NULL) {
    enable = GX_FALSE;
  }
  uint flags = MaskAndShiftLeft(enable, 1, 0) | MaskAndShiftLeft(ambSrc, 1, 1) |
               MaskAndShiftLeft(matSrc, 1, 2) | MaskAndShiftLeft(lights, 0xFF, 3) |
               MaskAndShiftLeft(diffFn, 3, 11) | MaskAndShiftLeft(attnFn, 3, 13);
  state = flags;
  gpGXState->mChanFlags =
      ((flags != prevFlags) << (channel + 1)) | (gpGXState->mChanFlags & ~(1 << (channel + 1)));
}

void CGX::SetNumTevStages(uchar num) {
  if (gpGXState->mNumTevStages != num) {
    gpGXState->mNumTevStages = num;
    GXSetNumTevStages(num);
  }
}

void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(gpGXState->mKColors[id], color)) {
    CopyGXColor(gpGXState->mKColors[id], color);
    GXSetTevKColor(id, color);
  }
}

void CGX::SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                        GXTevColorArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mColorInArgs) {
    state.mColorInArgs = flags;
    GXSetTevColorIn(stageId, a, b, c, d);
  }
}

void CGX::SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                        GXTevAlphaArg d) {
  uint flags = MaskAndShiftLeft(a, 0x1F, 0) | MaskAndShiftLeft(b, 0x1F, 5) |
               MaskAndShiftLeft(c, 0x1F, 10) | MaskAndShiftLeft(d, 0x1F, 15);
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mAlphaInArgs) {
    state.mAlphaInArgs = flags;
    GXSetTevAlphaIn(stageId, a, b, c, d);
  }
}

void CGX::SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                        GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, op, bias, scale, clamp, outReg);
  }
}

void CGX::SetTevColorOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mColorOps) {
    state.mColorOps = flags;
    GXSetTevColorOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

void CGX::SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                        GXBool clamp, GXTevRegID outReg) {
  uint flags = MaskAndShiftLeft(op, 0xF, 0) | MaskAndShiftLeft(bias, 3, 4) |
               MaskAndShiftLeft(scale, 3, 6) | MaskAndShiftLeft(clamp, 1, 8) |
               MaskAndShiftLeft(outReg, 3, 9);
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, op, bias, scale, clamp, outReg);
  }
}

void CGX::SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (flags != state.mAlphaOps) {
    state.mAlphaOps = flags;
    GXSetTevAlphaOp(stageId, static_cast< GXTevOp >(ShiftRightAndMask(flags, 0xF, 0)),
                    static_cast< GXTevBias >(ShiftRightAndMask(flags, 3, 4)),
                    static_cast< GXTevScale >(ShiftRightAndMask(flags, 3, 6)),
                    static_cast< GXBool >(ShiftRightAndMask(flags, 1, 8)),
                    static_cast< GXTevRegID >(ShiftRightAndMask(flags, 3, 9)));
  }
}

void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (sel != state.mKColorSel) {
    state.mKColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (sel != state.mKAlphaSel) {
    state.mKAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                      GXChannelID color) {
  STevState& state = gpGXState->mTevStates[stageId];
  uint flags = MaskAndShiftLeft(texCoord, 0xFF, 0) | MaskAndShiftLeft(texMap, 0xFF, 8) |
               MaskAndShiftLeft(color, 0xFF, 16);
  if (state.mTevOrderFlags != flags) {
    state.mTevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac, GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != gpGXState->mBlendMode) {
    update_fog(flags);
    gpGXState->mBlendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

void CGX::SetZMode(const GXBool compareEnable, GXCompare func, const GXBool updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != gpGXState->mZmode) {
    gpGXState->mZmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

void CGX::SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1, uchar ref1) {
  uint flags = MaskAndShiftLeft(comp0, 7, 0) | MaskAndShiftLeft(ref0, 0xFF, 3) |
               MaskAndShiftLeft(op, 7, 11) | MaskAndShiftLeft(comp1, 7, 14) |
               MaskAndShiftLeft(ref1, 0xFF, 17);
  if (gpGXState->mAlphaCompare != flags) {
    gpGXState->mAlphaCompare = flags;
    GXSetAlphaCompare(comp0, ref0, op, comp1, ref1);
    GXSetZCompLoc(comp0 == GX_ALWAYS);
  }
}

void CGX::SetTevIndirect(GXTevStageID stageId, GXIndTexStageID indStage, GXIndTexFormat fmt,
                         GXIndTexBiasSel biasSel, GXIndTexMtxID mtxSel, GXIndTexWrap wrapS,
                         GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                         GXIndTexAlphaSel alphaSel) {
  STevState& state = gpGXState->mTevStates[stageId];
  uint flags = MaskAndShiftLeft(indStage, 3, 0) | MaskAndShiftLeft(fmt, 3, 2) |
               MaskAndShiftLeft(biasSel, 7, 4) | MaskAndShiftLeft(mtxSel, 15, 7) |
               MaskAndShiftLeft(wrapS, 7, 11) | MaskAndShiftLeft(wrapT, 7, 14) |
               MaskAndShiftLeft(addPrev, 1, 17) | MaskAndShiftLeft(indLod, 1, 18) |
               MaskAndShiftLeft(alphaSel, 3, 19);
  if (state.mIndFlags != flags) {
    state.mIndFlags = flags;
    GXSetTevIndirect(stageId, indStage, fmt, biasSel, mtxSel, wrapS, wrapT, addPrev, indLod,
                     alphaSel);
  }
}

void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

void CGX::SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src, GXTexMtx mtx,
                         GXBool normalize, GXPTTexMtx postMtx) {
  STexState& state = gpGXState->mTexStates[dstCoord];
  uint vm = (mtx - GX_TEXMTX0) / 3;
  uint vp = postMtx - GX_PTTEXMTX0;
#if NONMATCHING
  // Similarly to GXTexMtx, this should also be divided by 3
  vp /= 3;
#endif
  uint flags = MaskAndShiftLeft(fn, 0xF, 0) | MaskAndShiftLeft(src, 0x1F, 4) |
               MaskAndShiftLeft(vm, 0x1F, 9) | MaskAndShiftLeft(normalize, 1, 14) |
               MaskAndShiftLeft(vp, 0x3F, 15);
  if (state.mCoordGen != flags) {
    state.mCoordGen = flags;
    GXSetTexCoordGen2(dstCoord, fn, src, mtx, normalize, postMtx);
  }
}

void CGX::SetNumIndStages(uchar num) {
  if (gpGXState->mNumIndStages != num) {
    gpGXState->mNumIndStages = num;
    GXSetNumIndStages(num);
  }
}

void CGX::SetArray(GXAttr attr, const void* data, uchar stride) {
  if (data == nullptr) {
    return;
  }
  if (attr >= GX_VA_POS && attr < GX_POS_MTX_ARRAY) {
    uint idx = attr - GX_VA_POS;
    if (gpGXState->mArrayPtrs[idx] == data) {
      return;
    }
    gpGXState->mArrayPtrs[idx] = data;
  }
  GXSetArray(attr, data, stride);
}

void CGX::CallDisplayList(const void* ptr, size_t size) {
  if (gpGXState->mChanFlags != 0) {
    FlushState();
  }
  GXCallDisplayList(ptr, size);
}

void CGX::Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx) {
  if (gpGXState->mChanFlags != 0) {
    FlushState();
  }
  GXBegin(prim, fmt, numVtx);
}

void CGX::End() {}

void CGX::SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                 const GXColor& color) {
  gpGXState->mFogType = type;
  gpGXState->mFogParams.mFogStartZ = startZ;
  gpGXState->mFogParams.mFogEndZ = endZ;
  gpGXState->mFogParams.mFogNearZ = nearZ;
  gpGXState->mFogParams.mFogFarZ = farZ;
  CopyGXColor(gpGXState->mFogParams.mFogColor, color);
  apply_fog();
}

void CGX::SetLineWidth(uchar width, GXTexOffset offset) {
  ushort flags = width | offset << 8;
  if (flags != gpGXState->mLineWidthAndOffset) {
    gpGXState->mLineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}

CGX::STevState::STevState()
: mColorInArgs(0)
, mAlphaInArgs(0)
, mColorOps(0)
, mAlphaOps(0)
, mIndFlags(0)
, mTevOrderFlags(UINT_MAX)
, mKColorSel(UCHAR_MAX)
, mKAlphaSel(UCHAR_MAX) {}

CGX::STexState::STexState() : mCoordGen(0) {}

CGX::SGXState::SGXState()
: mDescList(0)
, mPrevNumChans(2)
, mNumChans(0)
, mNumTexGens(UCHAR_MAX)
, mNumTevStages(UCHAR_MAX)
, mNumIndStages(UCHAR_MAX)
, mZmode(UCHAR_MAX)
, mFogType(0)
, mLineWidthAndOffset(USHRT_MAX)
, mBlendMode(USHRT_MAX)
, mAlphaCompare(UINT_MAX) {
  const GXColor sGXClear = {0, 0, 0, 0};
  const GXColor sGXWhite = {255, 255, 255, 255};
  for (int i = 0; i < 2; ++i) {
    mPrevChanCtrls[i] = USHRT_MAX;
    mChanCtrls[i] = USHRT_MAX;
    mChanAmbColors[i] = sGXClear;
    mChanMatColors[i] = sGXWhite;
  }
  for (int i = 0; i < 4; ++i) {
    mKColors[i] = sGXClear;
  }
  SetChanCtrl(Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  SetChanCtrl(Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
}

void CGX::ResetGXStates() {
  gpGXState->mDescList = 0;
  GXClearVtxDesc();
  for (int i = 0; i < 12; i++) {
    gpGXState->mArrayPtrs[i] = reinterpret_cast< const void* >(1);
  }
  for (int i = 0; i < 8; i++) {
    CTexture::InvalidateTexmap(static_cast< GXTexMapID >(i));
  }
  for (int i = 0; i < 4; i++) {
    GXSetTevKColor(static_cast< GXTevKColorID >(i), gpGXState->mKColors[i]);
  }
  GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
  SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCurrentMtx(GX_PNMTX0);
  SetNumIndStages(0);
  for (int i = 0; i < 4; i++) {
    GXSetIndTexCoordScale(static_cast< GXIndTexStageID >(i), GX_ITS_1, GX_ITS_1);
  }
  for (int i = 0; i < 16; i++) {
    SetTevDirect(static_cast< GXTevStageID >(i));
  }
  for (int i = 0; i < 8; i++) {
    GXSetTexCoordScaleManually(static_cast< GXTexCoordID >(i), false, 0, 0);
  }
  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
  GXSetDstAlpha(GX_FALSE, 0);
}

void CGX::ResetGXStatesFull() {
  new (gpGXState) SGXState();
  CGX::ResetGXStates();
}

void CGX::FlushChanCtrl(GXChannelID chan, ushort flags) {
  GXBool enable = ShiftRightAndMask(flags, 1, 0);
  GXColorSrc ambSrc = static_cast< GXColorSrc >(ShiftRightAndMask(flags, 1, 1));
  GXColorSrc matSrc = static_cast< GXColorSrc >(ShiftRightAndMask(flags, 1, 2));
  uint lightMask = ShiftRightAndMask(flags, 0xFF, 3);
  GXDiffuseFn diffFn = static_cast< GXDiffuseFn >(ShiftRightAndMask(flags, 3, 11));
  GXAttnFn attnFn = static_cast< GXAttnFn >(ShiftRightAndMask(flags, 3, 13));
  GXSetChanCtrl(chan, enable, ambSrc, matSrc, lightMask, diffFn, attnFn);
}

void CGX::FlushState() {
  if (gpGXState->mChanFlags & 1) {
    GXSetNumChans(gpGXState->mNumChans);
    gpGXState->mPrevNumChans = gpGXState->mNumChans;
  }
  if (gpGXState->mChanFlags & 2) {
    FlushChanCtrl(GX_COLOR0, gpGXState->mChanCtrls[0]);
    gpGXState->mPrevChanCtrls[0] = gpGXState->mChanCtrls[0];
  }
  if (gpGXState->mChanFlags & 4) {
    FlushChanCtrl(GX_COLOR1, gpGXState->mChanCtrls[1]);
    gpGXState->mPrevChanCtrls[1] = gpGXState->mChanCtrls[1];
  }
  gpGXState->mChanFlags = 0;
}

void CGX::SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp) {
  static const float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(id, const_cast< float(*)[3] >(indMtx), scaleExp);
}

void CGX::SetVtxDescv_Compressed(uint flags) {
  if (flags == gpGXState->mDescList) {
    return;
  }
  GXVtxDescList* list = sVtxDescList;
  for (uint idx = 0; idx < 11; ++idx) {
    uint shift = idx * 2;
    if ((flags & 3 << shift) == (gpGXState->mDescList & 3 << shift)) {
      continue;
    }
    list->attr = static_cast< GXAttr >(GX_VA_POS + idx);
    list->type = static_cast< GXAttrType >(flags >> shift & 3);
    ++list;
  }
  list->attr = GX_VA_NULL;
  list->type = GX_NONE;
  GXSetVtxDescv(sVtxDescList);
  gpGXState->mDescList = flags;
}

void CGX::SetVtxDesc(GXAttr attr, GXAttrType type) {
  uint lshift = (attr - GX_VA_POS) * 2;
  uint rshift = 3 << lshift;
  uint flags = type << lshift;
  if (flags != (gpGXState->mDescList & rshift)) {
    gpGXState->mDescList = flags | (gpGXState->mDescList & ~rshift);
    GXSetVtxDesc(attr, type);
  }
}

void CGX::ResetVtxDescv() {
  static const GXVtxDescList vtxDescList[2] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  SetVtxDescv(vtxDescList);
}

void CGX::SetVtxDescv(const GXVtxDescList* list) {
  uint flags = 0;
  for (; list->attr != GX_VA_NULL; ++list) {
    flags |= (list->type & 3) << (list->attr - GX_VA_POS) * 2;
  }
  SetVtxDescv_Compressed(flags);
}

void CGX::SetStandardDirectTev_Compressed(GXTevStageID stageId, uint colorArgs, uint alphaArgs,
                                          uint colorOps, uint alphaOps) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
  if (state.mColorInArgs != colorArgs) {
    state.mColorInArgs = colorArgs;
    GXSetTevColorIn(stageId, static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 0)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 5)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 10)),
                    static_cast< GXTevColorArg >(ShiftRightAndMask(colorArgs, 31, 15)));
  }
  if (state.mAlphaInArgs != alphaArgs) {
    state.mAlphaInArgs = alphaArgs;
    GXSetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 0)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 5)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 10)),
                    static_cast< GXTevAlphaArg >(ShiftRightAndMask(alphaArgs, 31, 15)));
  }
  if (colorOps != alphaOps || (colorOps & 0x1FF) != 0x100) {
    SetTevColorOp_Compressed(stageId, colorOps);
    SetTevAlphaOp_Compressed(stageId, alphaOps);
  } else if (colorOps != state.mColorOps || colorOps != state.mAlphaOps) {
    // Fast path for GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true
    state.mAlphaOps = colorOps;
    state.mColorOps = colorOps;
    GXTevRegID outReg = static_cast< GXTevRegID >(ShiftRightAndMask(colorOps, 3, 9));
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, outReg);
  }
}

void CGX::SetStandardTevColorAlphaOp(GXTevStageID stageId) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (state.mColorOps != 0x100 || state.mAlphaOps != 0x100) {
    state.mAlphaOps = 0x100;
    state.mColorOps = 0x100;
    GXSetTevColorOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
    GXSetTevAlphaOp(stageId, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  }
}

void CGX::GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ,
                 float* fogFarZ, GXColor* fogColor) {
  if (fogType != nullptr) {
    *fogType = static_cast< GXFogType >(gpGXState->mFogType);
  }
  if (fogStartZ != nullptr) {
    *fogStartZ = gpGXState->mFogParams.mFogStartZ;
  }
  if (fogEndZ != nullptr) {
    *fogEndZ = gpGXState->mFogParams.mFogEndZ;
  }
  if (fogNearZ != nullptr) {
    *fogNearZ = gpGXState->mFogParams.mFogNearZ;
  }
  if (fogFarZ != nullptr) {
    *fogFarZ = gpGXState->mFogParams.mFogFarZ;
  }
  if (fogColor != nullptr) {
    CopyGXColor(*fogColor, gpGXState->mFogParams.mFogColor);
  }
}

void CGX::SetDstAlpha(bool enable, uchar alpha) {
  // TODO
  GXSetDstAlpha(enable, alpha);
}

#ifndef TARGET_PC
struct GXData {
  ushort cpSRreg;
  ushort cpCRreg;
};
extern GXData* __GXData;

static inline void write_bp_cmd(u32 cmd) {
  GXWGFifo.u8 = GX_LOAD_BP_REG;
  GXWGFifo.u32 = cmd;
  __GXData->cpCRreg = 0;
}
#endif

void CGX::update_fog(uint flags) {
  if (gpGXState->mFogType == 0) {
    return;
  }
  if ((gpGXState->mBlendMode & 0xE0) == (flags & 0xE0)) {
    return;
  }
  if ((flags & 0xE0) == 0x20) {
#ifdef TARGET_PC
    static const GXColor sGXClear = {0, 0, 0, 0};
    GXSetFogColor(sGXClear);
#else
    write_bp_cmd(0xf2000000);
#endif
  } else {
#ifdef TARGET_PC
    GXSetFogColor(gpGXState->mFogParams.mFogColor);
#else
    write_bp_cmd((gpGXState->mFogParams.mFogColor.b) |
                 (gpGXState->mFogParams.mFogColor.g << 8) |
                 (gpGXState->mFogParams.mFogColor.r << 16) | 0xf2000000);
#endif
  }
}
