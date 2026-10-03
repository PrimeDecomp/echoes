#ifndef _CPLAYERSTUCKTRACKER
#define _CPLAYERSTUCKTRACKER

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TReservedAverage.hpp"

// Prime-correlated names; Echoes retains only samples from the starting-jump state.
class CPlayerStuckTracker {
public:
  enum EPlayerState { kPS_Jump, kPS_StartingJump, kPS_Moving };

  CPlayerStuckTracker();
  void AddState(EPlayerState state, const CVector3f& position, const CVector3f& velocity,
                const CVector2f& input);
  bool IsPlayerStuck() const;
  void ResetStats();

private:
  TReservedAverage< CVector3f, 20 > mPositions;
  TReservedAverage< float, 20 > mSpeeds;
  TReservedAverage< CVector2f, 20 > mInputs;
  int mUpdateCount;
};
CHECK_SIZEOF(CPlayerStuckTracker, 0x1f0)

#endif // _CPLAYERSTUCKTRACKER
