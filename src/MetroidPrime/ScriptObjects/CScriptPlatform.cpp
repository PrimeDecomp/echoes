#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"

#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CScriptCameraSpline.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CPlatformWaypointTracker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSpline.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "rstl/algorithm.hpp"

CScriptPlatform::CScriptPlatform(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& model, const CActorParameters& params, const CAABox& bounds,
    const rstl::optional_object< TLockedToken< const COBBTreeGroup > >& dcln,
    const CHealthInfo& health, const CDamageVulnerability& vulnerability,
    const CMaterialList& materials, bool renderRainSplashes, uint maxRainSplashes, uint rainGenRate,
    const CGameSplineDesc& motionSpline, uint motionFlags, const CVector3f& conveyorVelocity,
    const CMayaSpline& rollSpline, const CMayaSpline& yawSpline, const CMayaSpline& pitchSpline,
    float initialTime, float randomAnimationOffset)
: CPhysicsActor(uid, name, info, 0, xf, model, materials, bounds, SMoverData(15000.f), params,
                skDefaultStepData)
, mMoveDelay(0.f)
, mCollisionRecoverDelay(0.f)
, mFadeInTime(params.GetFadeInTime())
, mFadeOutTime(params.GetFadeOutTime())
, mConveyorVelocity(conveyorVelocity)
, mDragDelta(CVector3f::Zero())
, mRotationDelta(CQuaternion::NoRotation())
, mPreviousRotation(xf.GetRotation())
, mCurrentRotation(xf.GetRotation())
, mInitialHealth(health)
, mHealth(health)
, mDamageVulnerability(vulnerability)
, mTreeGroupContainer(dcln)
, mMaxRainSplashes(maxRainSplashes)
, mRainGenRate(rainGenRate)
, mBoundsTrigger(kInvalidUniqueId)
, mMotionSpline(rs_new CGameSplineDesc(motionSpline))
, mSplineController(nullptr)
, mMotionTime(0.f)
, mMotionFlags(motionFlags)
, mInitialTime(initialTime)
, mMotionDuration(motionSpline.GetDuration())
, mWaypointTracker(nullptr)
, mRollSpline(nullptr)
, mYawSpline(nullptr)
, mPitchSpline(nullptr)
, mActorRotateId(kInvalidUniqueId)
, x452_(kInvalidUniqueId)
, mLookAtTarget(kInvalidUniqueId)
, mRandomAnimationOffset(randomAnimationOffset)
, mInitialTransform(xf)
, mDead(false)
, mControlledAnimation(false)
, mRenderRainSplashes(renderRainSplashes)
, mSquishedRider(false)
, mMotionActive(false)
, mPassedMotionEnd(false)
, mPassedMotionStart(false)
, mMotionForward(true)
, mPreviousMotionForward(true)
, x48d_25_(false)
, mMotionTransformed(false) {
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_NoStaticCollision, kMT_NoPlatformCollision, kMT_Platform)));
  SetMovable(false);
  if (HasAnimation()) {
    AnimationData()->EnableLooping(true);
    AnimationData()->SetIsAnimating(true);
  }
  if (mTreeGroupContainer) {
    mTreeGroup = rs_new CCollidableOBBTreeGroup(**mTreeGroupContainer, GetMaterialList());
  }
  if (rollSpline.GetKnotCount()) {
    mRollSpline = rs_new CMayaSpline(rollSpline);
  }
  if (yawSpline.GetKnotCount()) {
    mYawSpline = rs_new CMayaSpline(yawSpline);
  }
  if (pitchSpline.GetKnotCount()) {
    mPitchSpline = rs_new CMayaSpline(pitchSpline);
  }
}

CScriptPlatform::~CScriptPlatform() {}

rstl::optional_object< CAABox > CScriptPlatform::GetTouchBounds() const {
  if (GetActive()) {
    if (mTreeGroup.get()) {
      return mTreeGroup->CalculateAABox(GetTransform());
    }
    return GetBoundingBox();
  }
  return rstl::optional_object< CAABox >();
}

void CScriptPlatform::StopMotion() {
  mMotionActive = false;
  Stop();
  mPreviousRotation = GetTransform().GetRotation();
  mPreviousRotation.Orthonormalize();
  mCurrentRotation = mPreviousRotation;
  mDragDelta = CVector3f::Zero();
  mRotationDelta = CQuaternion::NoRotation();
}

void CScriptPlatform::fn_800a3d18() { StopMotion(); }

void CScriptPlatform::AdvanceMotionTime(float dt) {
  float motionTime;
  float delta = dt;
  mPreviousMotionForward = mMotionForward;
  mPassedMotionEnd = false;
  mPassedMotionStart = false;
  if (!mMotionForward) {
    delta = -dt;
  }
  float duration = 0.f;
  if (mMotionSpline.get()) {
    duration = mMotionSpline->GetDuration();
  }
  if (mSplineController.get()) {
    duration = mSplineController->GetPositionSpline().GetDuration();
  }
  const uint fixedDuration = mMotionFlags & 0x200;
  if (fixedDuration) {
    duration = mMotionDuration;
  }
  if (!mMotionActive && !fixedDuration) {
    return;
  }
  if (duration > 0.f) {
    mMotionTime += delta;
    const float invDuration = 1.f / duration;
    motionTime = mMotionTime;
    if (motionTime >= duration) {
      if ((mMotionFlags & 4) != 0) {
        const float loops = motionTime * invDuration;
        mMotionTime = motionTime - int(loops) * duration;
        mPassedMotionEnd = true;
      } else {
        mMotionTime = duration;
        fn_800a3d18();
      }
      mMotionForward = true;
    } else if (motionTime < 0.f) {
      if ((mMotionFlags & 4) != 0) {
        mMotionTime = -motionTime;
        mPassedMotionStart = true;
      } else {
        mMotionTime = 0.f;
        fn_800a3d18();
      }
      mMotionForward = true;
    }
  }
}

void CScriptPlatform::AddRider(rstl::vector< SRiders >& riders, TUniqueId id,
                               const CPhysicsActor* ridee, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  rstl::vector< SRiders >::iterator it =
      rstl::find(riders.begin(), riders.end(),
                 SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >()));
  if (it == riders.end()) {
    SRiders rider(id, CTransform4f::Identity(), rstl::optional_object< float >(decayTimer));
    if (ridee) {
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(id))) {
        const CVector3f offset = ridee->GetTransform().TransposeRotate(actor->GetTranslation() -
                                                                       ridee->GetTranslation());
        rider.mTransform = CTransform4f::Translate(offset);
        if (ridee) {
          mgr.DeliverScriptMsg(
              CScriptMsg(ridee->GetUniqueId(), actor->GetUniqueId(), EScriptObjectMessage('XONP')));
        }
      }
    } else {
      mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, id, EScriptObjectMessage('XONP')));
    }
    riders.reserve(riders.size() + 1);
    riders.push_back_unsafe(rider);
  } else {
    (*it).mDecayTimer = decayTimer;
  }
}

CScriptPlatform::TNearList
CScriptPlatform::BuildNearListFromRiders(CStateManager& mgr,
                                         const rstl::vector< SRiders >& riders) {
  TNearList result;
  for (rstl::vector< SRiders >::const_iterator it = riders.begin(); it != riders.end(); ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      result.push_back(actor->GetUniqueId());
    }
  }
  return result;
}

void CScriptPlatform::DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr) {
  rstl::vector< SRiders >::iterator it = riders.begin();
  while (it != riders.end()) {
    if (it->mDecayTimer.valid()) {
      *it->mDecayTimer -= dt;
      if (*it->mDecayTimer <= 0.f) {
        const TUniqueId id = it->mUid;
        it = riders.erase(it);
        mgr.DeliverScriptMsg(CScriptMsg(kInvalidUniqueId, id, EScriptObjectMessage('XONP')));
        continue;
      }
    }
    ++it;
  }
}

void CScriptPlatform::MoveRiders(CStateManager& mgr, float dt, bool active,
                                 rstl::vector< SRiders >& riders,
                                 rstl::vector< SRiders >& collidedRiders, const TNearList& nearList,
                                 const CTransform4f& oldXf, const CTransform4f& newXf,
                                 const CVector3f& dragDelta, CQuaternion rotDelta) {
  rstl::vector< SRiders >::iterator it = riders.begin();
  while (it != riders.end()) {
    if (!active) {
      ++it;
      continue;
    }
    CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(it->mUid));
    if (!actor || !actor->GetActive()) {
      ++it;
      continue;
    }

    CVector3f rotationDelta = newXf.Rotate(it->mTransform.GetTranslation()) -
                              oldXf.Rotate(it->mTransform.GetTranslation());
    bool clearZ = true;
    if (CPlayer* player = TCastToPtr< CPlayer >(*actor)) {
      if (mTreeGroup.null()) {
        rotationDelta = CVector3f::Zero();
      }
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        clearZ = false;
      }
    }
    if (clearZ) {
      rotationDelta.SetZ(0.f);
    }
    const CVector3f displacement = dragDelta + rotationDelta;
    const CVector3f translation = actor->GetTranslation() + displacement;
    actor->MoveCollisionPrimitive(displacement);
    const bool collision = CGameCollision::DetectCollisionBoolean(
        mgr, *actor->GetCollisionPrimitive(), actor->GetPrimitiveTransform(),
        actor->GetMaterialFilter(), nearList);
    actor->MoveCollisionPrimitive(CVector3f::Zero());
    if (collision) {
      it = riders.erase(it);
      AddRider(collidedRiders, actor->GetUniqueId(), nullptr, mgr,
               rstl::optional_object< float >(1.f / 6.f));
      continue;
    }

    CPlayer* player = TCastToPtr< CPlayer >(*actor);
    if (player && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
        !mTreeGroup.null()) {
      player->GetMorphBall()->TransformSpiderBallState(rotDelta, dragDelta);
    }
    actor->SetTranslation(translation);
    if ((!player || player->GetOrbitState() == CPlayer::kOS_NoOrbit) && !mTreeGroup.null()) {
      actor->SetTransform((rotDelta * CQuaternion::FromMatrix(actor->GetTransform()))
                              .BuildTransform4f(actor->GetTranslation()));
    }
    ++it;
  }
}

void CScriptPlatform::PreThink(float dt, CStateManager& mgr) {
  DecayRiders(mRiders, dt, mgr);
  DecayRiders(mDynamicSlaves, dt, mgr);
  TNearList nearList;
  rstl::vector< SRiders > collidedRiders;
  if ((mMotionFlags & 0x40) != 0) {
    const CTransform4f xf = GetTransform();
    nearList.push_back(GetUniqueId());
    MoveRiders(mgr, dt, GetActive(), mRiders, collidedRiders, nearList, xf, xf, mConveyorVelocity,
               CQuaternion::NoRotation());
    return;
  }
  if (x48d_25_) {
    ResetMotion(mInitialTime, mgr);
    x48d_25_ = false;
  }
  if (!mMotionActive && !mMotionTransformed && mActorRotateId == kInvalidUniqueId &&
      x452_ == kInvalidUniqueId) {
    return;
  }

  UpdateSlaveTransforms(mgr);
  mCollisionRecoverDelay -= dt;
  mMoveDelay -= dt;
  if (mMoveDelay > 0.f) {
    return;
  }
  mDragDelta = CVector3f::Zero();
  const CTransform4f oldXf = GetTransform();
  const CMotionState oldMotion = GetMotionState();
  if (GetActive()) {
    for (int i = 0; i < mRiders.size(); ++i) {
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mRiders[i].mUid))) {
        mRiders[i].mTransform.SetTranslation(
            GetTransform().TransposeRotate(actor->GetTranslation() - GetTranslation()));
      }
    }
    if (mMotionActive || mMotionTransformed) {
      mRotationDelta = Move(dt, mgr);
    } else {
      mRotationDelta = CalculateRotationDelta();
    }
    SetTransform((mRotationDelta * CQuaternion::FromMatrix(GetTransform()))
                     .BuildTransform4f(GetTranslation()));
    SetTransform(CQuaternion::FromMatrix(GetTransform())
                     .BuildNormalized()
                     .BuildTransform4f(GetTranslation()));
  }
  const CTransform4f newXf = GetTransform();
  mDragDelta = newXf.GetTranslation() - oldXf.GetTranslation();
  MoveRiders(mgr, dt, GetActive(), mRiders, collidedRiders, nearList, oldXf, newXf, mDragDelta,
             mRotationDelta);
  mSquishedRider = false;
  if (!collidedRiders.empty()) {
    const TNearList collidedNear = BuildNearListFromRiders(mgr, collidedRiders);
    if (CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(),
                                                      GetPrimitiveTransform(), collidedNear, mgr)) {
      SetMotionState(oldMotion);
      Stop();
      mMoveDelay = 0.035f;
      MoveRiders(mgr, dt, GetActive(), mRiders, collidedRiders, nearList, newXf, oldXf, -mDragDelta,
                 mRotationDelta.BuildInverted());
      mDragDelta = CVector3f::Zero();
      SendScriptMsgs(EScriptObjectState('MDFY'), mgr, kSM_None);
      mSquishedRider = true;
      AdvanceMotionTime(-dt);
    }
  }
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr) {
  mStaticSlaves.reserve(GetConnectionList().size());
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Play && it->msg == kSM_Activate) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(it->objId)))) {
        actor->AddMaterial(kMT_PlatformSlave, mgr);
        CTransform4f xf = actor->GetTransform();
        xf.SetTranslation(actor->GetTranslation() - GetTranslation());
        mStaticSlaves.push_back_unsafe(
            SRiders(actor->GetUniqueId(), xf, rstl::optional_object< float >()));
      }
    } else if (it->state == kSS_InheritBounds && it->msg == kSM_Activate) {
      const CStateManager::TIdListResult ids = mgr.GetIdListForScript(it->objId);
      for (CStateManager::TIdList::const_iterator id = ids.first; id != ids.second; ++id) {
        if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(id->second))) {
          mBoundsTrigger = id->second;
        }
      }
    }
  }
}

void CScriptPlatform::DragSlave(CStateManager& mgr, TMovedList& moved, const SRiders& slave) {
  CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(slave.mUid));
  if (!actor) {
    return;
  }
  if (IsInMovedList(slave.mUid, moved)) {
    return;
  }
  moved.push_back(slave.mUid.Value());
  CTransform4f parent = CTransform4f::Identity();
  CExplosion* explosion = TCastToPtr< CExplosion >(actor);
  CScriptEffect* effect = TCastToPtr< CScriptEffect >(actor);
  CWeapon* weapon = TCastToPtr< CWeapon >(actor);
  if ((mMotionFlags & 0x20) != 0 || explosion || effect || weapon) {
    parent = GetTransform();
  }
  parent.SetTranslation(GetTranslation());
  const CTransform4f xf =
      parent * mInitialTransform.GetRotation().GetQuickInverse() * slave.mTransform;
  if (explosion || weapon) {
    actor->SetTransform(parent * slave.mTransform);
  } else {
    if ((mMotionFlags & 0x10) != 0) {
      actor->SetTransform(GetTransform());
      actor->SetTranslation(xf.GetTranslation());
    }
    if ((mMotionFlags & 0x2000) != 0) {
      CVector3f forward = GetTransform().GetForward();
      forward.SetZ(0.f);
      if (forward.CanBeNormalized()) {
        actor->SetTransform(CTransform4f::LookAt(xf.GetTranslation(), xf.GetTranslation() + forward,
                                                 CVector3f::Up()));
      } else {
        actor->SetTranslation(xf.GetTranslation());
      }
    }
    if ((mMotionFlags & 0x800) != 0) {
      actor->SetTransform(xf);
    } else {
      actor->SetTranslation(xf.GetTranslation());
    }
  }
  if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(actor)) {
    platform->DragSlaves(mgr, moved);
  }
}

void CScriptPlatform::DragSlaves(CStateManager& mgr, TMovedList& moved) {
  for (rstl::vector< SRiders >::const_iterator it = mStaticSlaves.begin();
       it != mStaticSlaves.end(); ++it) {
    const SRiders& slave = *it;
    if ((mMotionFlags & 0x400) != 0) {
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(mgr.ObjectById(slave.mUid))) {
        platform->TranslateMotion(mDragDelta);
        if ((mMotionFlags & 0x20) != 0) {
          platform->RotateMotion(mRotationDelta, GetTranslation());
        }
        continue;
      }
      if (CScriptCamera* camera = TCastToPtr< CScriptCamera >(mgr.ObjectById(slave.mUid))) {
        camera->TranslateSplines(mDragDelta);
        if ((mMotionFlags & 0x20) != 0) {
          camera->RotateSplines(mRotationDelta, GetTranslation());
        }
        continue;
      }
      if (CScriptCameraHint* hint = TCastToPtr< CScriptCameraHint >(mgr.ObjectById(slave.mUid))) {
        hint->SetPathCameraPosition(mDragDelta, mgr);
        if ((mMotionFlags & 0x20) != 0) {
          hint->SetPathCameraRotation(mRotationDelta, GetTranslation(), mgr);
        }
        continue;
      }
    }
    DragSlave(mgr, moved, slave);
  }

  rstl::vector< SRiders >::iterator it = mDynamicSlaves.begin();
  while (it != mDynamicSlaves.end()) {
    if (TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      DragSlave(mgr, moved, *it);
      ++it;
    } else {
      it = mDynamicSlaves.erase(it);
    }
  }
}

void CScriptPlatform::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if ((mMotionFlags & 0x40) == 0) {
    if (HasAnimation()) {
      if (!mControlledAnimation) {
        const float offset = mRandomAnimationOffset * mgr.Random()->Range(-1.f, 1.f) + dt;
        mRandomAnimationOffset = 0.f;
        UpdateAnimation(offset, mgr, true);
      }
      if (mRenderRainSplashes && mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain && HasModelData() &&
          mgr.GetEnvFxManager()->GetRainMagnitude() != 0.f) {
        mgr.ActorModelParticles()->StartRainSplashes(*this, mgr, mMaxRainSplashes, mRainGenRate,
                                                     0.f);
      }
    }
    if ((mMotionActive || mMotionTransformed || mActorRotateId != kInvalidUniqueId) &&
        (!mStaticSlaves.empty() || !mDynamicSlaves.empty())) {
      TMovedList moved;
      DragSlaves(mgr, moved);
    }
  }
  if (!mDead && GetHealthInfo()->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
  }
  mMotionTransformed = false;
}

bool CScriptPlatform::IsInMovedList(TUniqueId id, const TMovedList& moved) {
  const ushort index = id.Value();
  for (TMovedList::const_iterator it = moved.begin(); it != moved.end(); ++it) {
    if (index == *it)
      return true;
  }
  return false;
}

CHealthInfo* CScriptPlatform::HealthInfo() { return &mHealth; }

const CDamageVulnerability* CScriptPlatform::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CScriptPlatform::SetMotionTime(float time, CStateManager& mgr) {
  mMotionTime = time;
  if (mWaypointTracker.get()) {
    mWaypointTracker->SetTime(time);
  }
  mMotionForward = true;
  mPreviousMotionForward = true;
  mPassedMotionEnd = false;
  mPassedMotionStart = false;
  Stop();
  CVector3f position = GetTranslation();
  if (mSplineController.get() && mSplineController->GetPositionKnotCount() != 0) {
    position = mSplineController->GetPositionByTime(time);
  }
  SetTranslation(position);
  if (!mStaticSlaves.empty() || !mDynamicSlaves.empty()) {
    TMovedList moved;
    DragSlaves(mgr, moved);
  }
}

void CScriptPlatform::TeleportToWaypoint(TUniqueId id, CStateManager& mgr) {
  if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id)) && mWaypointTracker.get()) {
    const float time = mWaypointTracker->GetWaypointTime(id, mgr);
    if (time >= 0.f) {
      SetMotionTime(time, mgr);
    }
  }
}

void CScriptPlatform::TranslateMotion(const CVector3f& delta) {
  if (mSplineController.get()) {
    mSplineController->PositionSpline().Translate(delta);
  }
  SetTranslation(GetTranslation() + delta);
  mMotionTransformed = true;
}

void CScriptPlatform::RotateMotion(const CQuaternion& rotation, const CVector3f& pivot) {
  if (mSplineController.get()) {
    mSplineController->PositionSpline().Rotate(rotation, pivot);
  }
  SetTranslation(rotation.Transform(GetTranslation() - pivot) + pivot);
  mMotionTransformed = true;
}

void CScriptPlatform::fn_800a1df8() {
  x48d_25_ = true;
  if ((mMotionFlags & 8) != 0) {
    mMotionActive = true;
  } else {
    StopMotion();
  }
  mDead = false;
  mHealth = mInitialHealth;
}

void CScriptPlatform::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_AreaLoaded: {
    BuildSlaveList(mgr);
    for (int i = 0; i < mStaticSlaves.size(); ++i) {
      if (CScriptPlatform* platform =
              TCastToPtr< CScriptPlatform >(mgr.ObjectById(mStaticSlaves[i].mUid))) {
        platform->x452_ = GetUniqueId();
      }
    }
    const TUniqueId waypoint = FindConnectedObject(mgr, kSS_Connect, kSM_Attach);
    if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypoint))) {
      mSplineController = rs_new CSpline(
          mMotionSpline->GetDuration(), mMotionSpline->IsClosedLoop(), mMotionSpline->GetSpline(),
          CMayaSpline(), mMotionSpline->GetType(), mMotionSpline->GetType());
      ScriptCameraSpline::Initialise(*this, kSS_Connect, kSM_Attach, kSS_CameraTarget, kSM_Follow,
                                     mgr, *mSplineController);
      mWaypointTracker =
          rs_new CPlatformWaypointTracker(mSplineController->GetDuration(), GetUniqueId());
      mWaypointTracker->Build(waypoint, mSplineController->GetPositionSpline(),
                              mSplineController->PositionTimeSpline(), (mMotionFlags & 4) != 0,
                              mgr);
    }
    mMotionSpline = nullptr;
    mInitialTransform = GetTransform();
    fn_800a1df8();
    if ((mMotionFlags & 0x100) != 0) {
      AddMaterial(kMT_Solid, mgr);
      RemoveMaterial(kMT_ProjectilePassthrough, mgr);
    } else {
      RemoveMaterial(kMT_Solid, mgr);
      AddMaterial(kMT_ProjectilePassthrough, mgr);
    }
    for (int i = 0; i < GetConnectionList().size(); ++i) {
      const SConnection& connection = GetConnectionList()[i];
      if (connection.state == kSS_ScanSource && connection.msg == kSM_Attach) {
        AddMaterial(kMT_Scannable, mgr);
        break;
      }
    }
    mLookAtTarget = FindConnectedObject(mgr, kSS_Connect, kSM_InternalMessage0);
    break;
  }
  case EScriptObjectMessage('XONP'):
    AddRider(mRiders, sender, this, mgr, rstl::optional_object< float >(1.f / 6.f));
    break;
  case kSM_Stop:
    StopMotion();
    break;
  case kSM_Next: {
    TUniqueId waypoint = kInvalidUniqueId;
    float time = 0.f;
    if (mWaypointTracker.get()) {
      time = mWaypointTracker->FindNextWaypointTime(mMotionTime, waypoint);
    }
    if (time != mMotionTime) {
      SetMotionTime(time, mgr);
      if (CScriptWaypoint* actor = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(waypoint))) {
        mgr.SendScriptMsg(actor, GetUniqueId(), EScriptObjectMessage('ARRV'), kInvalidUniqueId);
      }
    }
    break;
  }
  case kSM_Start:
    mMotionActive = true;
    break;
  case kSM_Reset:
    fn_800a1df8();
    break;
  case kSM_Increment:
    if (!GetActive()) {
      mgr.SendScriptMsg(this, GetUniqueId(), kSM_Activate, kInvalidUniqueId);
    }
    CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), mFadeInTime);
    break;
  case kSM_Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), mFadeOutTime);
    break;
  case kSM_Delete:
    DecayRiders(mRiders, 1.6666667f, mgr);
    break;
  case kSM_Deactivate:
    for (int i = 0; i < mStaticSlaves.size(); ++i) {
      if (CScriptPlatform* platform =
              TCastToPtr< CScriptPlatform >(mgr.ObjectById(mStaticSlaves[i].mUid))) {
        platform->x452_ = kInvalidUniqueId;
      }
    }
    break;
  case kSM_Kill:
    if (!GetActive()) {
      HealthInfo()->SetHP(0.f);
      mDead = true;
      SendScriptMsgs(kSS_Dead, mgr, GetUniqueId(), kSM_None);
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

const CCollisionPrimitive* CScriptPlatform::GetCollisionPrimitive() const {
  return !mTreeGroup.get() ? CPhysicsActor::GetCollisionPrimitive() : mTreeGroup.get();
}

CTransform4f CScriptPlatform::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.AddTranslation(GetPrimitiveOffset());
  return xf;
}

void CScriptPlatform::SplashThink(const CAABox& bounds, const CFluidPlane& fluid, float dt,
                                  CStateManager& mgr) const {}

void CScriptPlatform::AddRider(TUniqueId id, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  AddRider(mRiders, id, this, mgr, rstl::optional_object< float >(decayTimer));
}

void CScriptPlatform::AddSlave(TUniqueId id, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  rstl::vector< SRiders >::iterator it =
      rstl::find(mDynamicSlaves.begin(), mDynamicSlaves.end(),
                 SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >()));
  if (it == mDynamicSlaves.end()) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      actor->AddMaterial(kMT_PlatformSlave, mgr);
      const CTransform4f xf = GetTransform().GetQuickInverse() * actor->GetTransform();
      mDynamicSlaves.reserve(mDynamicSlaves.size() + 1);
      mDynamicSlaves.push_back_unsafe(SRiders(id, xf, rstl::optional_object< float >(decayTimer)));
    }
  } else {
    SRiders& rider = *it;
    rider.mDecayTimer = decayTimer;
  }
}

void CScriptPlatform::UpdateSlaveTransforms(CStateManager& mgr) {
  const CTransform4f inverse = GetTransform().GetQuickInverse();
  for (rstl::vector< SRiders >::iterator it = mDynamicSlaves.begin(); it != mDynamicSlaves.end();
       ++it) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(it->mUid))) {
      const CTransform4f xf = inverse * actor->GetTransform();
      it->mTransform = xf;
      if (CScriptPlatform* platform = TCastToPtr< CScriptPlatform >(actor)) {
        platform->UpdateSlaveTransforms(mgr);
      }
    }
  }
}

bool CScriptPlatform::IsRider(TUniqueId id) const {
  return rstl::find(mRiders.begin(), mRiders.end(),
                    SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >())) !=
         mRiders.end();
}

bool CScriptPlatform::RemoveRider(TUniqueId id) {
  rstl::vector< SRiders >::iterator it =
      rstl::find(mRiders.begin(), mRiders.end(),
                 SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >()));
  if (it == mRiders.end()) {
    return false;
  }
  mRiders.erase(it);
  return true;
}

SRiders::SRiders(TUniqueId uid, const CTransform4f& xf,
                 const rstl::optional_object< float >& decayTimer)
: mUid(uid), mDecayTimer(decayTimer), mTransform(xf) {}

bool CScriptPlatform::IsSlave(TUniqueId id) const {
  return rstl::find(mStaticSlaves.begin(), mStaticSlaves.end(),
                    SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >())) !=
             mStaticSlaves.end() ||
         rstl::find(mDynamicSlaves.begin(), mDynamicSlaves.end(),
                    SRiders(id, CTransform4f::Identity(), rstl::optional_object< float >())) !=
             mDynamicSlaves.end();
}

CQuaternion CScriptPlatform::Move(float dt, CStateManager& mgr) {
  if (mSplineController.get() && mSplineController->GetPositionSpline().GetKnotCount() != 0) {
    const CVector3f position = mSplineController->GetPositionByTime(mMotionTime);
    mDragDelta = position - GetTranslation();
    MoveToWR(position, dt);
    TNearList nearList;
    mgr.BuildColliderList(nearList, *this, GetMotionVolume(dt));
    TNearList filtered;
    for (int i = 0; i < nearList.size(); ++i) {
      if (!IsRider(nearList[i]) && !IsSlave(nearList[i])) {
        filtered.push_back(nearList[i]);
      }
    }
    const CMotionState motion = PredictMotion(dt);
    MoveCollisionPrimitive(motion.GetTranslation());
    const bool collision = CGameCollision::DetectDynamicCollisionBoolean(
        *GetCollisionPrimitive(), GetPrimitiveTransform(), filtered, mgr);
    MoveCollisionPrimitive(CVector3f::Zero());
    if (collision || mSquishedRider) {
      if ((mMotionFlags & 2) != 0) {
        if (mCollisionRecoverDelay <= 0.f && !mSquishedRider) {
          mCollisionRecoverDelay = 0.035f;
        } else {
          mMotionForward = !mMotionForward;
          AdvanceMotionTime(dt);
          mSquishedRider = false;
          if (mWaypointTracker.get()) {
            mWaypointTracker->SendArrivals(mMotionTime, mPassedMotionEnd, mPassedMotionStart,
                                           mMotionForward, mSplineController->PositionTimeSpline(),
                                           mgr);
          }
        }
      }
    } else {
      AdvanceMotionTime(dt);
      AddMotionState(motion);
      if (mWaypointTracker.get()) {
        mWaypointTracker->SendArrivals(mMotionTime, mPassedMotionEnd, mPassedMotionStart,
                                       mMotionForward, mSplineController->PositionTimeSpline(),
                                       mgr);
      }
    }
    if ((mMotionFlags & (0x80 | 0x1000)) != 0) {
      const float time = mSplineController->GetDuration() *
                         mSplineController->PositionTimeSpline().EvaluateAt(mMotionTime);
      CVector3f tangent = mSplineController->GetPositionSpline().GetTangentByTime(time);
      if ((mMotionFlags & 0x1000) != 0) {
        tangent.SetZ(0.f);
      }
      if (tangent.CanBeNormalized()) {
        tangent.Normalize();
        CTransform4f xf = CTransform4f::Identity();
        xf.SetTranslation(GetTranslation());
        xf.SetColumn(kDY, tangent);
        CVector3f flat(tangent.GetX(), tangent.GetY(), 0.f);
        if (flat.CanBeNormalized()) {
          flat.Normalize();
          if (CVector3f::Dot(tangent, flat) < 0.99999f) {
            xf.SetColumn(kDZ, CQuaternion::LookAt(CUnitVector3f(flat), CUnitVector3f(tangent),
                                                  CRelAngle::FromRadians(2.f * M_PIF))
                                  .Transform(CVector3f::Up()));
          }
          xf.SetColumn(kDX, CVector3f::Cross(tangent, xf.GetUp()));
          if (!mRollSpline.null()) {
            const CQuaternion roll = CQuaternion::AxisAngle(
                CUnitVector3f(tangent),
                CRelAngle::FromDegrees(mRollSpline->EvaluateAt(mMotionTime)));
            xf.SetColumn(kDZ, roll.Transform(xf.GetUp()));
            xf.SetColumn(kDX, roll.Transform(xf.GetRight()));
          }
          SetTransform(xf);
          SetTransformExplicitly(xf);
        }
      }
    }
  } else {
    AdvanceMotionTime(dt);
  }
  if ((mMotionFlags & 0x200) != 0) {
    const CRelAngle pitch =
        CRelAngle::FromDegrees(mPitchSpline.null() ? 0.f : mPitchSpline->EvaluateAt(mMotionTime));
    const CRelAngle yaw =
        CRelAngle::FromDegrees(mYawSpline.null() ? 0.f : mYawSpline->EvaluateAt(mMotionTime));
    const CRelAngle roll =
        CRelAngle::FromDegrees(mRollSpline.null() ? 0.f : mRollSpline->EvaluateAt(mMotionTime));
    CTransform4f xf = mInitialTransform.GetRotation() * CTransform4f::RotateZ(yaw) *
                      CTransform4f::RotateY(roll) * CTransform4f::RotateX(pitch);
    xf.SetTranslation(GetTranslation());
    SetTransform(xf);
    SetTransformExplicitly(xf);
  }
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mLookAtTarget))) {
    if ((actor->GetOrbitPosition(mgr) - GetTranslation()).CanBeNormalized()) {
      CTransform4f xf =
          CTransform4f::LookAt(GetTranslation(), actor->GetOrbitPosition(mgr), CVector3f::Up());
      xf.SetTranslation(GetTranslation());
      SetTransform(xf);
    }
  }
  return CalculateRotationDelta();
}

void CScriptPlatform::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (mgr.GetObjectById(mBoundsTrigger) == nullptr) {
    mBoundsTrigger = kInvalidUniqueId;
  }
}

CVector3f CScriptPlatform::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CScriptPlatform::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (GetTouchBounds()) {
    return GetTouchBounds()->GetCenterPoint();
  }
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

CAABox CScriptPlatform::GetSortingBounds(const CStateManager& mgr) const {
  if (mBoundsTrigger != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(mBoundsTrigger))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return CActor::GetSortingBounds(mgr);
}

void CScriptPlatform::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptPlatform::SetTransformExplicitly(const CTransform4f& xf) { mCurrentRotation = xf; }

CQuaternion CScriptPlatform::CalculateRotationDelta() {
  CTransform4f delta = mCurrentRotation * mPreviousRotation.GetQuickInverse();
  mPreviousRotation = mCurrentRotation;
  return CQuaternion::FromMatrix(delta);
}

void CScriptPlatform::ResetMotion(float time, CStateManager& mgr) {
  CTransform4f xf = mInitialTransform;
  xf.SetTranslation(GetTranslation());
  CActor::SetTransform(xf);
  mPreviousRotation = xf.GetRotation();
  mPreviousRotation.Orthonormalize();
  mCurrentRotation = mPreviousRotation;
  if (mSplineController.get()) {
    const float duration = mSplineController->PositionTimeSpline().GetDuration();
    SetMotionTime(CMath::Clamp(0.f, time, duration), mgr);
  }
}

void CScriptPlatform::SetTransformIfNoPositionSpline(const CTransform4f& xf) {
  if (!mSplineController.get() ||
      (mSplineController.get() && mSplineController->GetPositionKnotCount() == 0)) {
    CActor::SetTransform(xf);
    mMotionTransformed = true;
  }
}

CEntity* LoadPlatform(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlatform sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlatform.inc"
  const rstl::optional_object< CModelData > model =
      LdrToModelData(sldrThis.editorProperties.transform.scale, sldrThis.model,
                     sldrThis.animationInformation, true);
  if (!model.valid()) {
    return nullptr;
  }

  CAABox bounds =
      LoadCAABox(mgr, info.GetAreaId(), sldrThis.collisionBox, sldrThis.collisionOffset);
  if (sldrThis.collisionBox == CVector3f::Zero()) {
    bounds = model->GetBounds(LdrToTransform4f(sldrThis.editorProperties).GetRotation());
  }

  rstl::optional_object< TLockedToken< const COBBTreeGroup > > dcln;
  if (gpResourceFactory->GetResourceTypeById(sldrThis.collisionModel) != 0) {
    dcln = TLockedToken< const COBBTreeGroup >(
        gpSimplePool->GetObj(SObjectTag('DCLN', sldrThis.collisionModel)));
  }

  const SLdrPlatformMotionProperties& motion = sldrThis.motionProperties;
  float duration = motion.motionSplineDuration;
  if (CMath::IsEpsilon(duration, 0.f, 0.00001f)) {
    duration = motion.motionControlSpline.GetDuration();
  }
  const CGameSplineDesc spline(
      motion.motionControlSpline,
      static_cast< CMotionSpline::ESplineType >(motion.motionSplineType.type), duration,
      (motion.motionFlagsPlatformMotion & 1) != 0);
  CMaterialList materials(kMT_Solid, kMT_Immovable, kMT_Platform, kMT_Occluder);
  if (sldrThis.excludeFromLineOfSightTest) {
    materials.Add(kMT_ExcludeFromLineOfSightTest);
  }

  return rs_new CScriptPlatform(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *model, LdrToActorParameters(sldrThis.actorInformation), bounds, dcln,
      LdrToHealthInfo(sldrThis.health), LdrToDamageVulnerability(sldrThis.vulnerability), materials,
      sldrThis.renderRainSplashes, sldrThis.maximumSplashes, sldrThis.splashGenerationRate, spline,
      motion.motionFlagsPlatformMotion, sldrThis.conveyorBeltVelocity, motion.rollControlSpline,
      motion.yawControlSpline, motion.pitchControlSpline, motion.initialTime,
      sldrThis.randomAnimationOffset);
}
