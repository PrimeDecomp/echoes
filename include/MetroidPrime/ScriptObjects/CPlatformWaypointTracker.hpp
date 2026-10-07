#ifndef _CPLATFORMWAYPOINTTRACKER
#define _CPLATFORMWAYPOINTTRACKER

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CMayaSpline;
class CMotionSpline;
class CStateManager;

class CPlatformWaypointTracker { // Guessed name
public:
  typedef rstl::reserved_vector< float, 8 > TTimeList;

  class CWaypointTimes { // Guessed name
  public:
    CWaypointTimes(TUniqueId waypoint, const TTimeList& forwardTimes,
                   const TTimeList& backwardTimes);
    CWaypointTimes(const CWaypointTimes& other);
    virtual ~CWaypointTimes();

    TUniqueId GetWaypointId() const { return mWaypointId; } // Guessed name
    float GetFirstTime() const;                  // Guessed name
    float FindNextForwardTime(float time) const; // Guessed name
    bool HasCrossedTime(float oldTime, float newTime, float duration, bool passedEnd,
                        bool passedStart, bool forward) const; // Guessed name

  private:
    TUniqueId mWaypointId;    // Guessed name
    TTimeList mForwardTimes;  // Guessed name
    TTimeList mBackwardTimes; // Guessed name
  };
  typedef char WaypointTimesSizeCheck[sizeof(CWaypointTimes) == 0x50 ? 1 : -1];

  CPlatformWaypointTracker(float duration, TUniqueId owner);
  virtual ~CPlatformWaypointTracker() {}

  void Build(TUniqueId firstWaypoint, const CMotionSpline& motion, CMayaSpline& control,
             bool removeClosingTime, CStateManager& mgr); // Guessed name
  void SendArrivals(float time, bool passedEnd, bool passedStart, bool forward,
                    CMayaSpline& control, CStateManager& mgr);       // Guessed name
  void SetTime(float time);                                          // Guessed name
  float GetWaypointTime(TUniqueId waypoint, const CStateManager& mgr) const;                   // Guessed name
  float FindNextWaypointTime(float time, TUniqueId& waypoint) const; // Guessed name

private:
  rstl::vector< CWaypointTimes > mWaypoints; // Guessed name
  float mLastTime;                           // Guessed name
  float mDuration;                           // Guessed name
  TUniqueId mOwnerId;                        // Guessed name
};
CHECK_SIZEOF(CPlatformWaypointTracker, 0x20)

#endif
