#ifndef _CFIXEDCAMERA
#define _CFIXEDCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

// Guessed name; the runtime camera uses the "Fixed Camera" label.
class CFixedCamera : public CGameCamera {
public:
  CFixedCamera(const TUniqueId& uid, const CTransform4f& xf, int index, int controllerIdx);

  // CEntity
  ~CFixedCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  // Guessed helper names.
  void SetScriptCameraId(TUniqueId uid);
  TUniqueId GetScriptCameraId() const { return mScriptCameraId; }
  void UpdateTargetPosition(float dt, CStateManager& mgr);
  CVector3f ConstrainLookDirection(const CVector3f& direction, CStateManager& mgr);

private:
  CVector3f mTargetPosition;
  TUniqueId mScriptCameraId;
};
CHECK_SIZEOF(CFixedCamera, 0x210)

#endif // _CFIXEDCAMERA
