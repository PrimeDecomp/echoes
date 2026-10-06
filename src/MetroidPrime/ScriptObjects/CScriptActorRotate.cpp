#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptActorRotate::~CScriptActorRotate() {}

void CScriptActorRotate::StopRotation() { mPlaying = false; }

void CScriptActorRotate::StartRotation() { mPlaying = true; }

void CScriptActorRotate::SetCurrentTime(float time) {
  mCurrentTime = CMath::Clamp(0.f, time, mDuration);
}

void CScriptActorRotate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  bool accepted = false;

  switch (message) {
  case kSM_Activate:
    CEntity::AcceptScriptMsg(mgr, msg);
    accepted = true;
  case kSM_AreaLoaded:
    mTargetId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    if ((mFlags & kF_AutoStart) == 0 || !GetActive()) {
      break;
    }
    // Fall through: activation may start the rotation.
  case kSM_Action:
  case kSM_Next:
    if (GetActive()) {
      if (TCastToConstPtr< CScriptActorRotate >(mgr.GetObjectById(mTargetId))) {
        StartRotation();
        mCurrentTime = 0.f;
      } else {
        UpdateActors(message == kSM_Next, mgr);
      }
    }
    break;
  case kSM_Deactivate: {
    const rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Play, kSM_Play);
    for (int i = 0; i < ids.size(); ++i) {
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(ids[i]))) {
        platform->SetActorRotateId(kInvalidUniqueId);
      }
    }
    StopRotation();
    break;
  }
  case kSM_Start:
    StartRotation();
    break;
  case kSM_Stop:
    StopRotation();
    break;
  default:
    break;
  }

  if (!accepted) {
    CEntity::AcceptScriptMsg(mgr, msg);
  }
}

void CScriptActorRotate::UpdateActors(bool next, CStateManager& mgr) {
  if (mPlaying) {
    return;
  }

  mActors.clear();
  const rstl::vector< TUniqueId > ids = FindConnectedObjects(mgr, kSS_Play, kSM_Play);
  if (ids.size() > 0) {
    mActors.reserve(ids.size());
    for (int i = 0; i < ids.size(); ++i) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(ids[i]))) {
        mActors.push_back(rstl::pair< TUniqueId, CTransform4f >(
            actor->GetUniqueId(), actor->GetTransform().GetRotation()));
      }
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(ids[i]))) {
        platform->SetActorRotateId(GetUniqueId());
      }
    }
  }

  SendScriptMsgs(kSS_Play, mgr, kInvalidUniqueId, kSM_None);
  if (!mActors.empty()) {
    StartRotation();
    if (next) {
      mCurrentTime = mDuration;
    } else {
      mCurrentTime = 0.f;
    }
  }
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr) {
  if (!mPlaying || !GetActive()) {
    return;
  }

  if ((mFlags & kF_AdvanceTime) != 0 && (mFlags & kF_ExternalTime) == 0) {
    mCurrentTime += dt;
  }

  if (TCastToPtr< CScriptActorRotate >(mgr.ObjectById(mTargetId))) {
    UpdateTargetRotation(mgr);
  } else {
    UpdateActorRotations(dt, mgr);
  }
}

void CScriptActorRotate::UpdateTargetRotation(CStateManager& mgr) {
  CheckEnd(mgr);
  CScriptActorRotate* target = TCastToPtr< CScriptActorRotate >(mgr.ObjectById(mTargetId));
  if (target == nullptr) {
    return;
  }

  const float xAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mXRotation.EvaluateAt(mCurrentTime)).AsRadians());
  const float yAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mYRotation.EvaluateAt(mCurrentTime)).AsRadians());
  const float zAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mZRotation.EvaluateAt(mCurrentTime)).AsRadians());
  const CTransform4f rotation = CTransform4f::RotateZ(CRelAngle::FromRadians(zAngle)) *
                                CTransform4f::RotateY(CRelAngle::FromRadians(yAngle)) *
                                CTransform4f::RotateX(CRelAngle::FromRadians(xAngle));
  target->SetActorTransforms(rotation);
}

void CScriptActorRotate::SetActorTransforms(const CTransform4f& rotation) {
  for (rstl::vector< rstl::pair< TUniqueId, CTransform4f > >::iterator it = mActors.begin();
       it != mActors.end(); ++it) {
    it->second = rotation;
  }
}

void CScriptActorRotate::UpdateActorRotations(float dt, CStateManager& mgr) {
  CheckEnd(mgr);
  float xAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mXRotation.EvaluateAt(mCurrentTime)).AsRadians());
  float yAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mYRotation.EvaluateAt(mCurrentTime)).AsRadians());
  float zAngle =
      CMath::ClampRadians(CRelAngle::FromDegrees(mZRotation.EvaluateAt(mCurrentTime)).AsRadians());
  if ((mFlags & kF_AngularVelocity) != 0) {
    xAngle = dt * CRelAngle::FromDegrees(mXRotation.EvaluateAt(mCurrentTime)).AsRadians();
    yAngle = dt * CRelAngle::FromDegrees(mYRotation.EvaluateAt(mCurrentTime)).AsRadians();
    zAngle = dt * CRelAngle::FromDegrees(mZRotation.EvaluateAt(mCurrentTime)).AsRadians();
  }

  const CTransform4f rotation = CTransform4f::RotateZ(CRelAngle::FromRadians(zAngle)) *
                                CTransform4f::RotateY(CRelAngle::FromRadians(yAngle)) *
                                CTransform4f::RotateX(CRelAngle::FromRadians(xAngle));
  for (rstl::vector< rstl::pair< TUniqueId, CTransform4f > >::iterator it = mActors.begin();
       it != mActors.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->first))) {
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(it->first))) {
        if ((mFlags & kF_AngularVelocity) != 0) {
          if ((mFlags & kF_LocalRotation) != 0) {
            CTransform4f xf = platform->GetTransform() * rotation;
            xf.Orthonormalize();
            platform->SetTransformExplicitly(xf);
          } else {
            CTransform4f xf = rotation * platform->GetTransform();
            xf.Orthonormalize();
            platform->SetTransformExplicitly(xf);
          }
        } else {
          if ((mFlags & kF_LocalRotation) != 0) {
            CTransform4f xf = it->second * rotation;
            xf.Orthonormalize();
            platform->SetTransformExplicitly(xf);
          } else {
            CTransform4f xf = rotation * it->second;
            xf.Orthonormalize();
            platform->SetTransformExplicitly(xf);
          }
        }
      } else {
        if (!mPlaying) {
          mCurrentTransform = rotation;
        }
        if ((mFlags & kF_AngularVelocity) != 0) {
          if ((mFlags & kF_LocalRotation) != 0) {
            CTransform4f xf = actor->GetTransform() * mCurrentTransform;
            xf.Orthonormalize();
            xf.SetTranslation(actor->GetTranslation());
            actor->SetTransform(xf);
          } else {
            CTransform4f xf = mCurrentTransform * actor->GetTransform();
            xf.Orthonormalize();
            xf.SetTranslation(actor->GetTranslation());
            actor->SetTransform(xf);
          }
        } else {
          if ((mFlags & kF_LocalRotation) != 0) {
            CTransform4f xf = it->second * mCurrentTransform;
            xf.Orthonormalize();
            xf.SetTranslation(actor->GetTranslation());
            actor->SetTransform(xf);
          } else {
            CTransform4f xf = mCurrentTransform * it->second;
            xf.Orthonormalize();
            xf.SetTranslation(actor->GetTranslation());
            actor->SetTransform(xf);
          }
        }
      }

      CScriptEffect* effect = TCastToPtr< CScriptEffect >(actor);
      CVector3f scale(1.f, 1.f, 1.f);
      if (actor->HasModelData()) {
        scale = actor->GetModelData()->GetScale();
      } else if (effect) {
        scale = effect->GetGlobalScale();
      }
      if (!mXScale.GetKnots().empty()) {
        scale.SetX(mXScale.EvaluateAt(mCurrentTime));
      }
      if (!mYScale.GetKnots().empty()) {
        scale.SetY(mYScale.EvaluateAt(mCurrentTime));
      }
      if (!mZScale.GetKnots().empty()) {
        scale.SetZ(mZScale.EvaluateAt(mCurrentTime));
      }
      if (actor->HasModelData()) {
        actor->ModelData()->SetScale(scale);
      } else if (effect) {
        effect->SetGlobalScale(scale);
      }
    }
  }

  if (mPlaying) {
    mCurrentTransform = rotation;
  } else {
    mCurrentTransform = CTransform4f::Identity();
  }
}

void CScriptActorRotate::CheckEnd(CStateManager& mgr) {
  if (!(mCurrentTime >= mDuration)) {
    return;
  }
  SendScriptMsgs(kSS_Zero, mgr, kInvalidUniqueId, kSM_None);
  if ((mFlags & kF_Loop) != 0) {
    mCurrentTime -= mDuration;
  } else {
    StopRotation();
    mCurrentTime = mDuration;
  }
}

CScriptActorRotate::CScriptActorRotate(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, uint flags,
                                       const SLdrSpline& xRotation, const SLdrSpline& yRotation,
                                       const SLdrSpline& zRotation, const SLdrSpline& xScale,
                                       const SLdrSpline& yScale, const SLdrSpline& zScale,
                                       float duration)
: CEntity(uid, info, name, 0)
, mDuration(duration)
, mXRotation(xRotation)
, mYRotation(yRotation)
, mZRotation(zRotation)
, mXScale(xScale)
, mYScale(yScale)
, mZScale(zScale)
, mFlags(flags)
, mCurrentTime(0.f)
, mCurrentTransform(CTransform4f::Identity())
, mActors()
, mTargetId(kInvalidUniqueId)
, mPlaying(false) {
  if ((mFlags & kF_DurationFromSplines) != 0) {
    mDuration = mXRotation.GetMaxTime();
    float maxTime = mYRotation.GetMaxTime();
    mDuration = maxTime < mDuration ? mDuration : maxTime;
    maxTime = mZRotation.GetMaxTime();
    mDuration = maxTime < mDuration ? mDuration : maxTime;
    maxTime = mXScale.GetMaxTime();
    mDuration = maxTime < mDuration ? mDuration : maxTime;
    maxTime = mYScale.GetMaxTime();
    mDuration = maxTime < mDuration ? mDuration : maxTime;
    maxTime = mZScale.GetMaxTime();
    mDuration = maxTime < mDuration ? mDuration : maxTime;
  }
}

CEntity* LoadActorRotate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrActorRotate sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrActorRotate.inc"

  return rs_new CScriptActorRotate(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.flagsActorRotate,
      sldrThis.rotationControls.xRotation, sldrThis.rotationControls.yRotation,
      sldrThis.rotationControls.zRotation, sldrThis.scaleControls.xScale,
      sldrThis.scaleControls.yScale, sldrThis.scaleControls.zScale, sldrThis.duration);
}
