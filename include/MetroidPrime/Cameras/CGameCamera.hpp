#ifndef _CGAMECAMERA
#define _CGAMECAMERA

#include "Kyoto/Math/CMatrix4f.hpp"
#include "MetroidPrime/CActor.hpp"

class CCameraManager;
class CFinalInput;
class CPlayer;

class CGameCamera : public CActor {
public:
  CGameCamera(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, float fov, float nearZ, float farZ, float aspect,
              TUniqueId watchedId, int index, int controllerIdx);

  // CEntity
  ~CGameCamera() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(const bool active) override;

  // CActor
  void ClearFluidList(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;

  // CGameCamera
  virtual void ProcessInput(const CFinalInput& input, CStateManager& mgr) = 0;
  virtual void Reset(const CTransform4f& xf, CStateManager& mgr) = 0;
  // Empty base implementations; names and unused parameters remain unresolved.
  virtual void UnkVtable84(TUniqueId fluidId, CStateManager& mgr);
  virtual void UnkVtable88(TUniqueId fluidId, CStateManager& mgr);

  void SetAspectRatio(float aspect);
  const CMatrix4f& GetPerspectiveMatrix() const;
  CVector3f ConvertToScreenSpace(const CVector3f& position) const;
  CVector3f ConvertToWorldSpace(const CVector3f& position) const;
  CTransform4f ValidateCameraTransform(const CTransform4f& newXf, const CTransform4f& oldXf, float dt);

  CPlayer& Player(CStateManager& mgr) const;
  const CPlayer& GetPlayer(const CStateManager& mgr) const;
  CCameraManager& CameraManager(CStateManager& mgr) const;
  const CCameraManager& GetCameraManager(const CStateManager& mgr) const;
  float GetFov() const;
  void SetFov(float fov);
  float GetTargetFov() const;
  void SetTargetFov(float fov);
  void SetFovAndTarget(float fov);
  void ResetFovInterpolation(float fov);
  void InterpolateFOV(float fov, float duration, float delay);
  void InterpolateFOV(float startFov, float duration, float delay, TUniqueId cameraId,
                      CStateManager& mgr);
  void UpdatePerspective(float dt, CStateManager& mgr);

  TUniqueId GetWatchedObject() const { return mWatchedObject; }
  void SetWatchedObject(TUniqueId id) { mWatchedObject = id; }
  float GetNearClipDistance() const { return mZnear; }
  float GetFarClipDistance() const { return mZfar; }
  float GetAspectRatio() const { return mAspect; }
  int GetControllerNumber() const { return mControllerIdx; }
  int GetInputIndex() const { return mInputIndex; } // Guessed name

private:
  // Guessed name
  struct SFovInterpolation {
    // Both helpers are emitted out of line in the target.
    SFovInterpolation(float delay, float remaining, float duration, float current, float target,
                      TUniqueId cameraId);
    void Set(float delay, float remaining, float duration, float current, float target,
             TUniqueId cameraId);

    float mDelay;
    float mRemaining;
    float mDuration;
    float mCurrent;
    float mTarget;
    TUniqueId mCameraId;
  };

  TUniqueId mWatchedObject;
  mutable CMatrix4f mPerspectiveMatrix;
  CTransform4f mOrigXf;
  float mZnear;
  float mZfar;
  float mAspect;
  int mInputIndex; // Input controller index; distinct from the owning player index below.
  int mControllerIdx;
  SFovInterpolation mFovInterpolation;
  mutable bool mPerspDirty : 1;
};
CHECK_SIZEOF(CGameCamera, 0x200)

#endif // _CGAMECAMERA
