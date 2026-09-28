#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"

#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper.hpp"

bool CTweakAutoMapper::GetShowOneMiniMapArea() const { return mData->base.unknown_0xcbe595d8; }

bool CTweakAutoMapper::GetScaleMoveSpeedWithCameraDistance() const {
  return mData->base.scaleMoveSpeedWithCameraDistance;
}

float CTweakAutoMapper::GetCameraDistance() const {
  return mData->base.mapScreenCameraDefaultDistance;
}

float CTweakAutoMapper::GetMinCamDistance() const { return mData->base.mapScreenCameraMinDistance; }

float CTweakAutoMapper::GetMaxCamDistance() const { return mData->base.mapScreenCameraMaxDistance; }

float CTweakAutoMapper::GetMinCamRotateX() const { return mData->base.mapScreenCameraMinXAngle; }

float CTweakAutoMapper::GetMaxCamRotateX() const { return mData->base.mapScreenCameraMaxXAngle; }

float CTweakAutoMapper::GetCamAngle() const { return mData->base.mapScreenCameraFOV; }

float CTweakAutoMapper::GetMiniCamDistance() const { return mData->base.miniMapCameraDistance; }

float CTweakAutoMapper::GetMiniCamXAngle() const { return mData->base.miniMapCameraAngle; }

float CTweakAutoMapper::GetMiniCamAngle() const { return mData->base.miniMapCameraFOV; }

const CColor& CTweakAutoMapper::GetSurfaceVisitedColor() const {
  return mData->base.mapVisitedSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineVisitedColor() const {
  return mData->base.mapVisitedOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceUnvisitedColor() const {
  return mData->base.mapUnvisitedSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineUnvisitedColor() const {
  return mData->base.mapUnvisitedOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceVisitedSelectColor() const {
  return mData->base.mapVisitedFocusAreaSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineVisitedSelectColor() const {
  return mData->base.mapVisitedFocusAreaOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceDarkVisitedColor() const {
  return mData->base.darkMapVisitedSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineDarkVisitedColor() const {
  return mData->base.darkMapVisitedOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceDarkUnvisitedColor() const {
  return mData->base.darkMapUnvisitedSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineDarkUnvisitedColor() const {
  return mData->base.darkMapUnvisitedOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceDarkVisitedSelectColor() const {
  return mData->base.darkMapVisitedFocusAreaSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineDarkVisitedSelectColor() const {
  return mData->base.darkMapVisitedFocusAreaOutlineColor;
}

float CTweakAutoMapper::GetMapSurfaceNormColorLinear() const {
  return mData->base.mapLightColorIntensity;
}

float CTweakAutoMapper::GetMapSurfaceNormColorConstant() const {
  return mData->base.mapAmbientColorIntensity;
}

float CTweakAutoMapper::GetOpenMapScreenTime() const { return mData->base.miniMapToMapScreenTime; }

float CTweakAutoMapper::GetCloseMapScreenTime() const { return mData->base.mapScreenToMiniMapTime; }

float CTweakAutoMapper::GetHintPanTime() const { return mData->base.unknown_0x73de4110; }

float CTweakAutoMapper::GetZoomUnitsPerFrame() const { return mData->base.mapScreenZoomSpeed; }

float CTweakAutoMapper::GetRotateDegPerFrame() const { return mData->base.mapScreenCircleSpeed; }

float CTweakAutoMapper::GetBaseMapScreenCameraMoveSpeed() const {
  return mData->base.mapScreenMoveSpeed;
}

const CColor& CTweakAutoMapper::GetSurfaceUnvisitedSelectColor() const {
  return mData->base.mapUnvisitedFocusAreaSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineUnvisitedSelectColor() const {
  return mData->base.mapUnvisitedFocusAreaOutlineColor;
}

const CColor& CTweakAutoMapper::GetSurfaceDarkUnvisitedSelectColor() const {
  return mData->base.darkMapUnvisitedFocusAreaSurfaceColor;
}

const CColor& CTweakAutoMapper::GetOutlineDarkUnvisitedSelectColor() const {
  return mData->base.darkMapUnvisitedFocusAreaOutlineColor;
}

const CColor& CTweakAutoMapper::GetScanLinesColor() const { return mData->base.unknown_0x1a4b8068; }

const CColor& CTweakAutoMapper::GetFrameColor() const { return mData->base.frameColor; }

const CColor& CTweakAutoMapper::GetTitleColor() const { return mData->base.titleColor; }

const CColor& CTweakAutoMapper::GetBlackColor() const { return mData->base.legendBackgroundColor; }

const CColor& CTweakAutoMapper::GetGradientColor() const { return mData->base.legendGradientColor; }

float CTweakAutoMapper::GetMiniAlphaSurfaceVisited() const {
  return mData->base.mapVisitedSurfaceAlphaMiniMap;
}

float CTweakAutoMapper::GetAlphaSurfaceVisited() const {
  return mData->base.mapVisitedSurfaceAlphaMapScreen;
}

float CTweakAutoMapper::GetMiniAlphaOutlineVisited() const {
  return mData->base.mapVisitedOutlineAlphaMiniMap;
}

float CTweakAutoMapper::GetAlphaOutlineVisited() const {
  return mData->base.mapVisitedOutlineAlphaMapScreen;
}

float CTweakAutoMapper::GetMiniAlphaSurfaceUnvisited() const {
  return mData->base.mapUnvisitedSurfaceAlphaMiniMap;
}

float CTweakAutoMapper::GetAlphaSurfaceUnvisited() const {
  return mData->base.mapUnvisitedSurfaceAlphaMapScreen;
}

float CTweakAutoMapper::GetMiniAlphaOutlineUnvisited() const {
  return mData->base.mapUnvisitedOutlineAlphaMiniMap;
}

float CTweakAutoMapper::GetAlphaOutlineUnvisited() const {
  return mData->base.mapUnvisitedOutlineAlphaMapScreen;
}

float CTweakAutoMapper::GetDoorHalfHeight() const { return mData->base.mapDoorHalfHeight; }

float CTweakAutoMapper::GetDoorHalfWidth() const { return mData->base.mapDoorHalfWidth; }

float CTweakAutoMapper::GetDoorHalfDepth() const { return mData->base.mapDoorHalfDepth; }

CVector2f CTweakAutoMapper::GetMiniMapViewportSize() const {
  return CVector2f(mData->base.miniMapViewportWidth, mData->base.miniMapViewportHeight);
}

float CTweakAutoMapper::GetMiniMapCamDistScale() const { return mData->base.unknown_0x3315d22b; }

CVector2f CTweakAutoMapper::GetMapPlaneScale() const {
  return CVector2f(mData->base.miniMapWidgetHalfWidth, mData->base.miniMapWidgetHalfHeight);
}

float CTweakAutoMapper::GetUniverseCamDistance() const { return mData->base.unknown_0xbdc57ce0; }

float CTweakAutoMapper::GetMinUniverseCamDistance() const { return mData->base.unknown_0x7d59c854; }

float CTweakAutoMapper::GetMaxUniverseCamDistance() const { return mData->base.unknown_0x3c4ef7d2; }

float CTweakAutoMapper::GetSwitchToFromUniverseTime() const {
  return mData->base.mapScreenToMapUniverseTime;
}

float CTweakAutoMapper::GetCamPanUnitsPerFrame() const { return mData->base.unknown_0x706f52fe; }

float CTweakAutoMapper::GetAutoMapperScaleX() const { return mData->base.unknown_0x62f9ebf6; }

float CTweakAutoMapper::GetAutoMapperScaleZ() const { return mData->base.unknown_0xa9a53853; }

float CTweakAutoMapper::GetCamVerticalOffset() const {
  return mData->base.mapScreenMove2DHoverDepth;
}

CColor CTweakAutoMapper::GetPlayerModelColor() const { return mData->base.playerModelColor; }

CColor CTweakAutoMapper::GetAreaFlashPulseColor() const { return mData->base.playerFlashedColor; }

const CColor& CTweakAutoMapper::GetTextColor() const { return mData->base.textColor; }

const CColor& CTweakAutoMapper::GetTextOutlineColor() const { return mData->base.textOutlineColor; }

const CColor& CTweakAutoMapper::GetDarkBeamDoorOutlineColor() const {
  return mData->doorColors.darkBeamDoorOutlineColor;
}

CColor CTweakAutoMapper::GetDoorColor(CMappableObject::EMappableObjectType type) const {
  switch (type) {
  default:
  case CMappableObject::kMOT_BlueDoor:
    return mData->doorColors.blueDoorColor;
  case CMappableObject::kMOT_MissileDoor:
    return mData->doorColors.missileDoorColor;
  case CMappableObject::kMOT_DarkBeamDoor:
    return mData->doorColors.darkBeamDoorColor;
  case CMappableObject::kMOT_AnnihilatorBeamDoor:
    return mData->doorColors.annihilatorBeamDoorColor;
  case CMappableObject::kMOT_LightBeamDoor:
    return mData->doorColors.lightBeamDoorColor;
  case CMappableObject::kMOT_SuperMissileDoor:
    return mData->doorColors.superMissileDoorColor;
  case CMappableObject::kMOT_SeekerDoor:
    return mData->doorColors.seekerDoorColor;
  case CMappableObject::kMOT_PowerBombDoor:
    return mData->doorColors.powerBombDoorColor;
  }
}
