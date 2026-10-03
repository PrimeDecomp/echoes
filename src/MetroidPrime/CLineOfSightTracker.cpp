#include "MetroidPrime/CLineOfSightTracker.hpp"

CLineOfSightTracker::CLineOfSightTracker(TUniqueId owner, CSegId segment,
                                         float minimumCheckInterval, float checkIntervalRange)
: mOwner(owner), mTarget(kInvalidUniqueId) {}

void CLineOfSightTracker::SetTarget(TUniqueId target) {}

void CLineOfSightTracker::Update(float dt, CStateManager& mgr) {}
