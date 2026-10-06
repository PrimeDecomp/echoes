#ifndef _CCAMERAMANAGER
#define _CCAMERAMANAGER

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CBallCamera;
class CCameraShakerManager;
class CCinematicCamera;
class CFinalInput;
class CFirstPersonCamera;
class CFixedCamera; // Guessed name
class CGameCamera;
class CHintManager;
class CMaterialFilter;
class CMaterialList;
class CMotionSpline;
class CPathCamera;
class CScriptWater;
class CSpindleCamera;
class CStateManager;
class CSurfaceCamera; // Guessed name

class CCameraManager {
public:
  CCameraManager(TUniqueId curCamera, int playerIndex);

  CHintManager* HintManager() { return mCameraHintManager.get(); }
  const CHintManager* GetHintManager() const { return mCameraHintManager.get(); }
  CCameraShakerManager* CameraShakerManager() { return mCameraShakeManager.get(); }
  CFirstPersonCamera* FirstPersonCamera() { return mFpCamera; }
  const CFirstPersonCamera* GetFirstPersonCamera() const { return mFpCamera; }
  const CBallCamera* GetBallCamera() const { return mBallCamera; }
  const CInterpolationCamera* GetInterpolationCamera() const { return mInterpCamera; }
  const CPathCamera* GetPathCamera() const { return mPathCamera; }
  CBallCamera* BallCamera() { return mBallCamera; }
  const CCinematicCamera* GetCinematicCamera() const { return mCinematicCamera; }

  float GetFirstPersonFOV() const;
  void SetFirstPersonFOV(float fov);
  static float GetDefaultAspectRatio();
  static float GetDefaultFirstPersonFarClipDistance();
  static float GetDefaultFirstPersonNearClipDistance();
  static float GetDefaultThirdPersonVerticalFOV();

  void SetAspectRatio(float aspect, CStateManager& mgr);
  void CreateCameras(CStateManager& mgr);
  void UpdateCameras(float dt, CStateManager& mgr);
  void ResetCameras(CStateManager& mgr);
  void UpdateFogState(CStateManager& mgr); // Guessed name; target caller passes an unused manager.
  TUniqueId GetCurrentCameraId(bool selector) const;
  CGameCamera* CurrentCamera(CStateManager& mgr, bool selector);
  const CGameCamera* GetCurrentCamera(const CStateManager& mgr, bool selector) const;
  void SetCurrentCameraId(TUniqueId uid);
  void UpdateAudioListener(CStateManager& mgr);
  void UpdateFilters(float dt, CStateManager& mgr);
  float GetWaterFarDistance(CStateManager& mgr, const CScriptWater* water);
  void SetWaterFogScale(float target, float speed);
  void TransferCameraTriggers(CGameCamera& from, CGameCamera& to,
                              CStateManager& mgr);                            // Guessed name
  void UpdateCameraTriggerOccupancy(CGameCamera& camera, CStateManager& mgr); // Guessed name
  void UpdateCameraTriggers(TUniqueId uid, CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  void SetCinematicCameraId(CStateManager& mgr, TUniqueId uid); // Guessed name
  void AddCinemaCamera(TUniqueId uid, CStateManager& mgr);
  void EnterCinematic(CStateManager& mgr);
  void StopCinematics(CStateManager& mgr);
  void SetCinematicPaused(bool paused); // Guessed name
  CTransform4f GetCurrentCameraTransform(const CStateManager& mgr, bool selector) const;
  const CGameArea::CAreaFog& GetFog() const { return mFog; } // Guessed name
  CVector3f GetGlobalCameraTranslation(const CStateManager& mgr, bool selector) const;

  static const CGameCamera* CastGameCameratoFirstPersonCamera(const CGameCamera*);
  bool IsInCinematicCamera() const;
  bool IsInFullScreenCinematic() const; // Guessed name, from viewport and timer consumers.
  bool IsInBallCamera() const; // Guessed name
  bool IsInFPCamera() const;
  bool IsInterpolationCameraActive() const;
  bool ShouldBypassInterpolationCamera() const;
  bool IsBallCameraTransitioning(const CStateManager& mgr) const; // Guessed name
  void SetPlayerCamera(CStateManager& mgr, TUniqueId uid);
  // The final flag is unresolved.
  void SetupInterpolation(const CTransform4f& xf, TUniqueId from, TUniqueId to,
                          bool interpolateRotation,
                          CInterpolationCamera::EPositionMode positionMode,
                          CInterpolationCamera::ERotationMode rotationMode,
                          CStateManager& mgr, bool flag, float duration, float fov);
  void CinematicCut(CStateManager& mgr);
  void SetPathCamera(TUniqueId uid, CStateManager& mgr);
  void ClearPathCamera(); // Guessed name
  void SetSpindleCamera(TUniqueId uid, CStateManager& mgr);
  void ClearSpindleCamera();                                                      // Guessed name
  void SetFixedCamera(TUniqueId uid, const CTransform4f& xf, CStateManager& mgr); // Guessed name
  void ClearFixedCamera();                                                        // Guessed name
  void SetSurfaceCamera(TUniqueId uid, CStateManager& mgr);                       // Guessed name
  void ClearSurfaceCamera();                                                      // Guessed name
  float GetCameraBobMagnitude() const;
  void AddCamera(TUniqueId uid, CStateManager& mgr);                                // Guessed name
  void UpdateCameraHistory(CStateManager& mgr);                                     // Guessed name
  void Reset(TUniqueId uid, CStateManager& mgr);                                    // Guessed name
  void StartScreenFlash();                                                          // Guessed name
  const CTransform4f& GetLastCameraTransform() const;                               // Guessed name
  void TransferCameraState(CGameCamera& from, CGameCamera& to, CStateManager& mgr); // Guessed name
  // Guessed name. Modes select material raycast, obstruction test, or a thick sweep.
  bool CheckSplineCollision(const CMotionSpline& spline, int mode, const CMaterialFilter& filter,
                            CStateManager& mgr, CMaterialList& hitMaterial, float step,
                            float thickness) const;

private:
  // Guessed name. Fixed-capacity circular history; pointers refer into mTransforms.
  struct SCameraHistory {
    explicit SCameraHistory(const CTransform4f& initial);

    void Push(const CTransform4f& xf);
    rstl::optional_object< CTransform4f > Last() const;
    int Size() const {
      if (mBegin == mEnd) {
        return mTransforms.size();
      }
      if (mBegin < mEnd) {
        return mEnd - mBegin;
      }
      return (mTransforms.end() - mBegin) + (mEnd - mTransforms.begin());
    }

    rstl::reserved_vector< CTransform4f, 80 > mTransforms;
    CTransform4f* mBegin;
    CTransform4f* mEnd;
  };

  int mPlayerIndex;
  rstl::vector< TUniqueId > mCameras;
  TUniqueId mCurCameraId;
  TUniqueId mCinematicCameraId;
  CFirstPersonCamera* mFpCamera;
  CBallCamera* mBallCamera;
  uint x20_;
  CInterpolationCamera* mInterpCamera;
  CPathCamera* mPathCamera;
  CSpindleCamera* mSpindleCamera;
  CCinematicCamera* mCinematicCamera;
  CSurfaceCamera* mSurfaceCamera;
  CFixedCamera* mFixedCamera;
  CGameArea::CAreaFog mFog;
  float mFogDensityFactor;
  float mFogDensitySpeed;
  float mFogDensityFactorTarget;
  float mFluidFogTime;              // Guessed name
  rstl::single_ptr< CHintManager > mCameraHintManager;
  rstl::single_ptr< CCameraShakerManager > mCameraShakeManager;
  float mFirstPersonFov;
  SCameraHistory mCameraHistory;
  float mScreenFlashTimer; // Guessed name
  int mFluidFilterHandle;
  bool mInWater : 1;
  bool xfa4_25_ : 1;
  bool mWasFogEnabled : 1;
  bool mFogEnabled : 1;
};
CHECK_SIZEOF(CCameraManager, 0xfa8)

#endif // _CCAMERAMANAGER
