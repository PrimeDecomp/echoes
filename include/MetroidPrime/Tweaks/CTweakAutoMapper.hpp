#ifndef _CTWEAKAUTOMAPPER
#define _CTWEAKAUTOMAPPER

#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CMappableObject.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakAutoMapper;
class CColor;

class CTweakAutoMapper {
public:
  // Guessed accessor spellings remain reconstructed; renamed SLdr field roles
  // are verified unless their property comment explicitly says non-matching.
  explicit CTweakAutoMapper(const SLdrTweakAutoMapper& data) : mData(&data) {}

  // Guessed names, recovered from the runtime accessors.
  CColor GetDoorColor(CMappableObject::EMappableObjectType type) const;
  const CColor& GetDarkBeamDoorOutlineColor() const;
  float GetDoorHalfHeight() const;
  float GetDoorHalfWidth() const;
  float GetDoorHalfDepth() const;

  bool GetShowOneMiniMapArea() const;
  bool GetScaleMoveSpeedWithCameraDistance() const;
  float GetCameraDistance() const;
  float GetMinCamDistance() const;
  float GetMaxCamDistance() const;
  float GetMinCamRotateX() const;
  float GetMaxCamRotateX() const;
  float GetCamAngle() const;
  float GetMiniCamDistance() const;
  float GetMiniCamXAngle() const;
  float GetMiniCamAngle() const;
  float GetOpenMapScreenTime() const;
  float GetCloseMapScreenTime() const;
  float GetHintPanTime() const;
  float GetZoomUnitsPerFrame() const;
  float GetRotateDegPerFrame() const;
  float GetBaseMapScreenCameraMoveSpeed() const;
  float GetMiniAlphaSurfaceVisited() const;
  float GetAlphaSurfaceVisited() const;
  float GetMiniAlphaOutlineVisited() const;
  float GetAlphaOutlineVisited() const;
  float GetMiniAlphaSurfaceUnvisited() const;
  float GetAlphaSurfaceUnvisited() const;
  float GetMiniAlphaOutlineUnvisited() const;
  float GetAlphaOutlineUnvisited() const;
  CVector2f GetMiniMapViewportSize() const; // Guessed name
  float GetMiniMapDynamicCameraDistanceScalar() const;
  float GetMapScreenMapUniverseDefaultCameraDistance() const;
  float GetMapScreenMapUniverseMinCameraDistance() const;
  float GetMapScreenMapUniverseMaxCameraDistance() const;
  float GetSwitchToFromUniverseTime() const;
  float GetCamPanUnitsPerFrame() const;
  float GetMapScreenClipWindowScaleX() const;
  float GetMapScreenClipWindowScaleY() const;
  float GetCamVerticalOffset() const;
  CVector2f GetMapPlaneScale() const; // Guessed name
  CColor GetPlayerModelColor() const; // Guessed name
  const CColor& GetSurfaceVisitedSelectColor() const;
  const CColor& GetOutlineVisitedSelectColor() const;
  const CColor& GetSurfaceUnvisitedSelectColor() const;
  const CColor& GetOutlineUnvisitedSelectColor() const;
  const CColor& GetSurfaceVisitedColor() const;
  const CColor& GetSurfaceUnvisitedColor() const;
  const CColor& GetOutlineVisitedColor() const;
  const CColor& GetOutlineUnvisitedColor() const;
  const CColor& GetSurfaceDarkVisitedSelectColor() const;
  const CColor& GetOutlineDarkVisitedSelectColor() const;
  const CColor& GetSurfaceDarkUnvisitedSelectColor() const;
  const CColor& GetOutlineDarkUnvisitedSelectColor() const;
  const CColor& GetSurfaceDarkVisitedColor() const;
  const CColor& GetSurfaceDarkUnvisitedColor() const;
  const CColor& GetOutlineDarkVisitedColor() const;
  const CColor& GetOutlineDarkUnvisitedColor() const;
  CColor GetAreaFlashPulseColor() const;
  float GetMapSurfaceNormColorLinear() const;
  float GetMapSurfaceNormColorConstant() const;

  // Guessed names
  const CColor& GetTextColor() const;
  const CColor& GetTextOutlineColor() const;
  const CColor& GetTitleColor() const;
  const CColor& GetScanlineColor() const;
  const CColor& GetFrameColor() const;
  const CColor& GetGradientColor() const;
  const CColor& GetBlackColor() const;

private:
  // Borrowed settings record.
  const SLdrTweakAutoMapper* mData;
};
CHECK_SIZEOF(CTweakAutoMapper, 0x4)

extern rstl::single_ptr< CTweakAutoMapper > gpTweakAutoMapper;

#endif // _CTWEAKAUTOMAPPER
