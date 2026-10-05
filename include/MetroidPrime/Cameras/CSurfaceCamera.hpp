#ifndef _CSURFACECAMERA
#define _CSURFACECAMERA

#include "MetroidPrime/Cameras/CCameraSpring.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

// Guessed name; the runtime camera uses the "Surface Camera" label.
class CSurfaceCamera : public CGameCamera {
public:
  CSurfaceCamera(TUniqueId uid, const CTransform4f& xf, bool active, int index, int controllerIdx);

  // CEntity
  ~CSurfaceCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  // Guessed name; selects the separate script provider.
  void SetScriptCameraId(TUniqueId uid);
  TUniqueId GetScriptCameraId() const { return mScriptCameraId; }

private:
  TUniqueId mScriptCameraId;
  CVector3f mTrackedPlayerPosition;
  CCameraSpring mPlayerPositionSpring;
  float mTargetSplineDistance;
  float mPlayerSplineDistance;
};
CHECK_SIZEOF(CSurfaceCamera, 0x230)

#endif // _CSURFACECAMERA
