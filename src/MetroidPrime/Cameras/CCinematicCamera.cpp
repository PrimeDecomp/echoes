#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

CCinematicCamera::CCinematicCamera(TUniqueId uid, const CTransform4f& xf, bool active, float fov,
                                   float nearZ, float farZ, float aspect, int index,
                                   int controllerIdx)
: CGameCamera(uid, rstl::string("Cinematic Camera"),
              CEntityInfo(kInvalidAreaId, NullConnectionList, active), xf, fov, nearZ, farZ, aspect,
              kInvalidUniqueId, index, controllerIdx)
, mTime(0.f)
, mMoveIntoEyePos(CVector3f::Zero())
, mScriptCameraId(kInvalidUniqueId)
, mFlags(0)
, mPaused(false) {
  // The original initializes mSlowMotionScale in Reset, not here.
}

CCinematicCamera::~CCinematicCamera() {}

void CCinematicCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {
  SetTransform(xf);
  mTime = 0.f;
  mPaused = false;
  mSlowMotionScale = 1.f;
  if (const CScriptCamera* camera =
          TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId))) {
    if (mFlags & CScriptCamera::kF_VerticalFov) {
      SetFovAndTarget(camera->GetSpline().GetFovByTime(mTime));
    } else {
      SetFovAndTarget(camera->GetSpline().GetFovByTime(mTime) / GetAspectRatio());
    }
    mMoveIntoEyePos = CalculateMoveOutofIntoEyePosition(false, mgr);
    Think(0.f, mgr);
  }
}

bool CCinematicCamera::CanSkip(const CStateManager& mgr) const {
  if (gpGameState->GetHardModeEnabled()) {
    return true;
  }
  if (const CScriptCamera* camera =
          TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId))) {
    return camera->HasBeenViewed();
  }
  return false;
}

void CCinematicCamera::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CScriptCamera* camera =
      TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId));
  if (!camera) {
    mgr.CameraManager(GetControllerNumber())->StopCinematics(mgr);
    return;
  }
  if (!mPaused) {
    mTime += dt;
  }

  CScriptCameraSpline& spline = camera->GetSpline();
  CTransform4f xf = camera->GetTransform();
  const float roll = spline.GetRollByTime(mTime);
  CVector3f up = CVector3f::Up();
  if (!CMath::IsEpsilon(roll, 0.f, 0.0001f)) {
    up = CQuaternion::YRotation(CRelAngle::FromDegrees(roll)).Transform(up);
  }
  xf.SetTranslation(spline.GetPositionByTime(mTime, xf, mgr));
  const CQuaternion orientation = spline.GetOrientationByTime(mTime, xf, mgr);
  xf = orientation.BuildTransform4f(xf.GetTranslation());
  xf = CTransform4f::LookAt(xf.GetTranslation(), xf.GetTranslation() + xf.GetForward(), up);
  if ((mFlags & CScriptCamera::kF_LookAtPlayer) != 0) {
    const CPlayer& player = Player(mgr);
    CVector3f target = player.GetEyePosition();
    if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      target = player.GetBallPosition();
    }
    if ((target - xf.GetTranslation()).ToVec2f().Magnitude() >= 0.0011920929f) {
      xf = CTransform4f::LookAt(xf.GetTranslation(), target, up);
    } else {
      xf.SetTranslation(target);
    }
  }
  SetTransform(xf);

  float fov = spline.GetFovByTime(mTime);
  if ((mFlags & CScriptCamera::kF_VerticalFov) == 0) {
    fov /= GetAspectRatio();
  }
  SetTargetFov(fov);
  mMoveIntoEyePos = CalculateMoveOutofIntoEyePosition(false, mgr);

  // TODO: fade the linked player actor using GetMoveOutofIntoAlpha once CScriptActor's
  // player-actor flag is recovered in its shared interface.
  if (mTime > camera->GetDuration()) {
    mgr.CameraManager(GetControllerNumber())->StopCinematics(mgr);
  }

  // TODO: forward mTime to the connected CScriptTimeKeyframe once its interface is recovered.
  if ((mFlags & CScriptCamera::kF_SlowMotion) != 0) {
    mSlowMotionScale = camera->GetSlowMotionSpline().EvaluateAt(mTime);
  }
  CActor::Think(dt, mgr);
}

void CCinematicCamera::Render(const CStateManager& mgr) const {}

void CCinematicCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

float CCinematicCamera::GetMoveOutofIntoAlpha() const {
  const float start = 0.25f + GetNearClipDistance();
  const float end = 1.f + start;
  const float distance = (GetTranslation() - mMoveIntoEyePos).Magnitude();
  float alpha = 0.f;
  if (distance >= start && distance <= end) {
    alpha = (distance - start) / (end - start);
  } else if (distance > end) {
    alpha = 1.f;
  }
  return alpha;
}

CVector3f CCinematicCamera::CalculateMoveOutofIntoEyePosition(bool outOfEye,
                                                              const CStateManager& mgr) const {
  static const char* leftEyeName = "L_eye";
  static const char* rightEyeName = "R_eye";
  CVector3f eyePos = GetPlayer(mgr).GetEyePosition();
  if (const CScriptCamera* camera =
          TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId))) {
    if (const CScriptActor* actor =
            TCastToConstPtr< CScriptActor >(mgr.GetObjectById(camera->GetCameraActorId()))) {
      if (actor->IsPlayerActor() && actor->HasAnimation()) {
        const CAnimData* animData = actor->GetAnimationData();
        const rstl::ncrc_ptr< CAnimTreeNode >& root = animData->GetAnimationTree();
        if (root.GetPtr()) {
          const CModelData* modelData = actor->GetModelData();
          const CSegId leftEye = animData->GetLocatorSegId(rstl::string_l(leftEyeName));
          const CSegId rightEye = animData->GetLocatorSegId(rstl::string_l(rightEyeName));
          if (leftEye != CSegId::Invalid() && rightEye != CSegId::Invalid()) {
            const CCharAnimTime time =
                outOfEye ? CCharAnimTime::ZeroFlat() : root->GetSteadyStateAnimInfo().GetDuration();
            const CCharAnimTime* timePtr = outOfEye ? nullptr : &time;
            const CTransform4f leftLocal =
                modelData->GetScaledLocatorTransformDynamic(rstl::string_l(leftEyeName), timePtr);
            const CTransform4f leftWorld = actor->GetTransform() * leftLocal;
            const CTransform4f rightLocal =
                modelData->GetScaledLocatorTransformDynamic(rstl::string_l(rightEyeName), timePtr);
            const CTransform4f rightWorld = actor->GetTransform() * rightLocal;
            eyePos = (leftWorld.GetTranslation() + rightWorld.GetTranslation()) * 0.5f;
          }
        }
      }
    }
  }
  return eyePos;
}
