#ifndef _CWAYPOINTNAVIGATION
#define _CWAYPOINTNAVIGATION

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CPatterned;
class CStateManager;

class CWaypointNavigation {
public:
  CWaypointNavigation();
  void Update(float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt, CPatterned& actor);
  void ConfigureWobbleSteering(bool clockwise, float strength);

private:
  enum EPatrolState { kPS_Invalid = -1, kPS_Moving, kPS_Paused, kPS_Done };
  float mMoveSpeed;
  float mPauseRemainingTime;
  TUniqueId mDestination;
  TUniqueId mLastDestination;
  EPatrolState mPatrolState;
  CVector3f mSegmentStart;
  CVector3f mDestinationPosition;
  CVector3f mFaceVector;
  float mWobbleSteering;
  bool mInPosition : 1;
  bool mHorizontalMovement : 1;
};
CHECK_SIZEOF(CWaypointNavigation, 0x3c)

#endif
