#ifndef _CPLAYERTURRET
#define _CPLAYERTURRET

#include "MetroidPrime/CActor.hpp"

// Partial layout of the REL player turret object, covering only the fields the DOL reads
// directly. Guessed class and member names; the gaps are unidentified.
class CPlayerTurret : public CActor {
public:
  float GetMaxAimAngle() const { return mMaxAimAngle; }
  void SetTargetPosition(const CVector3f& position) { mTargetPosition = position; }

  uchar x158_[0x2c];
  float mMaxAimAngle; // 0x184
  uchar x188_[0x88];
  CVector3f mTargetPosition; // 0x210
};
CHECK_OFFSETOF(CPlayerTurret, mMaxAimAngle, 0x184)
CHECK_OFFSETOF(CPlayerTurret, mTargetPosition, 0x210)

#endif // _CPLAYERTURRET
