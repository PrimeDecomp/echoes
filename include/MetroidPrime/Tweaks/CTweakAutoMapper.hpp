#ifndef _CTWEAKAUTOMAPPER
#define _CTWEAKAUTOMAPPER

#include "Kyoto/Math/CVector2f.hpp"
#include "rstl/single_ptr.hpp"

struct SLdrTweakAutoMapper;

class CTweakAutoMapper {
public:
  explicit CTweakAutoMapper(const SLdrTweakAutoMapper& data) : mData(&data) {}

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
  float GetMiniMapCamDistScale() const;
  float GetUniverseCamDistance() const;
  float GetMinUniverseCamDistance() const;
  float GetMaxUniverseCamDistance() const;
  float GetSwitchToFromUniverseTime() const;
  float GetCamPanUnitsPerFrame() const;
  float GetAutoMapperScaleX() const;
  float GetAutoMapperScaleZ() const;
  float GetCamVerticalOffset() const;

private:
  const SLdrTweakAutoMapper* mData;
};
CHECK_SIZEOF(CTweakAutoMapper, 0x4)

extern rstl::single_ptr< CTweakAutoMapper > gpTweakAutoMapper;

#endif // _CTWEAKAUTOMAPPER
