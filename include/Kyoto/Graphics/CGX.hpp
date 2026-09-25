#ifndef _CGX
#define _CGX

#include "types.h"

#include <stddef.h>

#include <dolphin/gx.h>
#include <dolphin/mtx.h>

class CGX {
public:
  struct STevState {
    uint mColorInArgs;
    uint mAlphaInArgs;
    uint mColorOps;
    uint mAlphaOps;
    uint mIndFlags;
    uint mTevOrderFlags;
    uchar mKColorSel;
    uchar mKAlphaSel;

    STevState();
  };

  struct STexState {
    uint mCoordGen;

    STexState();
  };

  struct SFogParams {
    float mFogStartZ;
    float mFogEndZ;
    float mFogNearZ;
    float mFogFarZ;
    GXColor mFogColor;
    uchar x14_;
    uchar x15_;

    SFogParams() : mFogStartZ(0.f), mFogEndZ(1.f), mFogNearZ(0.1f), mFogFarZ(1.f) {
      mFogColor.a = 0;
      mFogColor.b = 0;
      mFogColor.g = 0;
      mFogColor.r = 0;
      x14_ = 0;
      x15_ = 0;
    }
  };

  struct SGXState {
    const void* mArrayPtrs[12];
    ushort mPrevChanCtrls[2];
    ushort mChanCtrls[2];
    GXColor mChanAmbColors[2];
    GXColor mChanMatColors[2];
    uint mDescList;
    union {
      uchar mChanFlags;
      struct {
        uchar unused : 5;
        uchar chansDirty : 2;
        uchar numDirty : 1;
      } mFlags;
    };
    uchar mPrevNumChans;
    uchar mNumChans;
    uchar mNumTexGens;
    uchar mNumTevStages;
    uchar mNumIndStages;
    uchar mZmode;
    uchar mFogType;
    ushort mLineWidthAndOffset;
    ushort mBlendMode;
    GXColor mKColors[4];
    STevState mTevStates[16];
    STexState mTexStates[8];
    uint mAlphaCompare;
    SFogParams mFogParams;

    SGXState();
  };

  enum EChannelId {
    Channel0, // GX_COLOR0
    Channel1, // GX_COLOR1
  };

  static void SetNumChans(uchar num);
  static void SetNumTexGens(uchar num);
  static void SetNumTevStages(uchar num);
  static void SetNumIndStages(uchar num);
  static void SetChanAmbColor(EChannelId channel, const GXColor& color);
  static void SetChanMatColor(EChannelId channel, const GXColor& color);
  static void SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc,
                          GXLightID lights, GXDiffuseFn diffFn, GXAttnFn attnFn);
  static void SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl);
  static void SetTevKColor(GXTevKColorID id, const GXColor& color);
  static void SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                            GXTevColorArg d);
  static void SetTevColorIn_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                            GXTevAlphaArg d);
  static void SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                            GXBool clamp, GXTevRegID outReg);
  static void SetTevColorOp_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                            GXBool clamp, GXTevRegID outReg);
  static void SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel);
  static void SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel);
  static void SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                          GXChannelID color);
  static void SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                           GXLogicOp op);
  static void SetZMode(GXBool compareEnable, GXCompare func, GXBool updateEnable);
  static void SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                              uchar ref1);
  static void SetTevIndirect(GXTevStageID stageId, GXIndTexStageID indStage, GXIndTexFormat fmt,
                             GXIndTexBiasSel biasSel, GXIndTexMtxID mtxSel, GXIndTexWrap wrapS,
                             GXIndTexWrap wrapT, GXBool addPrev, GXBool indLod,
                             GXIndTexAlphaSel alphaSel);
  static void SetTevIndWarp(GXTevStageID stageId, GXIndTexStageID indStage, uchar signedOffset,
                            uchar replaceMode, GXIndTexMtxID mtxSel) {
    const GXIndTexWrap wrap = replaceMode != 0 ? GX_ITW_0 : GX_ITW_OFF;
    SetTevIndirect(stageId, indStage, GX_ITF_8, signedOffset != 0 ? GX_ITB_STU : GX_ITB_NONE,
                   mtxSel, wrap, wrap, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
  }
  static void SetTevDirect(GXTevStageID stageId);
  static void SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src, GXTexMtx mtx,
                             GXBool normalize, GXPTTexMtx postMtx);
  static void SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags);
  static void SetArray(GXAttr attr, const void* data, uchar stride);
  static void SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                     const GXColor& color);
  static void SetLineWidth(uchar width, GXTexOffset offset);
  static void SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp);
  static void SetVtxDescv_Compressed(uint flags);
  static void SetVtxDesc(GXAttr attr, GXAttrType type); // name?
  static void ResetVtxDescv();                          // name?
  static void SetVtxDescv(const GXVtxDescList* list);
  static void SetStandardDirectTev_Compressed(GXTevStageID stageId, uint colorArgs, uint alphaArgs,
                                              uint colorOps, uint alphaOps);
  static void SetStandardTevColorAlphaOp(GXTevStageID stageId);

  static void CallDisplayList(const void* ptr, size_t size);
  static void Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx);
  static void End();
  static void ResetGXStates();
  static void ResetGXStatesFull(); // name?
  static void SetDstAlpha(bool enable, uchar alpha);

  static inline void LoadTexMtxImm(const float mtx[][4], unsigned long id, GXTexMtxType type) {
    GXLoadTexMtxImm(const_cast< MtxPtr >(mtx), id, type);
  }

  static GXColor GetChanAmbColor(EChannelId channel);
  static const GXColor& GetTevKColor(GXTevKColorID id) { return gpGXState->mKColors[id]; }
  static const STevState& GetTevState(GXTevStageID stageId) {
    return gpGXState->mTevStates[stageId];
  }
  static void GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ,
                     float* fogFarZ, GXColor* fogColor);

  static inline bool CompareGXColors(const GXColor& lhs, const GXColor& rhs) {
    return *reinterpret_cast< const uint* >(&lhs) == *reinterpret_cast< const uint* >(&rhs);
  }
  static inline void CopyGXColor(GXColor& dst, const GXColor& src) {
    *reinterpret_cast< uint* >(&dst) = *reinterpret_cast< const uint* >(&src);
  }
  static inline uint MaskAndShiftLeft(uint v, uint m, uint s) { return (v & m) << s; }
  static inline uint ShiftRightAndMask(uint v, uint m, uint s) { return (v >> s) & m; }

private:
  static void FlushChanCtrl(GXChannelID chan, ushort flags);
  static void FlushState();
  static void update_fog(uint flags);
  static void apply_fog() {
    static const GXColor black = {0, 0, 0, 0};
    GXSetFog(static_cast< GXFogType >(gpGXState->mFogType),
             gpGXState->mFogParams.mFogStartZ, gpGXState->mFogParams.mFogEndZ,
             gpGXState->mFogParams.mFogNearZ, gpGXState->mFogParams.mFogFarZ,
             (gpGXState->mBlendMode & (7 << 5)) == (GX_BL_ONE << 5)
                 ? black
                 : gpGXState->mFogParams.mFogColor);
  }

  static SGXState sGXState;
  static SGXState* gpGXState;
};

#endif // _CGX
