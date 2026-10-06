#include "MetroidPrime/ScriptObjects/CPlatformWaypointTracker.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

CPlatformWaypointTracker::CPlatformWaypointTracker(float duration, TUniqueId owner)
: mLastTime(0.f), mDuration(duration), mOwnerId(owner) {}

CPlatformWaypointTracker::~CPlatformWaypointTracker() {}

void CPlatformWaypointTracker::Build(TUniqueId firstWaypoint, const CMotionSpline& motion,
                                     CMayaSpline& control, bool removeClosingTime,
                                     CStateManager& mgr) {
  const CScriptWaypoint* waypoint =
      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(firstWaypoint));
  mWaypoints.clear();
  mWaypoints.reserve(motion.GetKnotCount());

  rstl::vector< TUniqueId > visited;
  visited.reserve(motion.GetKnotCount());
  const float length = motion.GetLength();
  uint knot = 0;
  while (waypoint) {
    TTimeList intersections;
    control.FindIntersections(motion.GetKnotLength(knot) / length, intersections);
    if (removeClosingTime && intersections.size() > 1 &&
        CMath::IsEpsilon(intersections.front(), 0.f, 0.02f) &&
        CMath::IsEpsilon(intersections.back(), motion.GetKnotTime(motion.GetKnotCount() - 1),
                         0.02f)) {
      intersections.erase(intersections.end() - 1);
    }

    const TTimeList forwardTimes = control.FilterLeftIntersections(intersections);
    const TTimeList backwardTimes = control.FilterRightIntersections(intersections);
    mWaypoints.push_back_unsafe(CWaypointTimes(firstWaypoint, forwardTimes, backwardTimes));
    visited.push_back_unsafe(firstWaypoint);

    firstWaypoint = waypoint->FindConnectedObject(mgr, kSS_Arrived, kSM_Next);
    waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(firstWaypoint));
    if (rstl::find(visited.begin(), visited.end(), firstWaypoint) != visited.end()) {
      break;
    }
    ++knot;
  }
}

void CPlatformWaypointTracker::SendArrivals(float time, bool passedEnd, bool passedStart,
                                            bool forward, CMayaSpline& control,
                                            CStateManager& mgr) {
  for (int i = 0; i < mWaypoints.size(); ++i) {
    if (!mWaypoints[i].HasCrossedTime(mLastTime, time, mDuration, passedEnd, passedStart,
                                      forward)) {
      continue;
    }

    bool sendArrival = true;
    if (passedEnd || passedStart) {
      if (forward) {
        if (control.IsSegmentConstant(control.GetKnots().size() - 2) &&
            CMath::IsEpsilon(control.EvaluateAt(mLastTime), control.EvaluateAt(time), 0.00001f)) {
          sendArrival = false;
        }
      } else if (control.IsSegmentConstant(0) &&
                 CMath::IsEpsilon(control.EvaluateAt(mLastTime), control.EvaluateAt(time),
                                  0.00001f)) {
        sendArrival = false;
      }
    }

    CScriptWaypoint* waypoint =
        TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mWaypoints[i].GetWaypointId()));
    if (waypoint && sendArrival) {
      mgr.SendScriptMsg(waypoint, mOwnerId, EScriptObjectMessage('ARRV'), kInvalidUniqueId);
    }
  }
  mLastTime = time;
}

void CPlatformWaypointTracker::SetTime(float time) { mLastTime = time; }

float CPlatformWaypointTracker::GetWaypointTime(TUniqueId waypoint) const {
  for (int i = 0; i < mWaypoints.size(); ++i) {
    if (mWaypoints[i].GetWaypointId() == waypoint) {
      return mWaypoints[i].GetFirstTime();
    }
  }
  return -1.f;
}

float CPlatformWaypointTracker::FindNextWaypointTime(float time, TUniqueId& waypoint) const {
  float nextTime = 1000000.f;
  bool found = false;
  for (int i = 0; i <= mWaypoints.size() - 1; ++i) {
    const float candidate = mWaypoints[i].FindNextForwardTime(time);
    if (candidate >= 0.f && candidate < nextTime) {
      nextTime = candidate;
      waypoint = mWaypoints[i].GetWaypointId();
      found = true;
    }
  }
  if (found) {
    return nextTime;
  }
  waypoint = kInvalidUniqueId;
  return time;
}

CPlatformWaypointTracker::CWaypointTimes::CWaypointTimes(TUniqueId waypoint,
                                                         const TTimeList& forwardTimes,
                                                         const TTimeList& backwardTimes)
: mWaypointId(waypoint), mForwardTimes(forwardTimes), mBackwardTimes(backwardTimes) {}

CPlatformWaypointTracker::CWaypointTimes::CWaypointTimes(const CWaypointTimes& other)
: mWaypointId(other.mWaypointId)
, mForwardTimes(other.mForwardTimes)
, mBackwardTimes(other.mBackwardTimes) {}

CPlatformWaypointTracker::CWaypointTimes::~CWaypointTimes() {}

float CPlatformWaypointTracker::CWaypointTimes::GetFirstTime() const {
  float time = -1.f;
  if (!mForwardTimes.empty()) {
    time = mForwardTimes.front();
  }
  if (!mBackwardTimes.empty() && time > mBackwardTimes.front()) {
    time = mBackwardTimes.front();
  }
  return time;
}

float CPlatformWaypointTracker::CWaypointTimes::FindNextForwardTime(float time) const {
  for (int i = 0; i <= mForwardTimes.size() - 1; ++i) {
    if (mForwardTimes[i] > time) {
      return mForwardTimes[i];
    }
  }
  return -1.f;
}

bool CPlatformWaypointTracker::CWaypointTimes::HasCrossedTime(float oldTime, float newTime,
                                                              float duration, bool passedEnd,
                                                              bool passedStart,
                                                              bool forward) const {
  const float minTime = rstl::min_val(newTime, oldTime);
  const float maxTime = rstl::max_val(newTime, oldTime);
  const TTimeList& times = forward ? mForwardTimes : mBackwardTimes;

  if (!passedEnd && !passedStart) {
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] > minTime && times[i] <= maxTime) {
        return true;
      }
    }
  } else if (passedEnd) {
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] > maxTime || times[i] <= minTime) {
        return true;
      }
    }
  } else {
    const float closingTime = duration - minTime;
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] < maxTime || times[i] >= closingTime) {
        return true;
      }
    }
  }
  return false;
}
