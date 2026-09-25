#ifndef _CGX_IMPL
#define _CGX_IMPL

#include "Kyoto/Graphics/CGX.hpp"

#ifndef CGX_INLINE
#define CGX_INLINE inline
#endif

CGX_INLINE void CGX::SetNumChans(uchar num) {
  gpGXState->x4e_numChans = num;
  gpGXState->x4c_flags.numDirty = num != gpGXState->x4d_prevNumChans;
}

CGX_INLINE void CGX::SetNumTexGens(uchar num) {
  if (num != gpGXState->x4f_numTexGens) {
    gpGXState->x4f_numTexGens = num;
    GXSetNumTexGens(num);
  }
}

CGX_INLINE void CGX::SetChanAmbColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->x38_chanAmbColors[channel])) {
    CopyGXColor(gpGXState->x38_chanAmbColors[channel], color);
    GXSetChanAmbColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE GXColor CGX::GetChanAmbColor(EChannelId channel) {
  return gpGXState->x38_chanAmbColors[channel];
}

CGX_INLINE void CGX::SetChanMatColor(EChannelId channel, const GXColor& color) {
  if (!CompareGXColors(color, gpGXState->x40_chanMatColors[channel])) {
    CopyGXColor(gpGXState->x40_chanMatColors[channel], color);
    GXSetChanMatColor(static_cast< GXChannelID >(GX_COLOR0A0 + channel), color);
  }
}

CGX_INLINE void CGX::SetNumTevStages(uchar num) {
  if (gpGXState->x50_numTevStages != num) {
    gpGXState->x50_numTevStages = num;
    GXSetNumTevStages(num);
  }
}

CGX_INLINE void CGX::SetTevKColor(GXTevKColorID id, const GXColor& color) {
  if (!CompareGXColors(gpGXState->x58_kColors[id], color)) {
    CopyGXColor(gpGXState->x58_kColors[id], color);
    GXSetTevKColor(id, color);
  }
}

CGX_INLINE void CGX::SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel) {
  STevState& state = gpGXState->x68_tevStates[stageId];
  if (sel != state.x18_kColorSel) {
    state.x18_kColorSel = sel;
    GXSetTevKColorSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel) {
  STevState& state = gpGXState->x68_tevStates[stageId];
  if (sel != state.x19_kAlphaSel) {
    state.x19_kAlphaSel = sel;
    GXSetTevKAlphaSel(stageId, sel);
  }
}

CGX_INLINE void CGX::SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                                 GXChannelID color) {
  STevState& state = gpGXState->x68_tevStates[stageId];
  uint flags = MaskAndShiftLeft(texCoord, 0xFF, 0) | MaskAndShiftLeft(texMap, 0xFF, 8) |
               MaskAndShiftLeft(color, 0xFF, 16);
  if (state.x14_tevOrderFlags != flags) {
    state.x14_tevOrderFlags = flags;
    GXSetTevOrder(stageId, texCoord, texMap, color);
  }
}

CGX_INLINE void CGX::SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                                  GXLogicOp op) {
  uint flags = MaskAndShiftLeft(mode, 3, 0) | MaskAndShiftLeft(srcFac, 7, 2) |
               MaskAndShiftLeft(dstFac, 7, 5) | MaskAndShiftLeft(op, 0xF, 8);
  if (flags != gpGXState->x56_blendMode) {
    update_fog(flags);
    gpGXState->x56_blendMode = flags;
    GXSetBlendMode(mode, srcFac, dstFac, op);
  }
}

CGX_INLINE void CGX::SetZMode(const GXBool compareEnable, GXCompare func,
                              const GXBool updateEnable) {
  uchar flags = MaskAndShiftLeft(compareEnable, 0xFF, 0) | MaskAndShiftLeft(updateEnable, 0xFF, 1) |
                MaskAndShiftLeft(func, 0xFF, 2);
  if (flags != gpGXState->x52_zmode) {
    gpGXState->x52_zmode = flags;
    GXSetZMode(compareEnable, func, updateEnable);
  }
}

CGX_INLINE void CGX::SetTevDirect(GXTevStageID stageId) {
  STevState& state = gpGXState->x68_tevStates[stageId];
  if (state.x10_indFlags != 0) {
    state.x10_indFlags = 0;
    GXSetTevDirect(stageId);
  }
}

CGX_INLINE void CGX::SetNumIndStages(uchar num) {
  if (gpGXState->x51_numIndStages != num) {
    gpGXState->x51_numIndStages = num;
    GXSetNumIndStages(num);
  }
}

#endif // _CGX_IMPL
