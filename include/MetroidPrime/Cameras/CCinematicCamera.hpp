#ifndef _CCINEMATICCAMERA
#define _CCINEMATICCAMERA

#include "MetroidPrime/Cameras/CGameCamera.hpp"

class CCinematicCamera : public CGameCamera {
public:
  CCinematicCamera(TUniqueId uid, const CTransform4f& xf, bool active, float fov, float nearZ,
                   float farZ, float aspect, int index, int controllerIdx);

  // CEntity
  ~CCinematicCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const CTransform4f& xf, CStateManager& mgr) override;

  const bool CanSkip(const CStateManager& mgr) const; // Guessed name
  float GetMoveOutofIntoAlpha() const;
  CVector3f CalculateMoveOutofIntoEyePosition(bool outOfEye, const CStateManager& mgr) const;

  TUniqueId GetScriptCameraId() const { return mScriptCameraId; }
  void SetScriptCameraId(TUniqueId id) { mScriptCameraId = id; }
  uint GetFlags() const { return mFlags; }
  void SetFlags(uint flags) { mFlags = flags; }
  void SetPaused(bool paused) { mPaused = paused; }
  float GetSlowMotionScale() const { return mSlowMotionScale; } // Guessed name

private:
  float mTime;
  CVector3f mMoveIntoEyePos;
  TUniqueId mScriptCameraId;
  uint mFlags;
  float mSlowMotionScale;
  bool mPaused : 1;
};
CHECK_SIZEOF(CCinematicCamera, 0x220)

#endif // _CCINEMATICCAMERA
