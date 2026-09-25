#ifndef _CMODELFLAGS
#define _CMODELFLAGS

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"

class CModelFlags {
public:
  enum ETrans {
    kT_Opaque = 0,
    kT_One = 1, // ?
    kT_Two = 2, // ?
    kT_Blend = 5,
    kT_Additive = 7,
  };
  enum EFlags {
    kF_DepthCompare = 0x1,
    kF_DepthUpdate = 0x2,
    kF_NoTextureLock = 0x4,
    kF_DepthGreater = 0x8,
    kF_DepthNonInclusive = 0x10,
    kF_DrawNormal = 0x20,
    kF_ThermalUnsortedOnly = 0x40,
    kF_Unknown80 = 0x80,
    kF_Unknown200 = 0x200,
    kF_Unknown400 = 0x400,
  };

  CModelFlags(ETrans trans, float rgba)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(kF_DepthCompare | kF_DepthUpdate)
  , mColor(1.f, 1.f, 1.f, rgba) {}
  CModelFlags(ETrans trans, CColor color)
  : mBlendMode(trans)
  , mMatSetIdx(0)
  , mFlags(kF_DepthCompare | kF_DepthUpdate)
  , mColor(color) {}

  CModelFlags(ETrans blendMode, uchar shadIdx, EFlags flags, const CColor& col)
  : mBlendMode(blendMode), mMatSetIdx(shadIdx), mFlags(flags), mColor(col) {}

  CModelFlags(const CModelFlags& flags, uint otherFlags)
  : x0_(flags.x0_)
  , mBlendMode(flags.mBlendMode)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(otherFlags)
  , mColor(flags.mColor) {}
  CModelFlags(const CModelFlags& flags, bool b /* TODO what's this? */, int shaderSet)
  : x0_(flags.x0_)
  , mBlendMode(flags.mBlendMode)
  , mMatSetIdx(shaderSet)
  , mFlags(flags.mFlags)
  , mColor(flags.mColor) {}

  // ?
  CModelFlags(const CModelFlags& flags, ETrans trans, CColor color)
  : x0_(flags.x0_)
  , mBlendMode(trans)
  , mMatSetIdx(flags.mMatSetIdx)
  , mFlags(flags.mFlags)
  , mColor(color) {}

  // CModelFlags(const CModelFlags& other)
  // : x4_blendMode(other.x4_blendMode)
  // , x5_matSetIdx(other.x5_matSetIdx)
  // , x6_flags(other.x6_flags)
  // , x8_color(other.x8_color) {}
  CModelFlags& operator=(const CModelFlags& other) {
    x0_ = other.x0_;
    mBlendMode = other.mBlendMode;
    mMatSetIdx = other.mMatSetIdx;
    mFlags = other.mFlags;
    mColor = other.mColor;
    return *this;
  }

  CModelFlags UseShaderSet(int matSet) const { return CModelFlags(*this, false, matSet); }
  CModelFlags DontLoadTextures() const {
    return CModelFlags(*this, GetOtherFlags() | kF_NoTextureLock);
  }
  CModelFlags DepthCompareUpdate(bool compare, bool update) const {
    uint newFlags = 0;
    if (compare) {
      newFlags |= kF_DepthCompare;
    }
    if (update) {
      newFlags |= kF_DepthUpdate;
    }
    return CModelFlags(*this, (mFlags & ~(kF_DepthCompare | kF_DepthUpdate)) | newFlags);
  }
  CModelFlags DepthBackwards() const {
    return CModelFlags(*this, GetOtherFlags() | kF_DepthGreater);
  }

  ETrans GetTrans() const { return static_cast< ETrans >(mBlendMode); }
  int GetShaderSet() const { return mMatSetIdx; }
  uint GetOtherFlags() const { return mFlags; }
  CColor GetColor() const { return mColor; }
  const CColor& GetColorRef() const { return mColor; }

  bool operator==(const CModelFlags& other) const {
    // TODO: cast to char for extsb; see CScriptActor::PreRender
    return static_cast< char >(mBlendMode) == static_cast< char >(other.mBlendMode) &&
           static_cast< char >(mMatSetIdx) == static_cast< char >(other.mMatSetIdx) &&
           mFlags == other.mFlags && mColor == other.mColor;
  }

  static CModelFlags Normal() { return CModelFlags(kT_Opaque, 1.f); }
  static CModelFlags AlphaBlended(float alpha) { return CModelFlags(kT_Blend, alpha); }
  static CModelFlags AlphaBlended(const CColor& color) { return CModelFlags(kT_Blend, color); }
  static CModelFlags Additive(float f) { return CModelFlags(CModelFlags::kT_Additive, f); }
  static CModelFlags Additive(const CColor& color) { return CModelFlags(CModelFlags::kT_Additive, color); }
  static CModelFlags AdditiveRGB(const CColor& color);
  static CModelFlags ColorModulate(const CColor& color);

private:
  uint x0_;
  uchar mBlendMode;
  uchar mMatSetIdx;
  ushort mFlags;
  CColor mColor;
};
CHECK_SIZEOF(CModelFlags, 0xc)

#endif // _CMODELFLAGS
