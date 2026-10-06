#include "MetroidPrime/CLineOfSightTracker.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"

CLineOfSightTracker::CLineOfSightTracker(TUniqueId owner, CSegId segment,
                                         float minimumCheckInterval, float checkIntervalRange)
: mOwner(owner)
, mSegment(segment)
, mRayFilter(CMaterialList(kMT_Unknown59),
             CMaterialList(kMT_Character, kMT_Player, kMT_CollisionActor, kMT_NoPlatformCollision,
                           kMT_ExcludeFromLineOfSightTest),
             CMaterialFilter::kFT_IncludeExclude)
, mTarget(kInvalidUniqueId)
, mMinimumCheckInterval(minimumCheckInterval)
, mCheckIntervalRange(checkIntervalRange)
, mTimeUntilNextCheck(0.f)
, mClearTime(0.f)
, mBlockedTime(0.f)
, mHasLineOfSight(false) {}

void CLineOfSightTracker::SetTarget(TUniqueId target) {
  mTarget = target;
  mHasLineOfSight = false;
  mTimeUntilNextCheck = 0.f;
  mClearTime = 0.f;
  mBlockedTime = 0.f;
}

void CLineOfSightTracker::Update(float dt, CStateManager& mgr) {
  if (mTimeUntilNextCheck > 0.f) {
    mTimeUntilNextCheck -= dt;
  }

  if (mTimeUntilNextCheck <= 0.f) {
    mHasLineOfSight = false;
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTarget));
    if (target) {
      const CActor* owner = static_cast< const CActor* >(mgr.GetObjectById(mOwner));
      if (owner) {
        const CVector3f end = target->GetAimPosition(mgr, 0.f);
        CVector3f start = owner->GetAimPosition(mgr, 0.f);
        if (mSegment != CSegId::Null()) {
          const CTransform4f locator =
              owner->GetModelData()->GetAnimationData()->GetLocatorTransform(mSegment, nullptr);
          const CTransform4f worldLocator =
              owner->GetTransform() *
              CTransform4f(locator.BuildMatrix3f(),
                           CVector3f::ByElementMultiply(owner->GetModelData()->GetScale(),
                                                        locator.GetTranslation()));
          start = worldLocator.GetTranslation();
        }

        mHasLineOfSight = mgr.RayCollideWorld(start, end, mRayFilter, owner);
        mTimeUntilNextCheck = mMinimumCheckInterval + mCheckIntervalRange * mgr.Random()->Float();
      }
    }
  }

  if (mHasLineOfSight) {
    mClearTime += dt;
    mBlockedTime = 0.f;
  } else {
    mBlockedTime += dt;
    mClearTime = 0.f;
  }
}
