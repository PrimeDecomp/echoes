#ifndef _CPATHFINDNAVIGATION
#define _CPATHFINDNAVIGATION

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CPatterned;
class CStateManager;

class CPathFindNavigation {
public:
  CPathFindNavigation();
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt, CPatterned& actor);
  void SetDestination(const CVector3f& position);

private:
  // Guessed names, recovered from the path-search and steering callers.
  void StartPathFind(CStateManager& mgr, CPatterned& actor);
  void ApproachDest(CStateManager& mgr, CPatterned& actor);

  // Guessed member names, supported by native setters/search/steering consumers.
  CVector3f mSegmentStart;
  CVector3f mPathTarget;
  CVector3f mDestinationPosition;
  TUniqueId mFaceTarget;
  bool mInPosition : 1;
  bool x26_25_ : 1;
};
CHECK_SIZEOF(CPathFindNavigation, 0x28)

#endif
