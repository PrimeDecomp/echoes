#include "MetroidPrime/CCameraManager.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CCameraHintManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Cameras/CFixedCamera.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CInterpolationCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"
#include "MetroidPrime/Cameras/CSpindleCamera.hpp"
#include "MetroidPrime/Cameras/CSurfaceCamera.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "rstl/algorithm.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSurfaceCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

float CCameraManager::sFirstPersonFOV = 55.f;
float CCameraManager::sThirdPersonFOV = 60.f;
float CCameraManager::sNearPlane = 0.2f;
float CCameraManager::sFarPlane = 750.f;

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
, mCameraHintManager(rs_new CCameraHintManager(playerIndex, rstl::string_l("Camera Hint Manager")))
, mCameraShakeManager(rs_new CCameraShakerManager(playerIndex))
, mFirstPersonFov(sFirstPersonFOV)
, mCameraHistory(CTransform4f::Identity())
, mScreenFlashTimer(0.f)
, mFluidFilterHandle(0)
, mInWater(false)
, xfa4_25_(false)
, mWasFogEnabled(false)
, mFogEnabled(false) {
  // mSurfaceCamera is assigned by CreateCameras, not initialized by the original constructor.
}

float CCameraManager::GetFirstPersonFOV() const { return mFirstPersonFov; }

void CCameraManager::SetFirstPersonFOV(float fov) { mFirstPersonFov = fov; }

float CCameraManager::GetDefaultThirdPersonVerticalFOV() { return sThirdPersonFOV; }

float CCameraManager::GetDefaultFirstPersonNearClipDistance() { return sNearPlane; }

float CCameraManager::GetDefaultFirstPersonFarClipDistance() { return sFarPlane; }

float CCameraManager::GetDefaultAspectRatio() { return 1.42f; }

void CCameraManager::SetAspectRatio(float aspect, CStateManager& mgr) {
  for (int i = 0; i < mCameras.size(); ++i) {
    CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]));
    camera->SetAspectRatio(aspect);
  }
}

void CCameraManager::CreateCameras(CStateManager& mgr) {
  CPlayer& player = *mgr.Player(mPlayerIndex);
  const TUniqueId playerId = player.GetUniqueId();
  CTransform4f xf = CTransform4f::Identity();
  xf.SetTranslation(player.GetEyePosition());
  mCameraHistory.mBegin = mCameraHistory.mTransforms.begin();
  mCameraHistory.mEnd = mCameraHistory.mBegin + 1;

  const TUniqueId fpId = mgr.AllocateUniqueId();
  mFpCamera = rs_new CFirstPersonCamera(
      fpId, xf, playerId, player.GetTweakPlayer()->GetOrbitCameraSpeed(), GetFirstPersonFOV(),
      GetDefaultFirstPersonNearClipDistance(), GetDefaultFirstPersonFarClipDistance(),
      GetDefaultAspectRatio(), mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mFpCamera);
  AddCamera(mFpCamera->GetUniqueId(), mgr);
  mgr.Player(mPlayerIndex)->SetCameraState(CPlayer::kCS_FirstPerson, mgr);
  SetCurrentCameraId(fpId, mgr);

  const TUniqueId surfaceId = mgr.AllocateUniqueId();
  mSurfaceCamera = rs_new CSurfaceCamera(surfaceId, xf, false, mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mSurfaceCamera);
  AddCamera(mSurfaceCamera->GetUniqueId(), mgr);

  const TUniqueId pathId = mgr.AllocateUniqueId();
  mPathCamera = rs_new CPathCamera(pathId, xf, false, mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mPathCamera);
  AddCamera(mPathCamera->GetUniqueId(), mgr);

  const TUniqueId spindleId = mgr.AllocateUniqueId();
  mSpindleCamera = rs_new CSpindleCamera(spindleId, xf, false, mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mSpindleCamera);
  AddCamera(mSpindleCamera->GetUniqueId(), mgr);

  const TUniqueId fixedId = mgr.AllocateUniqueId();
  mFixedCamera = rs_new CFixedCamera(fixedId, xf, mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mFixedCamera);
  AddCamera(mFixedCamera->GetUniqueId(), mgr);

  const TUniqueId ballId = mgr.AllocateUniqueId();
  mBallCamera = rs_new CBallCamera(ballId, playerId, xf, GetDefaultThirdPersonVerticalFOV(),
                                   GetDefaultFirstPersonNearClipDistance(),
                                   GetDefaultFirstPersonFarClipDistance(), GetDefaultAspectRatio(),
                                   mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mBallCamera);
  AddCamera(mBallCamera->GetUniqueId(), mgr);

  const TUniqueId cinematicId = mgr.AllocateUniqueId();
  mCinematicCamera = rs_new CCinematicCamera(
      cinematicId, xf, false, GetDefaultThirdPersonVerticalFOV(),
      GetDefaultFirstPersonNearClipDistance(), GetDefaultFirstPersonFarClipDistance(),
      GetDefaultAspectRatio(), mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mCinematicCamera);
  AddCamera(mCinematicCamera->GetUniqueId(), mgr);

  const TUniqueId interpId = mgr.AllocateUniqueId();
  mInterpCamera = rs_new CInterpolationCamera(interpId, xf, mPlayerIndex, mPlayerIndex);
  mgr.AddObject(mInterpCamera);
  AddCamera(mInterpCamera->GetUniqueId(), mgr);

  CSfxManager::AddListener(CSfxManager::kSC_Game, CVector3f::Zero(), CVector3f::Zero(),
                           CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 0.f, 1.f), 50.f, 50.f, 1000.f,
                           1, CAudioSys::kMaxVolume, mPlayerIndex);
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
  CTransform4f xf = mgr.GetPlayer(mPlayerIndex)->CreateTransformFromMovementDirection();
  xf.SetTranslation(mgr.GetPlayer(mPlayerIndex)->GetEyePosition());

  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]))) {
      camera->Reset(xf, mgr);
    }
  }
}

void CCameraManager::UpdateFogState(CStateManager& mgr) {
  mWasFogEnabled = mFogEnabled;
  mFogEnabled = !mFog.IsFogDisabled();
}

TUniqueId CCameraManager::GetCurrentCameraId(bool selector) const {
  if (IsInCinematicCamera()) {
    if (mCinematicCamera) {
      return mCinematicCamera->GetUniqueId();
    }
    return kInvalidUniqueId;
  }
  return mCurCameraId;
}

CGameCamera* CCameraManager::CurrentCamera(CStateManager& mgr, bool selector) {
  return static_cast< CGameCamera* >(mgr.ObjectById(GetCurrentCameraId(selector)));
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& mgr, bool selector) const {
  return static_cast< const CGameCamera* >(mgr.GetObjectById(GetCurrentCameraId(selector)));
}

void CCameraManager::SetCurrentCameraId(TUniqueId uid, CStateManager& mgr) { mCurCameraId = uid; }

void CCameraManager::UpdateAudioListener(CStateManager& mgr) {
  const CTransform4f xf = GetCurrentCameraTransform(mgr, true);
  CSfxManager::UpdateListener(xf.GetTranslation(), CVector3f::Zero(), xf.GetForward(), xf.GetUp(),
                              CAudioSys::kMaxVolume, mPlayerIndex);
}

void CCameraManager::UpdateFilters(float dt, CStateManager& mgr) {
  if (mFogDensitySpeed != 0.f) {
    mFogDensityFactor += dt * mFogDensitySpeed;
    if (mFogDensitySpeed > 0.f ? mFogDensityFactor > mFogDensityFactorTarget
                               : mFogDensityFactor < mFogDensityFactorTarget) {
      mFogDensityFactor = mFogDensityFactorTarget;
      mFogDensitySpeed = 0.f;
    }
  }

  CCameraFilterPass& pass = mgr.CameraFilterPass(mPlayerIndex, 4);
  CGameCamera& camera = *CurrentCamera(mgr, false);
  camera.RemoveInvalidFluidIds(mgr);
  const CScriptWater* const water =
      TCastToConstPtr< CScriptWater >(mgr.GetObjectById(camera.InFluidId()));
  if (camera.GetFluidCount() && water) {
    const float near = camera.GetNearClipDistance();
    float far = GetWaterFarDistance(mgr, water);
    if (water->GetFluidPlane().GetFluidType() == 2) {
      far = 225.f;
      mFluidFogTime += dt;
      if (mFluidFogTime >= 8.f) {
        mFluidFogTime -= 8.f;
      }
      far += 75.f * sinf(M_2PIF * mFluidFogTime / 8.f);
    }
    const CColor& color = water->GetUnderwaterFogColor();
    mFog.SetFogExplicit(kRFM_PerspExp, color, CVector2f(near, far));
    if (mgr.GetPlayerState(mPlayerIndex)->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
      pass.DisableFilter(0.f);
    } else {
      pass.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f, color,
                     kInvalidAssetId);
    }
    if (!mInWater && water->GetLowPassFilterEnabled()) {
      mFluidFilterHandle = CSfxManager::AddLowPassFilter(water->GetLowPassFilterId(), 0.f);
    }
    mInWater = true;
  } else if (mInWater) {
    mFog.DisableFog();
    pass.DisableFilter(0.f);
    mInWater = false;
    CSfxManager::RemoveLowPassFilter(mFluidFilterHandle);
    mFluidFilterHandle = 0;
  }
  mFog.Update(dt);

  CCameraFilterPass& flash = mgr.CameraFilterPass(mPlayerIndex, 9);
  if (mScreenFlashTimer <= 0.f) {
    flash.DisableFilter(0.f);
  } else {
    mScreenFlashTimer += dt;
    if (mScreenFlashTimer > 1.25f) {
      mScreenFlashTimer = 0.f;
      flash.DisableFilter(0.f);
    } else if (!(mScreenFlashTimer < 0.95f)) {
      const float time = mScreenFlashTimer - 0.95f;
      CColor color(static_cast< uchar >(0xff), 0xdf, 0x89, 0xff);
      if (time < 0.1f) {
        color = color.WithAlphaOf((0.3f * time) / 0.1f);
      } else if (time >= 0.15f) {
        color = color.WithAlphaOf(0.3f * (1.f - CMath::Limit((time - 0.15f) / 0.15f, 1.f)));
      } else {
        color = color.WithAlphaOf(0.3f);
      }
      flash.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_Fullscreen, 0.f, color,
                      kInvalidAssetId);
    }
  }
}

float CCameraManager::GetWaterFarDistance(CStateManager& mgr, const CScriptWater* water) {
  float density = 1.f - water->GetFluidPlane().GetAlpha();
  if (mgr.GetPlayerState(mPlayerIndex)->HasPowerUp(CPlayerState::kIT_GravityBoost)) {
    density = water->GetFogGravSuitFactor() * density + water->GetFogGravSuitDist();
  } else {
    density = water->GetFogNoGravSuitFactor() * density + water->GetFogNoGravSuitDist();
  }
  return density * mFogDensityFactor;
}

void CCameraManager::SetWaterFogScale(float target, float speed) {
  mFogDensityFactorTarget = target;
  if (mFogDensityFactorTarget < mFogDensityFactor) {
    mFogDensitySpeed = -speed;
  } else {
    mFogDensitySpeed = speed;
  }
}

void CCameraManager::TransferCameraTriggers(CGameCamera& from, CGameCamera& to,
                                            CStateManager& mgr) {
  CObjectList& list = mgr.ObjectListById(kOL_Trigger);
  for (int index = list.GetFirstObjectIndex(); index != -1;
       index = list.GetNextObjectIndex(index)) {
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(list[index])) {
      if (trigger->GetActive()) {
        trigger->ReplaceInhabitant(from.GetUniqueId(), to.GetUniqueId(), mgr);
      }
    }
  }
}

void CCameraManager::UpdateCameraTriggerOccupancy(CGameCamera& camera, CStateManager& mgr) {
  CObjectList& list = mgr.ObjectListById(kOL_Trigger);
  for (int index = list.GetFirstObjectIndex(); index != -1;
       index = list.GetNextObjectIndex(index)) {
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(list[index])) {
      if (trigger->GetActive()) {
        trigger->RemoveInhabitantIfOutside(camera.GetUniqueId(), mgr);
      }
    }
  }
}

void CCameraManager::UpdateCameraTriggers(const TUniqueId& uid, CStateManager& mgr) {
  if (!TCastToPtr< CGameCamera >(mgr.ObjectById(uid))) {
    return;
  }
  CObjectList& list = mgr.ObjectListById(kOL_Trigger);
  for (int index = list.GetFirstObjectIndex(); index != -1;
       index = list.GetNextObjectIndex(index)) {
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(list[index])) {
      if (trigger->GetActive()) {
        trigger->UpdateCameraInhabitant(uid, mgr);
      }
    }
  }
}

void CCameraManager::Update(float dt, CStateManager& mgr) {
  mCameraHintManager->Update(dt, mgr);
  UpdateCameras(dt, mgr);
  UpdateAudioListener(mgr);
  mCameraShakeManager->Update(dt, mgr);
  UpdateFilters(dt, mgr);
  UpdateCameraHistory(mgr);
}

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = static_cast< CGameCamera* >(mgr.ObjectById(mCameras[i]))) {
      if (camera->GetInputIndex() == static_cast< int >(input.ControllerNumber())) {
        camera->ProcessInput(input, mgr);
      }
    }
  }
}

void CCameraManager::SetCinematicCameraId(CStateManager& mgr, TUniqueId uid) {
  if (mCinematicCameraId != kInvalidUniqueId && mCinematicCameraId != uid) {
    if (CScriptCamera* camera =
            TCastToPtr< CScriptCamera >(mgr.ObjectById(mCinematicCameraId))) {
      camera->MarkViewed(mgr);
    }
  }
  mCinematicCameraId = uid;
}

void CCameraManager::AddCinemaCamera(TUniqueId uid, CStateManager& mgr) {
  if (CScriptCamera* camera = TCastToPtr< CScriptCamera >(mgr.ObjectById(uid))) {
    if (mCinematicCameraId == kInvalidUniqueId) {
      EnterCinematic(mgr);
    }
    SetCinematicCameraId(mgr, uid);
    mCinematicCamera->SetActive(true);
    mCinematicCamera->SetScriptCameraId(uid);
    mCinematicCamera->SetFlags(camera->GetFlags());
    CTransform4f xf = GetCurrentCameraTransform(mgr, true);
    xf.SetTranslation(camera->GetSpline().GetPositionByTime(0.f, xf, mgr));
    const CQuaternion rotation = camera->GetSpline().GetOrientationByTime(0.f, xf, mgr);
    xf = rotation.BuildTransform4f(xf.GetTranslation());
    mCinematicCamera->SetTranslation(xf.GetTranslation());
    mCinematicCamera->Reset(xf, mgr);
    if (mCinematicCamera->GetFlags() & CScriptCamera::kF_CinematicPause) {
      mgr.SetCinematicPause(true);
    }
    if (mCinematicCamera->GetFlags() & 0x2) {
      gpMain->SetThirtyFps(false);
    }
  }
}

void CCameraManager::EnterCinematic(CStateManager& mgr) {
  mgr.Player(mPlayerIndex)->BreakFrozenState(mgr, CPlayer::kBFS_BreakWithEffects, false);
  CObjectList& list = mgr.ObjectListById(kOL_All);
  for (int index = list.GetFirstObjectIndex(); index != -1;
       index = list.GetNextObjectIndex(index)) {
    if (CExplosion* explosion = TCastToPtr< CExplosion >(list[index])) {
      mgr.DeleteObjectRequest(explosion->GetUniqueId());
    } else {
      CWeapon* weapon = TCastToPtr< CWeapon >(list[index]);
      if (weapon && weapon->GetActive() &&
          (weapon->GetAttribField() & CWeapon::kPA_KeepInCinematic) != CWeapon::kPA_KeepInCinematic) {
        CPatterned* patterned = TCastToPtr< CPatterned >(mgr.ObjectById(weapon->GetOwnerId()));
        CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(weapon->GetOwnerId()));
        if (patterned || player) {
          mgr.DeleteObjectRequest(weapon->GetUniqueId());
        }
      }
    }
  }
  mCameraShakeManager->Reset();
  UpdateCameraTriggers(mCinematicCamera->GetUniqueId(), mgr);
}

void CCameraManager::StopCinematics(CStateManager& mgr) {
  if (mCinematicCamera) {
    mCinematicCamera->SetActive(false);
    SetCinematicCameraId(mgr, kInvalidUniqueId);
    mgr.Player(mPlayerIndex)->UpdateCinematicState(mgr);
    mFpCamera->SkipCinematic();
    gpMain->SetThirtyFps(gpGameState->GetGameMode().GetNumPlayers() > 2);
  }
}

void CCameraManager::SetCinematicPaused(bool paused) {
  if (mCinematicCamera) {
    mCinematicCamera->SetPaused(paused);
  }
}

CTransform4f CCameraManager::GetCurrentCameraTransform(const CStateManager& mgr,
                                                       bool selector) const {
  const CGameCamera* camera = GetCurrentCamera(mgr, selector);
  return camera->GetTransform() * CTransform4f::Translate(mCameraShakeManager->GetTranslation(mgr));
}

CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& mgr,
                                                     bool selector) const {
  const CGameCamera* camera = GetCurrentCamera(mgr, selector);
  return camera->GetTransform().Rotate(mCameraShakeManager->GetTranslation(mgr));
}

bool CCameraManager::IsInCinematicCamera() const { return mCinematicCameraId != kInvalidUniqueId; }

bool CCameraManager::IsInFullScreenCinematic() const {
  if (IsInCinematicCamera()) {
    return (mCinematicCamera->GetFlags() & 0x2) != 0;
  }
  return false;
}

bool CCameraManager::IsInBallCamera() const { return mCurCameraId == mBallCamera->GetUniqueId(); }

bool CCameraManager::IsInFPCamera() const { return mCurCameraId == mFpCamera->GetUniqueId(); }

bool CCameraManager::IsInterpolationCameraActive() const { return mInterpCamera->GetActive(); }

bool CCameraManager::ShouldBypassInterpolationCamera() const { return false; }

bool CCameraManager::IsBallCameraTransitioning(const CStateManager& mgr) const {
  if (!IsInBallCamera()) {
    return false;
  }
  const CPlayer& player = *mgr.GetPlayer(mPlayerIndex);
  if (player.GetMorphBall()->GetBallState() == CMorphBall::kBS_ScrewAttack ||
      player.GetMorphBall()->GetBallState() == CMorphBall::kBS_ScrewAttackWallJump ||
      (player.GetMorphballTransitionState() == CPlayer::kMS_Morphing &&
       player.GetSpawnedMorphballState() == CPlayer::kMS_Morphed)) {
    return true;
  }
  return false;
}

void CCameraManager::SetPlayerCamera(CStateManager& mgr, TUniqueId uid) {
  if (!mInterpCamera->GetActive()) {
    return;
  }
  const CGameCamera* camera = TCastToConstPtr< CGameCamera >(mgr.GetObjectById(uid));
  if (camera && camera->GetActive()) {
    SetCurrentCameraId(uid, mgr);
  } else {
    switch (mgr.GetPlayer(mPlayerIndex)->GetMorphballTransitionState()) {
    case CPlayer::kMS_Unmorphing:
    case CPlayer::kMS_Unmorphed:
      SetCurrentCameraId(mFpCamera->GetUniqueId(), mgr);
      break;
    default:
      SetCurrentCameraId(mBallCamera->GetUniqueId(), mgr);
      break;
    }
  }
  UpdateCameraTriggers(GetCurrentCameraId(false), mgr);
  mInterpCamera->SetActive(false);
}

void CCameraManager::SetupInterpolation(const CTransform4f& xf, TUniqueId from, TUniqueId to,
                                        bool interpolateRotation,
                                        CInterpolationCamera::EPositionMode positionMode,
                                        CInterpolationCamera::ERotationMode rotationMode,
                                        CStateManager& mgr, bool flag, float duration, float fov) {
  if (!IsInFPCamera()) {
    mInterpCamera->SetInterpolation(xf, from, to, interpolateRotation, positionMode, rotationMode,
                                    mgr, flag, duration, fov);
    SetCurrentCameraId(mInterpCamera->GetUniqueId(), mgr);
  }
}

void CCameraManager::CinematicCut(CStateManager& mgr) {
  if (IsInCinematicCamera()) {
    mBallCamera->TeleportCamera(mCinematicCamera->GetTransform(), mgr);
    const CCinematicCamera* cine = mCinematicCamera;
    mBallCamera->InterpolateFOV(cine->GetFov(), 1.f, 0.f, mBallCamera->GetUniqueId(), mgr);
    StopCinematics(mgr);
  }
}

void CCameraManager::SetPathCamera(TUniqueId uid, CStateManager& mgr) {
  if (mPathCamera && (!mPathCamera->GetActive() ||
                      (mPathCamera->GetActive() && mPathCamera->GetScriptCameraId() != uid))) {
    if (TCastToConstPtr< CScriptPathCamera >(mgr.GetObjectById(uid))) {
      mPathCamera->SetActive(true);
      mPathCamera->SetScriptCameraId(uid);
      mPathCamera->Reset(GetCurrentCameraTransform(mgr, false), mgr);
      UpdateCameraTriggers(mPathCamera->GetUniqueId(), mgr);
    }
  }
}

void CCameraManager::ClearPathCamera() {
  mPathCamera->SetActive(false);
  mPathCamera->SetScriptCameraId(kInvalidUniqueId);
}

void CCameraManager::SetSpindleCamera(TUniqueId uid, CStateManager& mgr) {
  if (!mSpindleCamera->GetActive() || (mSpindleCamera->GetActive() && mSpindleCamera->GetScriptCameraId() != uid)) {
    if (TCastToPtr< CScriptSpindleCamera >(mgr.ObjectById(uid))) {
      mSpindleCamera->SetActive(true);
      mSpindleCamera->SetScriptCameraId(uid);
      mSpindleCamera->Reset(GetCurrentCameraTransform(mgr, false), mgr);
      UpdateCameraTriggers(mSpindleCamera->GetUniqueId(), mgr);
    }
  }
}

void CCameraManager::ClearSpindleCamera() {
  mSpindleCamera->SetActive(false);
  mSpindleCamera->SetScriptCameraId(kInvalidUniqueId);
}

void CCameraManager::SetFixedCamera(TUniqueId uid, const CTransform4f& xf, CStateManager& mgr) {
  if (!mFixedCamera->GetActive() || (mFixedCamera->GetActive() && mFixedCamera->GetScriptCameraId() != uid)) {
    mFixedCamera->SetActive(true);
    mFixedCamera->SetScriptCameraId(uid);
    mFixedCamera->Reset(xf, mgr);
    UpdateCameraTriggers(mFixedCamera->GetUniqueId(), mgr);
  }
}

void CCameraManager::ClearFixedCamera() { mFixedCamera->SetActive(false); }

void CCameraManager::SetSurfaceCamera(TUniqueId uid, CStateManager& mgr) {
  if (mSurfaceCamera &&
      (!mSurfaceCamera->GetActive() ||
       (mSurfaceCamera->GetActive() && mSurfaceCamera->GetScriptCameraId() != uid))) {
    if (TCastToConstPtr< CScriptSurfaceCamera >(mgr.GetObjectById(uid))) {
      mSurfaceCamera->SetActive(true);
      mSurfaceCamera->SetScriptCameraId(uid);
      mSurfaceCamera->Reset(GetCurrentCameraTransform(mgr, false), mgr);
      UpdateCameraTriggers(mSurfaceCamera->GetUniqueId(), mgr);
    }
  }
}

void CCameraManager::ClearSurfaceCamera(CStateManager&) {
  mSurfaceCamera->SetActive(false);
  mSurfaceCamera->SetScriptCameraId(kInvalidUniqueId);
}

float CCameraManager::GetCameraBobMagnitude() const {
  const float dot = CMath::AbsF(
      CMath::Limit(CVector3f::Dot(mFpCamera->GetTransform().GetForward(), CVector3f::Up()), 1.f));
  const float pitch = CMath::Limit(dot / cosf(M_PIF / 6.f), 1.f);
  return 1.f - pitch;
}

void CCameraManager::AddCamera(const TUniqueId& uid, CStateManager& mgr) {
  if (!TCastToConstPtr< CGameCamera >(mgr.GetObjectById(uid))) {
    return;
  }
  rstl::vector< TUniqueId >::iterator it = rstl::find(mCameras.begin(), mCameras.end(), uid);
  if (it == mCameras.end()) {
    if (mCameras.size() == mCameras.capacity()) {
      mCameras.reserve(mCameras.size() + 1);
    }
    mCameras.push_back_unsafe(uid);
  }
}

void CCameraManager::SCameraHistory::Push(const CTransform4f& xf) {
  bool full = false;
  if (mBegin == mEnd) {
    full = true;
  }
  *mEnd = xf;
  ++mEnd;
  if (mEnd == mTransforms.end()) {
    mEnd = mTransforms.begin();
  }
  if (full) {
    ++mBegin;
    if (mBegin == mTransforms.end()) {
      mBegin = mTransforms.begin();
    }
  }
}

void CCameraManager::UpdateCameraHistory(CStateManager& mgr) {
  const CGameCamera* camera =
      TCastToConstPtr< CGameCamera >(mgr.GetObjectById(GetCurrentCameraId(false)));
  const CTransform4f xf = camera->GetTransform();
  if (mCameraHistory.Size() != 0) {
    const CTransform4f last = *mCameraHistory.Last();
    const CVector3f delta = xf.GetTranslation() - last.GetTranslation();
    if (delta.IsMagnitudeSafe() && delta.Magnitude() > 0.5f) {
      mCameraHistory.Push(xf);
    }
  } else {
    mCameraHistory.Push(xf);
  }
}

void CCameraManager::Reset(TUniqueId uid, CStateManager& mgr) {
  ResetCameras(mgr);
  ClearPathCamera();
  ClearSpindleCamera();
  ClearSurfaceCamera(mgr);
  ClearFixedCamera();
  mCameraHintManager->Reset(mgr);
  mCameraShakeManager->Reset();
  SetCinematicCameraId(mgr, kInvalidUniqueId);
  mFirstPersonFov = sFirstPersonFOV;
  SetAspectRatio(1.42f, mgr);
  if (TCastToConstPtr< CGameCamera >(mgr.GetObjectById(uid))) {
    SetCurrentCameraId(uid, mgr);
  } else {
    SetCurrentCameraId(mFpCamera->GetUniqueId(), mgr);
  }
  CCameraFilterPass& filter = mgr.CameraFilterPass(mPlayerIndex, 4);
  mFog.DisableFog();
  filter.DisableFilter(0.f);
  mInWater = false;
  CSfxManager::RemoveLowPassFilter(mFluidFilterHandle);
  mFluidFilterHandle = 0;
  UpdateFilters(0.f, mgr);
  mCameraHistory.mBegin = mCameraHistory.mTransforms.begin();
  mCameraHistory.mEnd = mCameraHistory.mBegin + 1;
  for (int i = 0; i < mCameras.size(); ++i) {
    if (CGameCamera* camera = TCastToPtr< CGameCamera >(mgr.ObjectById(mCameras[i]))) {
      camera->ClearFluidList(mgr);
    }
  }
  mScreenFlashTimer = 0.f;
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
  // The original returns a reference into a destroyed temporary optional.
  if (mCameraHistory.Size() != 0 && mCameraHistory.Last().valid()) {
    return mCameraHistory.Last().data();
  }
  return CTransform4f::Identity();
}

void CCameraManager::TransferCameraState(CGameCamera& from, CGameCamera& to, CStateManager& mgr) {
  to.SetTranslation(from.GetTranslation());
  to.SetFluidList(from.GetFluidList());
  TransferCameraTriggers(from, to, mgr);
  UpdateCameraTriggerOccupancy(to, mgr);
}

bool CCameraManager::CheckSplineCollision(const CMotionSpline& spline, int mode,
                                          const CMaterialFilter& filter, CStateManager& mgr,
                                          CMaterialList& hitMaterial, float step,
                                          float thickness) const {
  if (spline.GetControlPointCount() == 0 || CMath::IsEpsilon(spline.GetLength(), 0.f, 0.00001f) ||
      step <= 0.f) {
    return true;
  }

  TUniqueId hitId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  int count = int(1.f + spline.GetLength() / step);
  switch (mode) {
  case 0: {
    CVector3f previous = spline.GetPositionByLength(0.f);
    for (int i = 0; i < count; ++i) {
      const CVector3f next = spline.GetPositionByLength((i + 1) * step);
      const CVector3f delta = next - previous;
      if (delta.Magnitude() > 0.1f) {
        mgr.BuildNearList(nearList, previous, delta.AsNormalized(), delta.Magnitude(), filter,
                          nullptr);
        const CRayCastResult result = mgr.RayWorldIntersection(
            hitId, previous, delta.AsNormalized(), delta.Magnitude(), filter, nearList);
        if (result.IsValid()) {
          hitMaterial = result.GetMaterial();
          return false;
        }
      }
      previous = next;
    }
    break;
  }
  case 1: {
    CVector3f previous = spline.GetPositionByLength(0.f);
    for (int i = 0; i < count; ++i) {
      const CVector3f next = spline.GetPositionByLength((i + 1) * step);
      const CVector3f delta = next - previous;
      if (delta.Magnitude() > 0.1f && !mgr.RayCollideWorld(previous, next, filter, nullptr)) {
        return false;
      }
      previous = next;
    }
    break;
  }
  case 2: {
    rstl::vector< CRayCastResult > forward;
    rstl::vector< CRayCastResult > reverse;
    forward.reserve(count);
    reverse.reserve(count);
    CVector3f previous = spline.GetPositionByLength(0.f);
    for (int i = 0; i < count; ++i) {
      const CVector3f next = spline.GetPositionByLength((i + 1) * step);
      const CVector3f delta = next - previous;
      if (delta.Magnitude() > 0.1f) {
        mgr.BuildNearList(nearList, previous, delta.AsNormalized(), delta.Magnitude(), filter,
                          nullptr);
        forward.push_back_unsafe(mgr.RayWorldIntersection(hitId, previous, delta.AsNormalized(),
                                                          delta.Magnitude(), filter, nearList));
        reverse.push_back_unsafe(mgr.RayWorldIntersection(hitId, next, -delta.AsNormalized(),
                                                          delta.Magnitude(), filter, nearList));
      } else {
        forward.push_back_unsafe(CRayCastResult::MakeInvalid());
        reverse.push_back_unsafe(CRayCastResult::MakeInvalid());
      }
      previous = next;
    }
    for (int i = 0; i < forward.size(); ++i) {
      if (forward[i].IsValid()) {
        CVector3f span = forward[i].GetPoint() - reverse[i].GetPoint();
        if (CMath::IsEpsilon(span.Magnitude(), 0.f, 0.00001f)) {
          const CVector3f pos = spline.GetPositionByLength((i + 1) * step);
          span = pos - forward[i].GetPoint();
        }
        if (span.Magnitude() > thickness) {
          hitMaterial = forward[i].GetMaterial();
          return false;
        }
      }
    }
    break;
  }
  }
  return true;
}

CCameraManager::SCameraHistory::SCameraHistory(const CTransform4f& initial)
: mTransforms(80, initial), mBegin(mTransforms.begin()), mEnd(mBegin + 1) {}
