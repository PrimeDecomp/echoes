#include "MetroidPrime/ScriptObjects/CPlatformWaypointTracker.hpp"

CPlatformWaypointTracker::CPlatformWaypointTracker(float duration, TUniqueId owner)
: mLastTime(0.f), mDuration(duration), mOwnerId(owner) {}

CPlatformWaypointTracker::~CPlatformWaypointTracker() {}

void CPlatformWaypointTracker::Build(TUniqueId firstWaypoint, const CMotionSpline& motion,
                                     CMayaSpline& control, bool removeClosingTime,
                                     CStateManager& mgr) {}

void CPlatformWaypointTracker::SendArrivals(float time, bool passedEnd, bool passedStart,
                                            bool forward, CMayaSpline& control,
                                            CStateManager& mgr) {}

void CPlatformWaypointTracker::SetTime(float time) {}

float CPlatformWaypointTracker::GetWaypointTime(TUniqueId waypoint) const {}

float CPlatformWaypointTracker::FindNextWaypointTime(float time, TUniqueId& waypoint) const {}

CPlatformWaypointTracker::CWaypointTimes::CWaypointTimes(TUniqueId waypoint,
                                                         const TTimeList& forwardTimes,
                                                         const TTimeList& backwardTimes)
: mWaypointId(waypoint), mForwardTimes(forwardTimes), mBackwardTimes(backwardTimes) {}

CPlatformWaypointTracker::CWaypointTimes::CWaypointTimes(const CWaypointTimes& other)
: mWaypointId(other.mWaypointId)
, mForwardTimes(other.mForwardTimes)
, mBackwardTimes(other.mBackwardTimes) {}

CPlatformWaypointTracker::CWaypointTimes::~CWaypointTimes() {}

float CPlatformWaypointTracker::CWaypointTimes::GetFirstTime() const {}

float CPlatformWaypointTracker::CWaypointTimes::FindNextForwardTime(float time) const {}

bool CPlatformWaypointTracker::CWaypointTimes::HasCrossedTime(float oldTime, float newTime,
                                                              float duration, bool passedEnd,
                                                              bool passedStart,
                                                              bool forward) const {}
