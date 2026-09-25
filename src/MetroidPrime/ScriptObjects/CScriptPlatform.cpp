#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptPlatform::CScriptPlatform(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& model, const CActorParameters& params, const CAABox& bounds,
    const rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > >& dcln,
    const CHealthInfo& health, const CDamageVulnerability& vulnerability,
    const CMaterialList& materials, bool detectCollision, uint maxRainSplashes, uint rainGenRate,
    const SPlatformMotionSpline& motionSpline, uint motionFlags, const CVector3f& conveyorVelocity,
    const CMayaSpline& rollSpline, const CMayaSpline& yawSpline, const CMayaSpline& pitchSpline,
    float initialTime, float xrayAlpha)
: CPhysicsActor(uid, name, info, 0, xf, model, materials, bounds, SMoverData(15000.f), params,
                StepData(0.f, 0.f, 0))
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
, mMotionSpline(rs_new SPlatformMotionSpline(motionSpline))
, mSplineController(nullptr)
, mMotionTime(0.f)
, mMotionFlags(motionFlags)
, mInitialTime(initialTime)
, mMotionDuration(motionSpline.mDuration)
, mWaypointTracker(nullptr)
, mRollSpline(rollSpline.GetKnotCount() ? rs_new CMayaSpline(rollSpline) : nullptr)
, mYawSpline(yawSpline.GetKnotCount() ? rs_new CMayaSpline(yawSpline) : nullptr)
, mPitchSpline(pitchSpline.GetKnotCount() ? rs_new CMayaSpline(pitchSpline) : nullptr)
, x450_(kInvalidUniqueId)
, x452_(kInvalidUniqueId)
, mLookAtTarget(kInvalidUniqueId)
, mXrayAlpha(xrayAlpha)
, mInitialTransform(xf)
, mDead(false)
, mControlledAnimation(false)
, mDetectCollision(detectCollision)
, mSquishedRider(false)
, mMotionActive(false)
, mPassedMotionEnd(false)
, mPassedMotionStart(false)
, mMotionForward(true)
, mPreviousMotionForward(true)
, x48d_25_(false)
, mMotionTransformed(false) {
  SetMovable(false);
  // TODO: original StepData initialization, material filter, animation setup and DCLN allocation.
}

CScriptPlatform::~CScriptPlatform() {
  // TODO: delete the spline controller and waypoint tracker once their interfaces are recovered.
}

rstl::optional_object< CAABox > CScriptPlatform::GetTouchBounds() const {
  if (!GetActive())
    return rstl::optional_object< CAABox >();
  // TODO: transform the complex collision bounds when mTreeGroup is present.
  return GetBoundingBox();
}

void CScriptPlatform::StopMotion() {
  mMotionActive = false;
  Stop();
  mPreviousRotation = GetTransform().GetRotation();
  mCurrentRotation = mPreviousRotation;
  mDragDelta = CVector3f::Zero();
  mRotationDelta = CQuaternion::NoRotation();
}

void CScriptPlatform::fn_800a3d18() { StopMotion(); }

void CScriptPlatform::AdvanceMotionTime(float dt) {
  // TODO: forward/reverse, loop/clamp and endpoint events.
}

void CScriptPlatform::AddRider(rstl::vector< SRiders >& riders, TUniqueId id,
                               const CPhysicsActor* ridee, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  // TODO: attach/update the rider transform and timer, then send XONP.
}

CScriptPlatform::TNearList
CScriptPlatform::BuildNearListFromRiders(CStateManager& mgr,
                                         const rstl::vector< SRiders >& riders) {
  TNearList result;
  for (int i = 0; i < riders.size(); ++i) {
    if (CActor* actor = TCastToPtr< CActor >(mgr.GetObjectByIdFromListAll(riders[i].mUid))) {
      result.push_back(actor->GetUniqueId());
    }
  }
  return result;
}

void CScriptPlatform::DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr) {
  // TODO: decrement optional timers, erase expired riders and send XONP.
}

void CScriptPlatform::MoveRiders(CStateManager& mgr, bool active, rstl::vector< SRiders >& riders,
                                 rstl::vector< SRiders >& collidedRiders, const TNearList& nearList,
                                 const CTransform4f& oldXf, const CTransform4f& newXf,
                                 const CVector3f& dragDelta, const CQuaternion& rotDelta) {
  // TODO: collision-tested rider displacement and rotation.
}

void CScriptPlatform::PreThink(float dt, CStateManager& mgr) {
  // TODO: platform motion, collision filtering and rider movement.
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr) {
  // TODO: resolve PLAY/ACTV slaves and IBND/ACTV bounds triggers.
}

void CScriptPlatform::DragSlave(CStateManager& mgr, TMovedList& moved, const SRiders& slave) {
  // TODO: apply the motion flags and recursively move platform slaves.
}

void CScriptPlatform::DragSlaves(CStateManager& mgr, TMovedList& moved) {
  // TODO: propagate motion to static/dynamic slaves and special actor types.
}

void CScriptPlatform::Think(float dt, CStateManager& mgr) {
  // TODO: animation, fade/death and rider decay.
}

bool CScriptPlatform::IsInMovedList(TUniqueId id, const TMovedList& moved) {
  for (int i = 0; i < moved.size(); ++i) {
    if (moved[i] == id.Value())
      return true;
  }
  return false;
}

CHealthInfo* CScriptPlatform::HealthInfo(CStateManager& mgr) { return &mHealth; }

const CDamageVulnerability* CScriptPlatform::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

void CScriptPlatform::SetMotionTime(float time, CStateManager& mgr) {
  mMotionTime = time;
  // TODO: reset motion flags, evaluate the controller and move slaves.
}

void CScriptPlatform::TeleportToWaypoint(TUniqueId id, CStateManager& mgr) {
  // TODO: obtain the connected waypoint time, then update motion.
}

void CScriptPlatform::TranslateMotion(const CVector3f& delta) {
  // TODO: translate the spline controller as well.
  SetTranslation(GetTranslation() + delta);
  mMotionTransformed = true;
}

void CScriptPlatform::RotateMotion(const CQuaternion& rotation, const CVector3f& pivot) {
  // TODO: rotate the spline controller as well.
  SetTranslation(rotation.Transform(GetTranslation() - pivot) + pivot);
  mMotionTransformed = true;
}

void CScriptPlatform::fn_800a1df8() {
  // TODO: restore the motion activation flags.
  mDead = false;
  mHealth = mInitialHealth;
}

void CScriptPlatform::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  // TODO: motion setup/control, riders, bounds trigger and health reset.
  CPhysicsActor::AcceptScriptMsg(mgr, msg);
}

const CCollisionPrimitive* CScriptPlatform::GetCollisionPrimitive() const {
  return mTreeGroup.get() ? mTreeGroup.get() : CPhysicsActor::GetCollisionPrimitive();
}

CTransform4f CScriptPlatform::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.SetTranslation(xf.GetTranslation() + GetPrimitiveOffset());
  return xf;
}

void CScriptPlatform::SplashThink(const CAABox& bounds, const CFluidPlane& fluid, float dt,
                                  CStateManager& mgr) const {}

void CScriptPlatform::AddRider(TUniqueId id, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  AddRider(mRiders, id, this, mgr, decayTimer);
}

void CScriptPlatform::AddSlave(TUniqueId id, CStateManager& mgr,
                               const rstl::optional_object< float >& decayTimer) {
  // TODO: attach/update a dynamic slave and its optional decay timer.
}

void CScriptPlatform::UpdateSlaveTransforms(CStateManager& mgr) {
  // TODO: recompute dynamic slave transforms recursively.
}

bool CScriptPlatform::IsRider(TUniqueId id) const {
  for (int i = 0; i < mRiders.size(); ++i) {
    if (mRiders[i].mUid == id)
      return true;
  }
  return false;
}

bool CScriptPlatform::RemoveRider(TUniqueId id) {
  for (rstl::vector< SRiders >::iterator it = mRiders.begin(); it != mRiders.end(); ++it) {
    if (it->mUid == id) {
      mRiders.erase(it);
      return true;
    }
  }
  return false;
}

SRiders::SRiders(TUniqueId uid, const CTransform4f& xf,
                 const rstl::optional_object< float >& decayTimer)
: mUid(uid), mDecayTimer(decayTimer), mTransform(xf) {}

bool CScriptPlatform::IsSlave(TUniqueId id) const {
  for (int i = 0; i < mStaticSlaves.size(); ++i) {
    if (mStaticSlaves[i].mUid == id)
      return true;
  }
  for (int i = 0; i < mDynamicSlaves.size(); ++i) {
    if (mDynamicSlaves[i].mUid == id)
      return true;
  }
  return false;
}

CQuaternion CScriptPlatform::Move(float dt, CStateManager& mgr) {
  // TODO: spline position/orientation, collision recovery and look-at target.
  return CQuaternion::NoRotation();
}

void CScriptPlatform::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  if (mgr.GetObjectById(mBoundsTrigger) == nullptr) {
    mBoundsTrigger = kInvalidUniqueId;
  }
}

CVector3f CScriptPlatform::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CScriptPlatform::GetAimPosition(const CStateManager& mgr, float dt) const {
  // TODO: prefer the center of the touch bounds.
  return CPhysicsActor::GetAimPosition(mgr, dt);
}

CAABox CScriptPlatform::GetSortingBounds(const CStateManager& mgr) const {
  // TODO: use the connected bounds trigger when available.
  return CActor::GetSortingBounds(mgr);
}

void CScriptPlatform::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

void CScriptPlatform::SetTransformExplicitly(const CTransform4f& xf) { mCurrentRotation = xf; }

CQuaternion CScriptPlatform::CalculateRotationDelta() {
  CTransform4f delta = mCurrentRotation * mPreviousRotation.GetQuickInverse();
  mPreviousRotation = mCurrentRotation;
  return CQuaternion::FromMatrix(delta);
}

void CScriptPlatform::fn_800a0200(float time, CStateManager& mgr) {
  // TODO: restore initial orientation and clamp the spline time.
}
