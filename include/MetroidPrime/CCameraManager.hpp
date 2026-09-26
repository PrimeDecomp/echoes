#ifndef _CCAMERAMANAGER
#define _CCAMERAMANAGER

#include "MetroidPrime/TGameTypes.hpp"

class CGameCamera;
class CStateManager;
class CTransform4f;

class CCameraManager {
public:
  static float GetDefaultAspectRatio();
  static float GetDefaultFirstPersonFarClipDistance();
  static float GetDefaultFirstPersonNearClipDistance();
  static float GetDefaultThirdPersonVerticalFOV();

  const CGameCamera* GetCurrentCamera(const CStateManager&, int) const;
  CTransform4f GetCurrentCameraTransform(const CStateManager& mgr, int mode) const;

  static const CGameCamera* CastGameCameratoFirstPersonCamera(const CGameCamera*);
  bool IsInCinematicCamera() const;
  void SetPlayerCamera(CStateManager& mgr, TUniqueId uid);
  void UpdateCameraTriggers(TUniqueId uid, CStateManager& mgr);
};

#endif // _CCAMERAMANAGER
