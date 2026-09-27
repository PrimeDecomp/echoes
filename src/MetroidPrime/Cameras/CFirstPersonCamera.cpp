#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"

#include "MetroidPrime/Player/CPlayer.hpp"

CFirstPersonCamera::CFirstPersonCamera(const TUniqueId& uid, const CTransform4f& xf,
                                       TUniqueId watchedId, float orbitCameraSpeed, float fov,
                                       float nearZ, float farZ, float aspect, int index,
                                       int controllerIdx)
: CGameCamera(uid, rstl::string("First Person Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, true), xf, fov, nearZ, farZ, aspect,
              watchedId, index, controllerIdx)
, mOrbitCameraSpeed(orbitCameraSpeed)
, mLockCamera(false)
, mGunFollowXf(xf)
, mPitch(0.f)
, mPitchId(kInvalidUniqueId)
, mPitchTransitionTimer(0.f)
, mPendingFluidId(kInvalidUniqueId)
, mCloseInVec(CVector3f::Zero())
, mCloseInTimer(0.f)
, mInitialFov(fov)
, mDeferBallTransitionProcessing(false)
, mFluidEffectsPending(false) {}

CFirstPersonCamera::~CFirstPersonCamera() {}

void CFirstPersonCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

void CFirstPersonCamera::UpdateElevation(CStateManager& mgr) {
  mPitch = 0.f;
  // TODO: outside cinematics, evaluate the selected pitch volume for the watched player.
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr, float dt) {
  // TODO: recover Echoes's free-look/orbit/grapple interpolation and camera-bob composition.
  // Use shared vector/quaternion helpers; Prime's free-look damping differs here.
}

void CFirstPersonCamera::PreThink(float dt, CStateManager& mgr) {}

void CFirstPersonCamera::Render(const CStateManager& mgr) const {}

void CFirstPersonCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  SetTransformAlt(xf);
  SetTranslation(Player(mgr).GetEyePosition());
  mGunFollowXf = GetTransform();
  mPitchId = kInvalidUniqueId;
  mPitchTransitionTimer = 0.f;
}

void CFirstPersonCamera::SkipCinematic() {
  mCloseInVec = CVector3f::Zero();
  mCloseInTimer = 0.f;
}

void CFirstPersonCamera::Think(float dt, CStateManager& mgr) {
  // TODO: gate on player health/morph state, process pending fluid effects, update pitch/transform,
  // validate the result, and apply Echoes's death-camera transition before CActor::Think.
}

const CTransform4f& CFirstPersonCamera::GetGunFollowTransform() const { return mGunFollowXf; }

void CFirstPersonCamera::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CGameCamera::AcceptScriptMsg(mgr, msg);
  if (msg.GetMessage() == kSM_XALD) {
    mPitchId = kInvalidUniqueId;
  }
}

CVector3f CFirstPersonCamera::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  return GetTranslation() + 5.f * GetTransform().GetForward();
}

void CFirstPersonCamera::UnkVtable84() {}

void CFirstPersonCamera::UnkVtable88(TUniqueId fluidId) {
  mFluidEffectsPending = true;
  mPendingFluidId = fluidId;
}

void CFirstPersonCamera::UpdateFluidEffects(CStateManager& mgr) {
  // TODO: create the water-sheet HUD effect and play the fluid entry/exit sound for this player.
  mPendingFluidId = kInvalidUniqueId;
}

void CFirstPersonCamera::SetScriptPitchId(TUniqueId uid) {
  mPitchId = uid;
  mPitchTransitionTimer = 1.f;
}
