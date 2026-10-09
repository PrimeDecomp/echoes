#ifndef _CPATHCAMERA
#define _CPATHCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CScriptPathCamera;

class CPathCamera : public CGameCamera {
public:
  // Names from Prime; the Echoes script camera stores this as an int.
  enum EInitialSplinePosition {
    kISP_BallCamBasis,
    kISP_Negative,
    kISP_Positive,
    kISP_ClampBasis,
  };

  CPathCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index, int controllerIdx);

  // CEntity
  ~CPathCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  // Guessed names for the Echoes-specific runtime/script split.
  const CScriptPathCamera* GetScriptCamera(const CStateManager& mgr) const;
  float CalculateLookAtDistance(const CStateManager& mgr) const;
  float CalculatePositionDistance(float dt, const CStateManager& mgr) const;
  CVector3f MoveAlongSpline(float dt, CStateManager& mgr);
  CTransform4f AvoidDoorCollisions(const CTransform4f& xf, CStateManager& mgr);
  void UpdateOrientation(float dt, const CTransform4f& xf, const CStateManager& mgr);
  void UpdateFov(const CStateManager& mgr);

  TUniqueId GetScriptCameraId() const { return mScriptCameraId; }
  void SetScriptCameraId(TUniqueId id) { mScriptCameraId = id; }

private:
  TUniqueId mScriptCameraId;
  float mPositionDistance;
  float mLookAtDistance;
  float mPlayerDistance;
  float mSpeed;
};
CHECK_SIZEOF(CPathCamera, 0x218)

#endif // _CPATHCAMERA
