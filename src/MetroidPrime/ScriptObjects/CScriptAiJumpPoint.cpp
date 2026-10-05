#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

CScriptAiJumpPoint::CScriptAiJumpPoint(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CTransform4f& xf,
                                       float jumpApex, int type)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mJumpApex(jumpApex)
, mType(type)
, mTouchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, mInUse(false)
, mOccupant(kInvalidUniqueId)
, mCurrentWaypoint(kInvalidUniqueId)
, mNextWaypoint(kInvalidUniqueId)
, mTimeRemaining(0.f) {}

void CScriptAiJumpPoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_AreaLoaded:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->state != kSS_Arrived || conn->msg != kSM_Next) {
        continue;
      }

      const TUniqueId id = mgr.GetIdForScript(conn->objId);
      if (const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
        mCurrentWaypoint = id;
        mNextWaypoint = waypoint->NextWaypoint(mgr);
        return;
      }
    }
    break;
  default:
    break;
  }
}

bool CScriptAiJumpPoint::GetInUse(TUniqueId uid) const {
  return mInUse || mTimeRemaining > 0.f ||
         (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant);
}

void CScriptAiJumpPoint::Think(float dt, CStateManager&) {
  if (mTimeRemaining > 0.f) {
    mTimeRemaining -= dt;
  }
}

void CScriptAiJumpPoint::AddToRenderer(const CStateManager&) const {}

void CScriptAiJumpPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptAiJumpPoint::GetTouchBounds() const { return mTouchBounds; }
