#ifndef _CCAMERAMANAGER
#define _CCAMERAMANAGER

class CGameCamera;
class CStateManager;

class CCameraManager {
public:
  const CGameCamera* GetCurrentCamera(const CStateManager&, int) const;

  static const CGameCamera* CastGameCameratoFirstPersonCamera(const CGameCamera*);
};

#endif // _CCAMERAMANAGER
