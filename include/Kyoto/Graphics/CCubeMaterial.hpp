#ifndef _CCUBEMATERIAL
#define _CCUBEMATERIAL

#include "types.h"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include <dolphin/gx.h>

enum EStateFlags {
  kStateFlag_KonstValues = (1 << 3),
  kStateFlag_DepthSorting = (1 << 4),
  kStateFlag_AlphaTest = (1 << 5),
  kStateFlag_Reflection = (1 << 6),
  kStateFlag_DepthWrite = (1 << 7),
  kStateFlag_ReflectionSurfaceEye = (1 << 8),
  kStateFlag_ShadowOccluderMesh = (1 << 9),
  kStateFlag_ReflectionIndirectTexture = (1 << 10),
  kStateFlag_Lightmap = (1 << 11),
  kStateFlag_LightmapUvArray = (1 << 13),
  kStateFlag_TextureSlotMask = static_cast< uint >(~kStateFlag_LightmapUvArray),
};

class CCubeSurface;
class CCubeModel;

class CCubeMaterial {
public:
  explicit CCubeMaterial(const void* data) : x0_data(data) {}
  static void ResetCachedMaterials();
  static void EnsureViewDepStateCached(const CCubeSurface* surface);
  static void EnsureTevsDirect();
  static void KillCachedViewDepState();

  static void UseThermalTevs();
  static void UseNormalTevs();
  static void ResetTransparencyKColor();
  static int GetTransparencyKColor();
  static void ResetExtraTexCoord();
  static uint GetExtraTexCoord();
  static uint GetExtraPostTexMtx();

  const uchar* GetData() const { return static_cast< const uchar* >(x0_data); }
  uint GetFlags() const { return CBasics::SwapBytes(*reinterpret_cast< const uint* >(GetData())); }
  bool IsFlagSet(const EStateFlags flag) const { return (GetFlags() & flag) != 0; }
  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                  const CCubeModel& model) const;
  void SetCurrentBlack() const;
  uint GetCompressedBlend() const;

  static const CVector3f& GetViewingReflection() { return sViewingFrom; }

private:
  static void SetupBlendMode(uint blendFactors, const CModelFlags& flags, bool alphaTest);
  static uint HandleReflection(GXTexMapID indTexSlot, int indMtxScaleExp, uint tevCount,
                               uint texCount, uint tcgCount, uint kColorCount);

  static const CCubeModel* sLastModelCached;
  static const CCubeModel* sRenderingModel;
  static CVector3f sViewingFrom;

  const void* x0_data;
};

#endif // _CCUBEMATERIAL
