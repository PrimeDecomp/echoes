#ifndef _CCAMERAMANAGER
#define _CCAMERAMANAGER

class CGameCamera;
class CStateManager;

class CCameraManager {
public:
  static float GetDefaultAspectRatio();
  static float GetDefaultFirstPersonFarClipDistance();
  static float GetDefaultFirstPersonNearClipDistance();
  static float GetDefaultThirdPersonVerticalFOV();

  const CGameCamera* GetCurrentCamera(const CStateManager&, int) const;

  static const CGameCamera* CastGameCameratoFirstPersonCamera(const CGameCamera*);
  bool IsInCinematicCamera() const;
};

#endif // _CCAMERAMANAGER
