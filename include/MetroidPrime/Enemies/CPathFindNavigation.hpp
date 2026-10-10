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
  void SetFaceTarget(const TUniqueId& id) { mFaceTarget = id; }
  TUniqueId GetFaceTarget() const { return mFaceTarget; } // Guessed name
  const CVector3f& GetDestinationPosition() const { return mDestinationPosition; } // Guessed name
  void SetUseLocomotionFacing(bool use) { mUseLocomotionFacing = use; } // Guessed name

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
  bool mUseLocomotionFacing : 1;
};
CHECK_SIZEOF(CPathFindNavigation, 0x28)

#endif
