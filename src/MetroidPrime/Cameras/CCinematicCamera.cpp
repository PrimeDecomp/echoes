#include "MetroidPrime/Cameras/CCinematicCamera.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimeKeyframe.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/TCastTo.hpp"

CCinematicCamera::CCinematicCamera(TUniqueId uid, const CTransform4f& xf, bool active, float fov,
                                   float nearZ, float farZ, float aspect, int index,
                                   int controllerIdx)
: CGameCamera(uid, rstl::string_l("Cinematic Camera"),
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
    CScriptCameraSpline& spline = camera->GetSpline();
    if ((mFlags & CScriptCamera::kF_VerticalFov) != 0) {
      SetFovAndTarget(spline.GetFovByTime(mTime));
    } else {
      SetFovAndTarget(spline.GetFovByTime(mTime) / GetAspectRatio());
    }
    mMoveIntoEyePos = CalculateMoveOutofIntoEyePosition(false, mgr);
    Think(0.f, mgr);
  }
}

const bool CCinematicCamera::CanSkip(const CStateManager& mgr) const {
  bool result = false;
  if (gpGameState->GetHardModeEnabled()) {
    result = true;
  } else if (const CScriptCamera* camera =
                 TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId))) {
    result = camera->HasBeenViewed();
  }
  return result;
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
  CScriptCameraSpline& spline = camera->GetSpline();
  if (!mPaused) {
    mTime += dt;
  }

  CTransform4f xf = camera->GetTransform();
  const float roll = spline.GetRollByTime(mTime);
  CVector3f up = CVector3f::Up();
  if (!close_enough(roll, 0.f, 0.0001f)) {
    up = CQuaternion::YRotation(CRelAngle::FromDegrees(roll)).Transform(CVector3f::Up());
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
    if (CVector3f(target - xf.GetTranslation()).ToVec2f().Magnitude() < 0.0011920929f) {
      xf.SetTranslation(target);
    } else {
      xf = CTransform4f::LookAt(xf.GetTranslation(), target, up);
    }
  }
  SetTransform(xf);

  if ((mFlags & CScriptCamera::kF_VerticalFov) != 0) {
    SetFovAndTarget(spline.GetFovByTime(mTime));
  } else {
    SetFovAndTarget(spline.GetFovByTime(mTime) / GetAspectRatio());
  }
  mMoveIntoEyePos = CalculateMoveOutofIntoEyePosition(false, mgr);

  if (CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(camera->GetCameraActorId()))) {
    if (actor->IsPlayerActor()) {
      actor->SetModelFlags(CModelFlags::AlphaBlended(GetMoveOutofIntoAlpha()));
    }
  }
  if (mTime > camera->GetDuration()) {
    mgr.CameraManager(GetControllerNumber())->StopCinematics(mgr);
  }

  if (CScriptTimeKeyframe* keyframe =
          TCastToPtr< CScriptTimeKeyframe >(mgr.ObjectById(camera->GetTimeKeyframeId()))) {
    keyframe->SetTime(mTime, mgr);
  }
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
  static const char* skLeftEyeLocator = "L_eye";
  static const char* skRightEyeLocator = "R_eye";
  const CPlayer& player = Player(const_cast< CStateManager& >(mgr));
  CVector3f eyePos = player.GetEyePosition();
  const CScriptCamera* camera =
      TCastToConstPtr< CScriptCamera >(mgr.GetObjectById(mScriptCameraId));
  if (camera == nullptr) {
    return eyePos;
  }
  const CScriptActor* actor =
      TCastToConstPtr< CScriptActor >(mgr.GetObjectById(camera->GetCameraActorId()));
  if (actor && actor->IsPlayerActor() && actor->HasAnimation()) {
    const CAnimData* animData = actor->GetModelData()->GetAnimationData();
    const rstl::ncrc_ptr< CAnimTreeNode >& tree = animData->GetAnimationTree();
    if (!tree.IsNull()) {
      const CModelData* modelData = actor->GetModelData();
      const CSegId leftEye = animData->GetLocatorSegId(rstl::string_l(skLeftEyeLocator));
      const CSegId rightEye = animData->GetLocatorSegId(rstl::string_l(skRightEyeLocator));
      if (leftEye != CSegId::Invalid() && rightEye != CSegId::Invalid()) {
        const CCharAnimTime time = outOfEye ? CCharAnimTime::ZeroFlat()
                                            : tree->VGetSteadyStateAnimInfo().GetDuration();
        const CCharAnimTime* timePtr = outOfEye ? nullptr : &time;
        const CTransform4f leftLocator =
            modelData->GetScaledLocatorTransformDynamic(rstl::string_l(skLeftEyeLocator), timePtr);
        const CTransform4f leftXf = actor->GetTransform() * leftLocator;
        const CTransform4f rightLocator = modelData->GetScaledLocatorTransformDynamic(
            rstl::string_l(skRightEyeLocator), timePtr);
        const CTransform4f rightXf = actor->GetTransform() * rightLocator;
        eyePos = 0.5f * (leftXf.GetTranslation() + rightXf.GetTranslation());
      }
    }
  }
  return eyePos;
}
