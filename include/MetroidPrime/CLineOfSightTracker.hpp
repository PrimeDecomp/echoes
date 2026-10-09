#ifndef _CLINEOFSIGHTTRACKER
#define _CLINEOFSIGHTTRACKER

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CStateManager;

// Guessed class name
class CLineOfSightTracker {
public:
  CLineOfSightTracker(TUniqueId owner, CSegId segment, float minimumCheckInterval,
                      float checkIntervalRange);
  void Update(float dt, CStateManager& mgr);                                // Guessed name
  void SetTarget(TUniqueId target);                                         // Guessed name
  bool HasLineOfSight() const { return mHasLineOfSight; }                   // Guessed name
  void ClearLineOfSight() { mHasLineOfSight = false; }                      // Guessed name
  void SetSegment(const CSegId& segment) { mSegment = segment; }            // Guessed name
  void SetRayFilter(const CMaterialFilter& filter) { mRayFilter = filter; } // Guessed name
  float GetClearTime() const { return mClearTime; }                         // Guessed name
  float GetBlockedTime() const { return mBlockedTime; }                     // Guessed name

private:
  TUniqueId mOwner;            // Guessed name
  CSegId mSegment;             // Guessed name
  CMaterialFilter mRayFilter;  // Guessed name
  TUniqueId mTarget;           // Guessed name
  float mMinimumCheckInterval; // Guessed name
  float mCheckIntervalRange;   // Guessed name
  float mTimeUntilNextCheck;   // Guessed name
  float mClearTime;            // Guessed name
  float mBlockedTime;          // Guessed name
  bool mHasLineOfSight : 1;    // Guessed name
};
CHECK_SIZEOF(CLineOfSightTracker, 0x40)

#endif // _CLINEOFSIGHTTRACKER
