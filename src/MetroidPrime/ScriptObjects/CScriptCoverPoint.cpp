#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CStateManager.hpp"

CScriptCoverPoint::CScriptCoverPoint(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf, uint flags,
                                     const bool crouch, float horizontalSafeAngle,
                                     float verticalSafeAngle, float lockTime)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mFlags(flags)
, mHorizontalSafeHalfAngle(CRelAngle::FromDegrees(0.5f * horizontalSafeAngle).AsRadians())
, mVerticalSafeHalfAngle(CRelAngle::FromDegrees(0.5f * verticalSafeAngle).AsRadians())
, mLockTime(lockTime)
, mCrouch(crouch)
, mInUse(false)
, mOccupant(kInvalidUniqueId)
, mRetreatPoint(kInvalidUniqueId)
, mTouchBounds(CAABox(xf.GetTranslation(), xf.GetTranslation()))
, mTimeRemaining(0.f) {}

void CScriptCoverPoint::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);

  switch (message) {
  case kSM_AreaLoaded:
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Retreat) {
        mRetreatPoint = mgr.GetIdForScript(it->objId);
        break;
      }
    }
    break;
  default:
    break;
  }
}

pas::ECoverDirection CScriptCoverPoint::GetAttackDirection() const {
  return static_cast< pas::ECoverDirection >(mFlags);
}

bool CScriptCoverPoint::ShouldCrouch() const { return mCrouch; }

bool CScriptCoverPoint::ShouldStay() const { return (mFlags >> 3) & 1; }

bool CScriptCoverPoint::ShouldWallHang() const { return (mFlags >> 4) & 1; }

bool CScriptCoverPoint::ShouldLandHere() const { return (mFlags >> 5) & 1; }

bool CScriptCoverPoint::Blown(const CVector3f& point) const {
  if (GetActive()) {
    if (ShouldWallHang()) {
      return false;
    }

    const CVector3f delta = point - GetTranslation();
    const CVector3f forward = GetTransform().GetForward();
    if (CVector3f::GetAngleDiff(forward, delta.DropZ()) <= mHorizontalSafeHalfAngle &&
        CVector3f::GetAngleDiff(delta, CVector3f(delta.GetX(), delta.GetY(), 0.f)) <=
            mVerticalSafeHalfAngle) {
      return false;
    }
  }
  return true;
}

bool CScriptCoverPoint::GetInUse(TUniqueId uid) const {
  return mInUse || mTimeRemaining > 0.f ||
         (mOccupant != kInvalidUniqueId && uid != kInvalidUniqueId && uid != mOccupant);
}

void CScriptCoverPoint::SetInUse(bool inUse) {
  mInUse = inUse;
  if (!mInUse) {
    mTimeRemaining = mLockTime;
  }
}

void CScriptCoverPoint::Think(float dt, CStateManager&) {
  if (mTimeRemaining > 0.f) {
    mTimeRemaining -= dt;
  }
}

void CScriptCoverPoint::AddToRenderer(const CStateManager&) const {}

void CScriptCoverPoint::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptCoverPoint::GetTouchBounds() const { return mTouchBounds; }
