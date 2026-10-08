#include "MetroidPrime/CMappableObject.hpp"

#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "dolphin/gx/GXEnum.h"

#include "rstl/math.hpp"

struct SDrawData {
  float mX;
  float mY;
  float mZ;
  uchar mIdxA;
  uchar mIdxB;
  uchar mIdxC;
  uchar mIdxD;
};

static const SDrawData skDoorSurfaceInfos[6] = {
    // clang-format off
    { 0.f,  0.f, -1.f, 6, 4, 2, 0},
    { 0.f,  0.f,  1.f, 3, 1, 7, 5},
    { 0.f, -1.f,  1.f, 1, 0, 5, 4},
    { 0.f,  1.f,  1.f, 7, 6, 3, 2},
    {-1.f,  0.f,  0.f, 3, 2, 1, 0},
    { 1.f,  0.f,  0.f, 5, 4, 7, 6},
    // clang-format on
};

// Guessed names. Pre-built display lists for all six door surfaces and their outline.
static const uchar skDoorSurfacesDisplayList[0x40] ATTRIBUTE_ALIGN(32) = {
    // clang-format off
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 6, 4, 2, 0,
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 3, 1, 7, 5,
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 1, 0, 5, 4,
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 7, 6, 3, 2,
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 3, 2, 1, 0,
    GX_NOP, GX_TRIANGLESTRIP, 0, 4, 5, 4, 7, 6,
    // clang-format on
};

static const uchar skDoorOutlineDisplayList[0x40] ATTRIBUTE_ALIGN(32) = {
    // clang-format off
    GX_NOP, GX_LINESTRIP, 0, 10, 0, 1, 3, 2, 0, 4, 5, 7, 6, 4, GX_NOP, GX_NOP,
    GX_NOP, GX_LINES, 0, 2, 1, 5, GX_NOP, GX_NOP,
    GX_NOP, GX_LINES, 0, 2, 3, 7, GX_NOP, GX_NOP,
    GX_NOP, GX_LINES, 0, 2, 2, 6, GX_NOP, GX_NOP,
    // clang-format on
};

CVector3f CMappableObject::skDoorVerts[8] = {
    CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(),
    CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(),
};

void CMappableObject::ReadAutomapperTweaks() {
  const float x = gpTweakAutoMapper->GetDoorHalfDepth();
  const float y = gpTweakAutoMapper->GetDoorHalfWidth();
  const float z = gpTweakAutoMapper->GetDoorHalfHeight();
  skDoorVerts[0] = CVector3f(-x, -y, 0.f);
  skDoorVerts[1] = CVector3f(-x, -y, z * 2.f);
  skDoorVerts[2] = CVector3f(-x, y, 0.f);
  skDoorVerts[3] = CVector3f(-x, y, z * 2.f);
  skDoorVerts[4] = CVector3f(-x * .2f, -y, 0.f);
  skDoorVerts[5] = CVector3f(-x * .2f, -y, z * 2.f);
  skDoorVerts[6] = CVector3f(-x * .2f, y, 0.f);
  skDoorVerts[7] = CVector3f(-x * .2f, y, z * 2.f);
}

void CMappableObject::DrawDoorSurface(const CColor& surfaceColor, const CColor& outlineColor,
                                      int surfaceIdx, bool needsVtxLoad) const {
  const SDrawData& drawData = skDoorSurfaceInfos[surfaceIdx];
  if (needsVtxLoad) {
    CGX::SetArray(GX_VA_POS, skDoorVerts, sizeof(skDoorVerts[0]));
  }

  CGX::SetTevKColor(GX_KCOLOR0, surfaceColor.GetGXColor());
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition1x8(drawData.mIdxA);
  GXPosition1x8(drawData.mIdxB);
  GXPosition1x8(drawData.mIdxC);
  GXPosition1x8(drawData.mIdxD);
  CGX::End();

  CGX::SetTevKColor(GX_KCOLOR0, outlineColor.GetGXColor());
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 5);
  GXPosition1x8(drawData.mIdxA);
  GXPosition1x8(drawData.mIdxB);
  GXPosition1x8(drawData.mIdxD);
  GXPosition1x8(drawData.mIdxC);
  GXPosition1x8(drawData.mIdxA);
  CGX::End();
}

rstl::pair< CColor, CColor >
CMappableObject::GetDoorColors(int curAreaId, const CMapWorldInfo& mwInfo, float alpha) const {
  CColor firstColor;
  bool doorVisited = mwInfo.IsDoorVisited(mObjId);

  if (mwInfo.IsAreaVisited(mObjId.AreaNum())) {
    if (doorVisited && mType != kMOT_DarkBeamDoor && mType != kMOT_AnnihilatorBeamDoor &&
        mType != kMOT_LightBeamDoor) {
      firstColor = gpTweakAutoMapper->GetDoorColor(kMOT_BlueDoor);
    } else {
      firstColor = gpTweakAutoMapper->GetDoorColor(mType);
    }
  } else {
    firstColor = CColor(0);
  }

  firstColor = firstColor.WithAlphaModulatedBy(alpha);
  if (mType == kMOT_DarkBeamDoor) {
    return rstl::pair< CColor, CColor >(
        firstColor, gpTweakAutoMapper->GetDarkBeamDoorOutlineColor().WithAlphaModulatedBy(alpha));
  }

  const CColor secondColor(rstl::min_val(1.0f, firstColor.GetRed() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetGreen() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetBlue() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetAlpha() * 1.4f));
  return rstl::pair< CColor, CColor >(firstColor, secondColor);
}

void CMappableObject::PostConstruct(const void*) {
  for (int i = 0; i < offsetof(CMappableObject, x40_) / sizeof(int); ++i) {
    reinterpret_cast< uint* >(this)[i] = CBasics::SwapBytes(reinterpret_cast< uint* >(this)[i]);
  }
  mTransform = AdjustTransformForType();
}

static inline void draw_door(const CColor& surfaceColor, const CColor& outlineColor) {
  CGX::SetTevKColor(GX_KCOLOR0, surfaceColor.GetGXColor());
  CGX::CallDisplayList(skDoorSurfacesDisplayList, sizeof(skDoorSurfacesDisplayList));
  CGX::SetTevKColor(GX_KCOLOR0, outlineColor.GetGXColor());
  CGX::CallDisplayList(skDoorOutlineDisplayList, sizeof(skDoorOutlineDisplayList));
}

void CMappableObject::Draw(int curArea, const CMapWorldInfo& mwInfo, float alpha,
                           bool needsVtxLoad) const {
  if (IsDoorType(mType)) {
    rstl::pair< CColor, CColor > colors = GetDoorColors(curArea, mwInfo, alpha);
    if (needsVtxLoad) {
      CGX::SetArray(GX_VA_POS, skDoorVerts, sizeof(skDoorVerts[0]));
    }
    draw_door(colors.first, colors.second);
    return;
  }

  bool visible = true;
  CAssetId iconRes = kInvalidAssetId;
  CColor iconColor = CColor(0xffffffff);
  switch (mType) {
  case kMOT_DownArrow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesFirstBreakTopIcon();
    break;
  case kMOT_UpArrow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesFirstBreakBottomIcon();
    break;
  case kMOT_SaveStation:
    iconRes = gpTweakPlayerRes->GetSaveStationIcon();
    break;
  case kMOT_MissileStation:
    iconRes = gpTweakPlayerRes->GetMissileStationIcon();
    break;
  case kMOT_Portal:
    iconRes = gpTweakPlayerRes->GetPortalIcon();
    break;
  case kMOT_Elevator:
    iconRes = gpTweakPlayerRes->GetElevatorIcon();
    break;
  case kMOT_TranslatorDoor:
    iconRes = gpTweakPlayerRes->GetTranslatorDoorIcon();
    visible = !mwInfo.IsObjectUnmapped(mObjId);
    break;
  default:
    break;
  }

  if (visible && iconRes != kInvalidAssetId) {
    TLockedToken< CTexture > tex = gpSimplePool->GetObj(SObjectTag('TXTR', iconRes));
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::StreamBegin(kP_TriangleStrip);
    CGraphics::StreamColor(iconColor.WithAlphaOf(alpha));
    CGraphics::StreamTexcoord(0.0f, 1.0f);
    CGraphics::StreamVertex(-2.6f, 0.0f, 2.6f);
    CGraphics::StreamTexcoord(0.0f, 0.0f);
    CGraphics::StreamVertex(-2.6f, 0.0f, -2.6f);
    CGraphics::StreamTexcoord(1.0f, 1.0f);
    CGraphics::StreamVertex(2.6f, 0.0f, 2.6f);
    CGraphics::StreamTexcoord(1.0f, 0.0f);
    CGraphics::StreamVertex(2.6f, 0.0f, -2.6f);
    CGraphics::StreamEnd();
  }
}

void CMappableObject::DrawDoor(int curAreaId, const CMapWorldInfo& mwInfo, float alpha) const {
  rstl::pair< CColor, CColor > colors = GetDoorColors(curAreaId, mwInfo, alpha);
  draw_door(colors.first, colors.second);
}

void CMappableObject::DrawDoorSurface(int curAreaId, const CMapWorldInfo& mwInfo, float alpha,
                                      int surfaceIdx, bool needsVtxLoad) const {
  rstl::pair< CColor, CColor > colors = GetDoorColors(curAreaId, mwInfo, alpha);
  DrawDoorSurface(colors.first, colors.second, surfaceIdx, needsVtxLoad);
}

CVector3f CMappableObject::BuildSurfaceCenterPoint(int surfaceIdx) const {
  const float x = gpTweakAutoMapper->GetDoorHalfDepth();
  const float y = gpTweakAutoMapper->GetDoorHalfWidth();
  const float z = gpTweakAutoMapper->GetDoorHalfHeight();
  switch (surfaceIdx) {
  case 0:
    return mTransform * CVector3f::Zero();
  case 1:
    return mTransform * CVector3f(0.f, 0.f, 2.f * z);
  case 2:
    return mTransform * CVector3f(0.f, -y, 0.f);
  case 3:
    return mTransform * CVector3f(0.f, y, 0.f);
  case 4:
    return mTransform * CVector3f(-x, 0.f, 0.f);
  case 5:
    return mTransform * CVector3f(x, 0.f, 0.f);
  default:
    return CVector3f::Zero();
  }
}

bool CMappableObject::GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const {
  bool areaVis = mwInfo.IsAreaVisible(mObjId.AreaNum());
  bool areaVisited = mwInfo.IsAreaVisited(mObjId.AreaNum());
  switch (mVisibilityMode) {
  case kVM_Always:
    return true;
  case kVM_Visit:
    return areaVisited;
  case kVM_MapStationOrVisit:
    return worldVis || areaVis;
  case kVM_DoorVisit:
    if (IsDoorType(mType)) {
      return mwInfo.IsDoorVisited(mObjId);
    }
    return areaVis;
  case kVM_Never:
    return false;
  default:
    return true;
  }
}

CTransform4f CMappableObject::AdjustTransformForType() const {
  if (IsDoorType(mType)) {
    return GetTransform();
  }
  return CTransform4f::Translate(GetTransform().GetTranslation());
}
