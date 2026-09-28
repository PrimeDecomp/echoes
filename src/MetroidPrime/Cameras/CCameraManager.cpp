#include "MetroidPrime/CCameraManager.hpp"

#include "Kyoto/Input/CFinalInput.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

// NonMatching scaffold: camera creation and the separate hint/shake subsystems remain TODO.
CCameraManager::CCameraManager(TUniqueId curCamera, int playerIndex)
: mPlayerIndex(playerIndex)
, mCurCameraId(curCamera)
, mCinematicCameraId(kInvalidUniqueId)
, mFpCamera(nullptr)
, mBallCamera(nullptr)
, x20_(0)
, mInterpCamera(nullptr)
, mPathCamera(nullptr)
, mSpindleCamera(nullptr)
, mCinematicCamera(nullptr)
, mFixedCamera(nullptr)
, mFogDensityFactor(1.f)
, mFogDensitySpeed(0.f)
, mFogDensityFactorTarget(1.f)
, mFluidFogTime(0.f)
, mCameraHintManager(nullptr)
, mCameraShakeManager(nullptr)
, mFirstPersonFov(55.f)
, mCameraHistory(CTransform4f::Identity())
, mScreenFlashTimer(0.f)
, mInWater(false)
, xfa4_25_(false)
, mWasFogEnabled(false)
, mFogEnabled(false) {
  // TODO: construct the owned hint and shake managers once their layouts are recovered.
  // mSurfaceCamera is assigned by CreateCameras, not initialized by the original constructor.
}

float CCameraManager::GetFirstPersonFOV() const { return mFirstPersonFov; }

void CCameraManager::SetFirstPersonFOV(float fov) { mFirstPersonFov = fov; }

float CCameraManager::GetDefaultThirdPersonVerticalFOV() { return 60.f; }

float CCameraManager::GetDefaultFirstPersonNearClipDistance() { return 0.2f; }

float CCameraManager::GetDefaultFirstPersonFarClipDistance() { return 750.f; }

float CCameraManager::GetDefaultAspectRatio() { return 1.42f; }

void CCameraManager::SetAspectRatio(float aspect, CStateManager& mgr) {
  for (int i = 0; i < mCameras.size(); ++i) {
    CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]));
    camera->SetAspectRatio(aspect);
  }
}

void CCameraManager::CreateCameras(CStateManager& mgr) {
  // TODO: create/register the eight runtime cameras and this player's audio listener.
}

void CCameraManager::UpdateCameras(float dt, CStateManager& mgr) {
  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]))) {
      camera->Think(dt, mgr);
      camera->UpdatePerspective(dt, mgr);
    }
  }
}

void CCameraManager::ResetCameras(CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  CTransform4f xf = player.CreateTransformFromMovementDirection();
  xf.SetTranslation(player.GetEyePosition());

  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]))) {
      camera->Reset(xf, mgr);
    }
  }
}

void CCameraManager::UpdateFogState() {
  mWasFogEnabled = mFogEnabled;
  mFogEnabled = !mFog.IsFogDisabled();
}

TUniqueId CCameraManager::GetCurrentCameraId(bool selector) const {
  if (IsInCinematicCamera()) {
    return mCinematicCamera ? mCinematicCamera->GetUniqueId() : kInvalidUniqueId;
  }
  return mCurCameraId;
}

CGameCamera* CCameraManager::CurrentCamera(CStateManager& mgr, bool selector) {
  return static_cast< CGameCamera* >(mgr.ObjectById(GetCurrentCameraId(selector)));
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& mgr, bool selector) const {
  return static_cast< const CGameCamera* >(mgr.GetObjectById(GetCurrentCameraId(selector)));
}

void CCameraManager::SetCurrentCameraId(TUniqueId uid) { mCurCameraId = uid; }

void CCameraManager::UpdateAudioListener(CStateManager& mgr) {
  // TODO: update the listener selected by mPlayerIndex using the shaken camera transform.
}

void CCameraManager::UpdateFilters(float dt, CStateManager& mgr) {
  // TODO: fluid fog, underwater sound transitions, and the screen-flash filter.
}

float CCameraManager::GetWaterFarDistance(CStateManager& mgr, const CScriptWater* water) {
  // TODO: combine fluid alpha with this player's Gravity Boost fog settings.
  return 0.f;
}

void CCameraManager::SetWaterFogScale(float target, float speed) {
  mFogDensityFactorTarget = target;
  mFogDensitySpeed = target < mFogDensityFactor ? -speed : speed;
}

void CCameraManager::TransferCameraTriggers(CGameCamera& from, CGameCamera& to,
                                            CStateManager& mgr) {
  // TODO: transfer camera occupancy in the active trigger list.
}

void CCameraManager::UpdateCameraTriggerOccupancy(CGameCamera& camera, CStateManager& mgr) {
  // TODO: update trigger occupancy for the supplied runtime camera.
}

void CCameraManager::UpdateCameraTriggers(TUniqueId uid, CStateManager& mgr) {
  // TODO: notify active triggers of the selected camera ID.
}

void CCameraManager::Update(float dt, CStateManager& mgr) {
  // TODO: update the separate camera-hint manager before the cameras.
  UpdateCameras(dt, mgr);
  UpdateAudioListener(mgr);
  // TODO: update the separate shake manager before applying filters/history.
  UpdateFilters(dt, mgr);
  UpdateCameraHistory(mgr);
}

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]))) {
      if (camera->GetInputIndex() == input.ControllerNumber()) {
        camera->ProcessInput(input, mgr);
      }
    }
  }
}

void CCameraManager::SetCinematicCameraId(CStateManager& mgr, TUniqueId uid) {
  if (mCinematicCameraId != kInvalidUniqueId && mCinematicCameraId != uid) {
    if (CScriptCamera* camera =
            TCastToPtr< CScriptCamera >(mgr.GetObjectByIdFromListAll(mCinematicCameraId))) {
      camera->MarkViewed(mgr);
    }
  }
  mCinematicCameraId = uid;
}

void CCameraManager::AddCinemaCamera(TUniqueId uid, CStateManager& mgr) {
  // TODO: copy script cinematic settings into the runtime camera and activate it.
}

void CCameraManager::EnterCinematic(CStateManager& mgr) {
  // TODO: unfreeze the player, remove owned projectiles/effects, and clear camera shakes.
}

void CCameraManager::StopCinematics(CStateManager& mgr) {
  // TODO: deactivate the cinematic camera and restore player/camera/pause state.
}

void CCameraManager::SetCinematicPaused(bool paused) {
  if (mCinematicCamera) {
    mCinematicCamera->SetPaused(paused);
  }
}

CTransform4f CCameraManager::GetCurrentCameraTransform(const CStateManager& mgr,
                                                       bool selector) const {
  // TODO: post-multiply by the separate shake manager's translation.
  return GetCurrentCamera(mgr, selector)->GetTransform();
}

CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& mgr,
                                                     bool selector) const {
  // TODO: rotate the separate shake manager's offset into world space.
  return CVector3f::Zero();
}

bool CCameraManager::IsInCinematicCamera() const { return mCinematicCameraId != kInvalidUniqueId; }

bool CCameraManager::fn_801ABD68() const {
  // TODO: identify the cinematic settings bit tested after IsInCinematicCamera.
  return false;
}

bool CCameraManager::IsInBallCamera() const { return mCurCameraId == mBallCamera->GetUniqueId(); }

bool CCameraManager::IsInFPCamera() const {
  return mCurCameraId == mFpCamera->GetUniqueId();
}

bool CCameraManager::IsInterpolationCameraActive() const {
  return mInterpCamera->GetActive();
}

bool CCameraManager::ShouldBypassInterpolationCamera() const { return false; }

bool CCameraManager::IsBallCameraTransitioning(const CStateManager& mgr) const {
  // TODO: combine ball-camera transition state with player morph/camera state.
  return false;
}

void CCameraManager::SetPlayerCamera(CStateManager& mgr, TUniqueId uid) {
  // TODO: select the active requested camera or the morph-state fallback, then end interpolation.
}

void CCameraManager::SetupInterpolation(const CTransform4f& xf, TUniqueId from, TUniqueId to,
                                        bool interpolateRotation,
                                        CInterpolationCamera::EPositionMode positionMode,
                                        CInterpolationCamera::ERotationMode rotationMode,
                                        CStateManager& mgr, bool flag,
                                        float duration, float fov) {
  if (!IsInFPCamera()) {
    mInterpCamera->SetInterpolation(xf, from, to, interpolateRotation, positionMode, rotationMode,
                                    mgr, flag, duration, fov);
    SetCurrentCameraId(mInterpCamera->GetUniqueId());
  }
}

void CCameraManager::CinematicCut(CStateManager& mgr) {
  // TODO: reset this player's cameras, then update the cinematic camera immediately.
}

void CCameraManager::SetPathCamera(TUniqueId uid, CStateManager& mgr) {
  // TODO: validate the path-camera script actor, activate/reset its runtime camera, and notify
  // triggers.
}

void CCameraManager::ClearPathCamera() {
  // TODO: deactivate the runtime path camera and clear its script actor ID.
}

void CCameraManager::SetSpindleCamera(TUniqueId uid, CStateManager& mgr) {
  // TODO: select/reset the runtime spindle camera from the script actor and notify triggers.
}

void CCameraManager::ClearSpindleCamera() {
  // TODO: deactivate the runtime spindle camera and clear its script actor ID.
}

void CCameraManager::SetFixedCamera(TUniqueId uid, const CTransform4f& xf, CStateManager& mgr) {
  // TODO: activate/reset the fixed camera with this target ID and transform, then notify triggers.
}

void CCameraManager::ClearFixedCamera() {
  // TODO: deactivate the runtime fixed camera.
}

void CCameraManager::SetSurfaceCamera(TUniqueId uid, CStateManager& mgr) {
  // TODO: validate the surface-camera script actor and activate/reset its runtime camera.
}

void CCameraManager::ClearSurfaceCamera() {
  // TODO: deactivate the surface camera and clear its script actor ID.
}

float CCameraManager::GetCameraBobMagnitude() const {
  // TODO: attenuate bob with the first-person camera's pitch using shared vector/math helpers.
  return 0.f;
}

void CCameraManager::AddCamera(TUniqueId uid, CStateManager& mgr) {
  if (!TCastToConstPtr< CGameCamera >(mgr.GetObjectById(uid))) {
    return;
  }
  for (int i = 0; i < mCameras.size(); ++i) {
    if (mCameras[i] == uid) {
      return;
    }
  }
  mCameras.push_back(uid);
}

void CCameraManager::SCameraHistory::Push(const CTransform4f& xf) {
  const bool full = mBegin == mEnd;
  *mEnd++ = xf;
  if (mEnd == mTransforms.end()) {
    mEnd = mTransforms.begin();
  }
  if (full && ++mBegin == mTransforms.end()) {
    mBegin = mTransforms.begin();
  }
}

void CCameraManager::UpdateCameraHistory(CStateManager& mgr) {
  const CTransform4f xf = GetCurrentCamera(mgr, false)->GetTransform();
  if (mCameraHistory.Size() == 0) {
    mCameraHistory.Push(xf);
    return;
  }

  const CTransform4f last = *mCameraHistory.Last();
  const CVector3f delta = xf.GetTranslation() - last.GetTranslation();
  if (delta.IsMagnitudeSafe() && delta.Magnitude() > 0.5f) {
    mCameraHistory.Push(xf);
  }
}

void CCameraManager::Reset(TUniqueId uid, CStateManager& mgr) {
  // TODO: reset camera selection, hints, shakes, fog, audio, and transform history together.
}

void CCameraManager::StartScreenFlash() { mScreenFlashTimer = 0.95f; }

rstl::optional_object< CTransform4f > CCameraManager::SCameraHistory::Last() const {
  if (Size() == 0) {
    return rstl::optional_object< CTransform4f >();
  }
  const CTransform4f* last = mEnd == mTransforms.begin() ? mTransforms.end() : mEnd;
  return rstl::optional_object< CTransform4f >(*--last);
}

const CTransform4f& CCameraManager::GetLastCameraTransform() const {
  // Return stable history storage; the target appears to return a destroyed optional's payload.
  if (mCameraHistory.Size() == 0) {
    return CTransform4f::Identity();
  }
  const CTransform4f* last = mCameraHistory.mEnd == mCameraHistory.mTransforms.begin()
                                 ? mCameraHistory.mTransforms.end()
                                 : mCameraHistory.mEnd;
  return *--last;
}

void CCameraManager::TransferCameraState(CGameCamera& from, CGameCamera& to, CStateManager& mgr) {
  // TODO: transfer translation, fluid membership and trigger occupancy, then notify triggers.
}

bool CCameraManager::CheckSplineCollision(const CMotionSpline& spline, int mode,
                                          const CMaterialFilter& filter, CStateManager& mgr,
                                          CMaterialList& hitMaterial, float step,
                                          float thickness) const {
  // TODO: sample the motion spline and perform the selected raycast/obstruction/thickness test.
  return false;
}
