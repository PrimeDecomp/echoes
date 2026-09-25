#ifndef _CGX_IMPL
#define _CGX_IMPL

#include "Kyoto/Graphics/CGX.hpp"

#ifndef CGX_INLINE
#define CGX_INLINE inline
#endif

CGX_INLINE void CGX::SetNumChans(uchar num) {
  gpGXState->mNumChans = num;
  gpGXState->mFlags.numDirty = num != gpGXState->mPrevNumChans;
}

CGX_INLINE void CGX::SetNumTexGens(uchar num) {
  if (num != gpGXState->mNumTexGens) {
    gpGXState->mNumTexGens = num;
    GXSetNumTexGens(num);
  }
}

CGX_INLINE void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->mChanAmbColors[channel])) {
    CopyGXColor(gpGXState->mChanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE GXColor CGX::GetChanAmbColor(EChannelId channel) {
  return gpGXState->mChanAmbColors[channel];
}

CGX_INLINE void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->mChanMatColors[channel])) {
    CopyGXColor(gpGXState->mChanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE void CGX::SetNumTevStages(uchar num) {
  if (gpGXState->mNumTevStages != num) {
    gpGXState->mNumTevStages = num;
    GXSetNumTevStages(num);
  }
}

CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(gpGXState->mKColors[id], color)) {
    CopyGXColor(gpGXState->mKColors[id], color);
    GXSetTevKColor(id, color);
  }
}

CGX_INLINE void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (sel != state.mKColorSel) {
    state.mKColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (sel != state.mKAlphaSel) {
    state.mKAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                                 GXChannelID color) {
  STevState& state = gpGXState->mTevStates[stageId];
  uint flags = MaskAndShiftLeft(texCoord, 0xFF, 0) | MaskAndShiftLeft(texMap, 0xFF, 8) |
               MaskAndShiftLeft(color, 0xFF, 16);
  if (state.mTevOrderFlags != flags) {
    state.mTevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

CGX_INLINE void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                                  GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != gpGXState->mBlendMode) {
    update_fog(flags);
    gpGXState->mBlendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

CGX_INLINE void CGX::SetZMode(const GXBool compareEnable, GXCompare func,
                              const GXBool updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != gpGXState->mZmode) {
    gpGXState->mZmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = gpGXState->mTevStates[stageId];
  if (state.mIndFlags != 0) {
    state.mIndFlags = 0;
    GXSetTevDirect(stageId);
  }
}

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (gpGXState->mNumIndStages != num) {
    gpGXState->mNumIndStages = num;
    GXSetNumIndStages(num);
  }
}

#endif // _CGX_IMPL
