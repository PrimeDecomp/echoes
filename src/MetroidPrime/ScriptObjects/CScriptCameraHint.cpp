#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"

#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPathCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"

CEntity* LoadCameraHint(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrCameraHint sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrCameraHint.inc"

  uint overrideFlags = 0;
  if (sldrThis.minSpeedDistance.field_override) {
    overrideFlags |= 0x1;
  }
  if (sldrThis.maxSpeedDistance.field_override) {
    overrideFlags |= 0x2;
  }
  if (sldrThis.backwardsDistance.field_override) {
    overrideFlags |= 0x4;
  }
  if (sldrThis.lookAtOffset.field_override) {
    overrideFlags |= 0x8;
  }
  if (sldrThis.fieldOfView.field_override) {
    overrideFlags |= 0x10;
  }
  if (sldrThis.attitudeRange.field_override) {
    overrideFlags |= 0x20;
  }
  if (sldrThis.angularSpeed.field_override) {
    overrideFlags |= 0x80;
  }
  if (sldrThis.azimuthRange.field_override) {
    overrideFlags |= 0x40;
  }
  if (sldrThis.zOffset.field_override) {
    overrideFlags |= 0x100;
  }

  int acrossAreas = 0;
  if (sldrThis.flagsCameraHint & 0x2000000) {
    acrossAreas = 1;
  }

  return rs_new CScriptCameraHint(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties),
      LdrToTransform4f(sldrThis.editorProperties), sldrThis.priority, sldrThis.timer,
      static_cast< CBallCamera::EBallCameraBehaviour >(sldrThis.behaviour.behaviourType),
      sldrThis.flagsCameraHint, overrideFlags, sldrThis.minSpeedDistance.distance,
      sldrThis.maxSpeedDistance.distance, sldrThis.backwardsDistance.distance,
      sldrThis.lookAtOffset.offset, sldrThis.worldOffset, sldrThis.fieldOfView.fOV,
      sldrThis.attitudeRange.angle * (M_PIF / 180.f),
      sldrThis.azimuthRange.angle * (M_PIF / 180.f),
      sldrThis.angularSpeed.speed * (M_PIF / 180.f), sldrThis.zOffset.zOffset,
      sldrThis.interpolateOnTime, sldrThis.interpolateOffTime,
      sldrThis.interpolateControlTime, sldrThis.positionInterpolationOn.type,
      sldrThis.rotationInterpolation.type, sldrThis.positionInterpolationOff.type,
      acrossAreas);
}

CCameraOverrideInfo::CCameraOverrideInfo(
    uint flags, uint overrideFlags, CBallCamera::EBallCameraBehaviour behaviour, float minDist,
    float maxDist, float backwardsDist, const CVector3f& lookAtOffset, const CVector3f& worldOffset,
    float fov, float attitudeRange, float azimuthRange, float anglePerSecond, float elevation,
    float interpolateOnTime, float interpolateOffTime, float controlInterpDur,
    int interpolateOnType, int interpolationMode, int interpolateOffType)
: mFlags(flags)
, mOverrideFlags(overrideFlags)
, mBehaviour(behaviour)
, mMinDist(minDist)
, mMaxDist(maxDist)
, mBackwardsDist(backwardsDist)
, mLookAtOffset(lookAtOffset)
, mWorldOffset(worldOffset)
, mFov(fov)
, mAttitudeRange(attitudeRange)
, mAzimuthRange(azimuthRange)
, mAnglePerSecond(anglePerSecond)
, mElevation(elevation)
, mInterpolateOnTime(interpolateOnTime)
, mInterpolateOffTime(interpolateOffTime)
, mControlInterpDur(controlInterpDur)
, mInterpolateOnType(interpolateOnType)
, mInterpolationMode(interpolationMode)
, mInterpolateOffType(interpolateOffType) {}

CCameraOverrideInfo::~CCameraOverrideInfo() {}

CScriptCameraHint::CScriptCameraHint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf, int priority,
                                     float timer, CBallCamera::EBallCameraBehaviour behaviour,
                                     uint flags, uint overrideFlags, float minDist, float maxDist,
                                     float backwardsDist, const CVector3f& lookAtOffset,
                                     const CVector3f& worldOffset, float fov, float attitudeRange,
                                     float azimuthRange, float anglePerSecond, float elevation,
                                     float interpolateOnTime, float interpolateOffTime,
                                     float controlInterpDur, int interpolateOnType,
                                     int interpolationMode, int interpolateOffType, int acrossAreas)
: CGameHint(uid, name, info, xf, priority, timer, acrossAreas, kBHT_None, 0, 0, 0.f, SCallback(),
            SCallback(), 0.f)
, mOverrideInfo(flags, overrideFlags, behaviour, minDist, maxDist, backwardsDist, lookAtOffset,
                worldOffset, fov, attitudeRange, azimuthRange, anglePerSecond, elevation,
                interpolateOnTime, interpolateOffTime, controlInterpDur, interpolateOnType,
                interpolationMode, interpolateOffType)
, mDelegatedCameraId(kInvalidUniqueId)
, mCameraTargetId(kInvalidUniqueId)
, mOrigXf(xf) {}

CScriptCameraHint::~CScriptCameraHint() {}

void CScriptCameraHint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();
  uint playerIndex = 0;
  if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(msg.GetOriginator()))) {
    playerIndex = mgr.MaskUIdNumPlayers(msg.GetOriginator());
  }
  if (const CGameCamera* camera =
          TCastToConstPtr< CGameCamera >(mgr.GetObjectById(msg.GetOriginator()))) {
    playerIndex = camera->GetControllerNumber();
  }

  if (playerIndex < mgr.GetNumPlayers()) {
    CHintManager* hints = mgr.CameraManager(playerIndex)->HintManager();
    switch (message) {
    case kSM_Deactivate:
    case kSM_Delete:
    case kSM_SetToZero:
      hints->ForceRemoveHint(GetUniqueId(), mgr, kInvalidUniqueId);
      break;
    default:
      break;
    }

    if (GetActive()) {
      switch (message) {
      case kSM_Increment:
        hints->AddHint(GetUniqueId(), sender, mgr);
        break;
      case kSM_Decrement:
        hints->RemoveHint(GetUniqueId(), sender, mgr);
        break;
      default:
        break;
      }
    }

    if (message == kSM_Follow) {
      if (!GetActive()) {
        SetActive(true);
      }
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(sender))) {
        CVector3f direction = mOrigXf.GetTranslation() - actor->GetTranslation();
        direction.SetZ(0.f);
        if (direction.CanBeNormalized()) {
          direction.Normalize();
        } else {
          direction = actor->GetTransform().GetColumn(kDY);
        }
        CVector3f position = actor->GetTranslation();
        position.SetZ(mOrigXf.GetTranslation().GetZ());
        SetTransform(CTransform4f::LookAt(position, position + direction));

        if (mOverrideInfo.GetBehaviourType() == CBallCamera::kBCB_Unknown8) {
          // The script spindle actor is distinct from the runtime CSpindleCamera.
          if (CActor* camera = static_cast< CActor* >(
                  TryCast(mgr.ObjectById(mDelegatedCameraId), kET_ScriptSpindleCamera))) {
            camera->SetTransform(GetTransform());
          }
        }
      }
      hints->AddHint(GetUniqueId(), sender, mgr);
    }
  }

  if (message == kSM_AreaLoaded) {
    mDelegatedCameraId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    mCameraTargetId = FindConnectedObject(mgr, kSS_CameraTarget, kSM_Attach);
  }
  CGameHint::AcceptScriptMsg(mgr, msg);
}

void CScriptCameraHint::SetPathCameraPosition(const CVector3f& position, CStateManager& mgr) const {
  if (CScriptPathCamera* camera =
          TCastToPtr< CScriptPathCamera >(mgr.ObjectById(mDelegatedCameraId))) {
    camera->TranslateSplines(position);
  }
}

void CScriptCameraHint::SetPathCameraRotation(const CQuaternion& rotation, const CVector3f& pivot,
                                             CStateManager& mgr) const {
  if (CScriptPathCamera* camera =
          TCastToPtr< CScriptPathCamera >(mgr.ObjectById(mDelegatedCameraId))) {
    camera->RotateSplines(rotation, pivot);
  }
}
