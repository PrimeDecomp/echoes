#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptActorRotate::~CScriptActorRotate() {}

void CScriptActorRotate::StopRotation() { mPlaying = false; }

void CScriptActorRotate::StartRotation() { mPlaying = true; }

void CScriptActorRotate::SetCurrentTime(float time) {
  if (time < 0.f) {
    mCurrentTime = 0.f;
  } else if (time > mDuration) {
    mCurrentTime = mDuration;
  } else {
    mCurrentTime = time;
  }
}

void CScriptActorRotate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  bool accepted = false;

  if (message == kSM_Activate) {
    CEntity::AcceptScriptMsg(mgr, msg);
    accepted = true;
  }

  switch (message) {
  case kSM_Activate:
  case kSM_AreaLoaded:
    mTargetId = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    if ((mFlags & kF_AutoStart) == 0 || !GetActive()) {
      break;
    }
    // Fall through: activation may start the rotation.
  case kSM_Action:
  case kSM_Next:
    if (GetActive()) {
      const CEntity* target = mgr.GetObjectById(mTargetId);
      if (target != nullptr && target->TypesMatch(kET_ScriptActorRotate) != nullptr) {
        StartRotation();
        mCurrentTime = 0.f;
      } else {
        UpdateActors(message == kSM_Next, mgr);
      }
    }
    break;
  case kSM_Deactivate:
    // TODO: clear this controller's ID on connected script platforms.
    StopRotation();
    break;
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
  mActors.reserve(ids.size());
  for (rstl::vector< TUniqueId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(*it))) {
      mActors.push_back(rstl::pair< TUniqueId, CTransform4f >(*it, actor->GetTransform()));
    }
    // TODO: record this controller on any connected script platform.
  }

  SendScriptMsgs(kSS_Play, mgr, kInvalidUniqueId, kSM_None);
  if (!mActors.empty()) {
    StartRotation();
    mCurrentTime = next ? mDuration : 0.f;
  }
}

void CScriptActorRotate::Think(float dt, CStateManager& mgr) {
  if (!mPlaying || !GetActive()) {
    return;
  }

  if ((mFlags & kF_AdvanceTime) != 0 && (mFlags & kF_ExternalTime) == 0) {
    mCurrentTime += dt;
  }

  CEntity* target = mgr.GetObjectByIdFromListAll(mTargetId);
  if (target != nullptr && target->TypesMatch(kET_ScriptActorRotate) != nullptr) {
    UpdateTargetRotation(mgr);
  } else {
    UpdateActorRotations(dt, mgr);
  }
}

void CScriptActorRotate::UpdateTargetRotation(CStateManager& mgr) {
  CheckEnd(mgr);
  CEntity* entity = mgr.GetObjectByIdFromListAll(mTargetId);
  if (entity == nullptr) {
    return;
  }
  CScriptActorRotate* target =
      static_cast< CScriptActorRotate* >(entity->TypesMatch(kET_ScriptActorRotate));
  if (target == nullptr) {
    return;
  }

  const CTransform4f rotation =
      CTransform4f::RotateZ(CRelAngle::FromDegrees(mZRotation.EvaluateAt(mCurrentTime))) *
      CTransform4f::RotateY(CRelAngle::FromDegrees(mYRotation.EvaluateAt(mCurrentTime))) *
      CTransform4f::RotateX(CRelAngle::FromDegrees(mXRotation.EvaluateAt(mCurrentTime)));
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
  // TODO: apply the sampled rotation and scale splines to each connected actor. The
  // transform composition and platform-specific path still need target verification.
}

void CScriptActorRotate::CheckEnd(CStateManager& mgr) {
  if (mCurrentTime < mDuration) {
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
    if (maxTime > mDuration) {
      mDuration = maxTime;
    }
    maxTime = mZRotation.GetMaxTime();
    if (maxTime > mDuration) {
      mDuration = maxTime;
    }
    maxTime = mXScale.GetMaxTime();
    if (maxTime > mDuration) {
      mDuration = maxTime;
    }
    maxTime = mYScale.GetMaxTime();
    if (maxTime > mDuration) {
      mDuration = maxTime;
    }
    maxTime = mZScale.GetMaxTime();
    if (maxTime > mDuration) {
      mDuration = maxTime;
    }
  }
}
