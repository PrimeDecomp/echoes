#ifndef _CINGSPOTPATHFINDNAVIGATION
#define _CINGSPOTPATHFINDNAVIGATION

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"

class CPatterned;
class CStateManager;

class CIngSpotPathFindNavigation {
public:
  CIngSpotPathFindNavigation();
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt, CPatterned& actor);
  bool HasPath(const CPatterned& actor) const;
  bool IsPathOver(const CPatterned& actor) const;

private:
  CVector3f mPreviousPosition; // Guessed name
  int mCurrentWaypoint;        // Guessed name
};
CHECK_SIZEOF(CIngSpotPathFindNavigation, 0x10)

#endif
