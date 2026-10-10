#include "MetroidPrime/Enemies/CWallCrawler.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CWallCrawler::CWallCrawler(EPatternedAI character, TUniqueId uid, const rstl::string& name,
                           EFlavorType flavor, CEntityInfo& info, const CTransform4f& xf,
                           const CModelData& mData, const CPatternedInfo& pInfo,
                           EMovementType moveType, EColliderType colType, EBodyType bodyType,
                           const CActorParameters& actParms, float sphereRadius,
                           float collisionCloseMargin, float alignAngVel, float advanceWpRadius,
                           float playerObstructionMinDist, EType type, bool disableMove, float f1,
                           float f2, float f3, float f4, float f5, float f6)
: CPatterned(character, uid, name, flavor, info, xf, mData, pInfo, moveType, colType, bodyType,
             actParms)
, mAlignSurface(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), ~static_cast< u64 >(0))
, mColSphere(CSphere(CVector3f::Zero(), sphereRadius), GetMaterialList())
, mCollisionCloseMargin(collisionCloseMargin)
, mAlignAngVel(alignAngVel)
, mTumbleAngle(0.f)
, mPatrolPauseRemTime(0.f)
, mAdvanceWpRadius(advanceWpRadius)
, mPlayerObstructionMinDist(playerObstructionMinDist)
, mBendingHackWeight(0.f)
, mType(type)
, mThinkCounter(0)
, mConstraintType(0)
, mConstraintPlane(0.f, CUnitVector3f(CVector3f(0.f, 1.f, 0.f).Normalize(), CUnitVector3f::kN_No))
, mAlignToFloor(false)
, mHasAlignSurface(false)
, mPlayerObstructed(false)
, mDisableMove(disableMove)
, mTouchBoundsScale(f1)
, mFloorAlignRate(f2)
, x850_(f3)
, x854_(f4)
, x858_(f5)
, x85c_(f6)
, x860_27_(false)
, x860_28_(false)
, x860_29_(false)
, x860_30_(false)
, x861_24_(true) {}

CWallCrawler::~CWallCrawler() {}

void CWallCrawler::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

rstl::optional_object< CAABox > CWallCrawler::GetTouchBounds() const {
  return GetBaseBoundingBox().GetTransformedAABox(GetPrimitiveTransform() *
                                                  CTransform4f::Scale(mTouchBoundsScale));
}

const CCollisionPrimitive* CWallCrawler::GetCollisionPrimitive() const { return &mColSphere; }

const CPlane& CWallCrawler::GetConstraintPlane() const { return mConstraintPlane; }

int CWallCrawler::GetConstraintType() const { return mConstraintType; }

void CWallCrawler::SetConstraint(CStateManager& mgr, int constraint) {
  mConstraintType = constraint;
  if (constraint != 0) {
    UpdateConstraintPlane(mgr);
  }
}

CVector3f CWallCrawler::ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir) {
  return vec - CVector3f::Dot(vec, planeDir) * planeDir;
}

CVector3f CWallCrawler::GetClosestPointOnPlane(const CVector3f& point, const CPlane& plane) {
  const CVector3f normal(plane.GetNormal());
  const float dist = CVector3f::Dot(normal, point) - plane.GetConstant();
  return point - dist * normal;
}

void CWallCrawler::AlignToPlane(const CUnitVector3f& normal, float dt) {
  const float dot = CVector3f::Dot(GetTransform().GetUp(), normal);
  if (close_enough(dot, 1.f)) {
    return;
  }
  if (dot < -0.999f) {
    return;
  }
  const CQuaternion rotation = CQuaternion::ShortestRotationArcClamped(
      GetTransform().GetUp(), normal, CRelAngle::FromDegrees(dt));
  const CQuaternion localRotation(rotation.GetScalar(),
                                  GetTransform().TransposeRotate(rotation.GetVector()));
  SetRotation((CQuaternion::FromMatrix(GetTransform()) * localRotation).BuildNormalized());
}

void CWallCrawler::UpdateConstraintPlane(CStateManager& mgr) {
  const TUniqueId waypointId =
      mDestObj == kInvalidUniqueId ? GetConnectedObject(mgr, kSS_Patrol, kSM_Follow) : mDestObj;
  CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(waypointId));
  CVector3f points[3];
  int numPoints = 0;
  for (int i = 0; numPoints < 3 && i < 30; ++i) {
    if (waypoint == nullptr) {
      break;
    }
    const CVector3f position = waypoint->GetTranslation();
    bool skipPoint = false;
    if (numPoints == 2) {
      const CVector3f n1 = (points[0] - points[1]).AsNormalized();
      const CVector3f n2 = (position - points[1]).AsNormalized();
      skipPoint = CMath::AbsF(CVector3f::Dot(n1, n2)) > 0.984f;
    }
    if (!skipPoint) {
      points[numPoints++] = position;
    }
    const TUniqueId nextId = GetNextWaypoint(mgr, waypoint, false);
    waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(nextId));
  }
  if (numPoints == 3) {
    mConstraintPlane = CPlane(points[0], points[1], points[2]);
  } else if (numPoints == 2) {
    if (CMath::AbsF(points[0].GetX() - points[1].GetX()) < 1e-5f ||
        CMath::AbsF(points[0].GetY() - points[1].GetY()) < 1e-5f) {
      return;
    }
    const CVector3f up(points[1].GetX(), points[1].GetY(), points[1].GetZ() + 1.f);
    mConstraintPlane = CPlane(points[0], points[1], up);
  }
}

void CWallCrawler::PreThink(float dt, CStateManager& mgr) {
  CPatterned::PreThink(dt, mgr);
  if (GetActive() && !mPlayerObstructed && mPatrolPauseRemTime <= 0.f && !mDisableMove &&
      close_enough(mBodyController->GetPercentageFrozen(), 0.f)) {
    if (mAlignToFloor) {
      const CQuaternion oldOrientation = CQuaternion::FromMatrix(GetTransform());
      const CMotionState motion = PredictMotion(dt);
      AddMotionState(motion);
      const CQuaternion newOrientation = CQuaternion::FromMatrix(GetTransform());
      ClearForcesAndTorques();
      if (mHasAlignSurface) {
        const CPlane plane = mAlignSurface.GetPlane();
        const CVector3f position = GetTranslation();
        const CVector3f projected = position - (plane.GetHeight(GetTranslation()) -
                                                mColSphere.GetSphere().GetRadius() - 0.01f) *
                                                   plane.GetNormal();
        SetTranslation(CVector3f::Lerp(position, projected, 60.f * mFloorAlignRate * dt));
      }
      MoveCollisionPrimitive(CVector3f::Zero());
    }
  } else if (mPatrolPauseRemTime > 0.f) {
    Stop();
  }
}

TUniqueId CWallCrawler::GetNextWaypoint(CStateManager& mgr, const CScriptWaypoint* waypoint,
                                        bool reverse) {
  return waypoint->NextWaypoint(mgr);
}

void CWallCrawler::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  if (message >= kSM_AreaLoaded) {
    if (message == kSM_Create) {
      AddMaterial(kMT_Immovable, mgr);
      RemoveMaterial(kMT_Solid, mgr);
      AddMaterial(kMT_NonSolidDamageable, mgr);
    }
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

bool CWallCrawler::UpdateWPDestination(CStateManager& mgr) {
  bool arrived = false;
  if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mDestObj))) {
    const CVector3f position = waypoint->GetTranslation();
    const CVector3f delta = position - GetTranslation();
    if (delta.MagSquared() < mAdvanceWpRadius * mAdvanceWpRadius) {
      mDestObj = GetNextWaypoint(mgr, waypoint, true);
      arrived = true;
      if (CScriptAIWaypoint* aiWaypoint = TCastToPtr< CScriptAIWaypoint >(waypoint)) {
        if (!close_enough(aiWaypoint->GetPause(), 0.f)) {
          mPatrolPauseRemTime = aiWaypoint->GetPause();
          if (mType == kT_Parasite) {
            mBodyController->SetLocomotionType(pas::kLT_Relaxed);
          }
        }
      }
      mgr.SendScriptMsg(waypoint, GetUniqueId(), kSM_Arrived);
    }
    SetDestPos(position);
  }
  return arrived;
}
