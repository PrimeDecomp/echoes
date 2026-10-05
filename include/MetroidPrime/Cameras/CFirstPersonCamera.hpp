#ifndef _CFIRSTPERSONCAMERA
#define _CFIRSTPERSONCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CFirstPersonCamera : public CGameCamera {
public:
  CFirstPersonCamera(const TUniqueId& uid, const CTransform4f& xf, TUniqueId watchedId,
                     float orbitCameraSpeed, float fov, float nearZ, float farZ, float aspect,
                     int index, int controllerIdx);

  // CEntity
  ~CFirstPersonCamera() override;
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
  void UnkVtable84() override;
  void UnkVtable88(TUniqueId fluidId) override;

  void UpdateElevation(CStateManager& mgr);
  void UpdateTransform(CStateManager& mgr, float dt);
  void SkipCinematic();
  void SetLockCamera(bool lock) { mLockCamera = lock; }
  const CTransform4f& GetGunFollowTransform() const;
  void UpdateFluidEffects(CStateManager& mgr); // Guessed name
  void SetScriptPitchId(TUniqueId uid);

private:
  float mOrbitCameraSpeed;
  bool mLockCamera;
  CTransform4f mGunFollowXf;
  float mPitch;
  TUniqueId mPitchId;
  float mPitchTransitionTimer; // Guessed name
  TUniqueId mPendingFluidId;   // Guessed name
  CVector3f mCloseInVec;
  float mCloseInTimer;
  float mInitialFov; // Guessed name; initialized from the FOV argument, later use unresolved.
  bool mDeferBallTransitionProcessing : 1;
  bool mFluidEffectsPending : 1;
};
CHECK_SIZEOF(CFirstPersonCamera, 0x260)

#endif // _CFIRSTPERSONCAMERA
