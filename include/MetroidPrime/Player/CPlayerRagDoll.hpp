#ifndef _CPLAYERRAGDOLL
#define _CPLAYERRAGDOLL

#include "MetroidPrime/CRagDoll.hpp"

class CPhysicsActor;

// Guessed name. Native player-owned specialization of CRagDoll; its sound and
// impulse members also correspond structurally to Prime's CPirateRagDoll.
class CPlayerRagDoll : public CRagDoll {
public:
  CPlayerRagDoll(CStateManager& mgr, CPhysicsActor* actor, ushort thudSfx, uint flags);
  ~CPlayerRagDoll() {}

  // CRagDoll
  void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData) override;
  void Update(CStateManager& mgr, float dt, float waterTop) override;
  void PreRender(const CVector3f& pos, CModelData& modelData) override;

private:
  // Guessed names, established by construction, Prime and Update consumers.
  CPhysicsActor* mActor;
  ushort mThudSfx;
  float mSfxTimer;
  CVector3f mLastSfxPos;
  CVector3f mTorsoImpulse;
  CAABox mOriginalBounds;
  bool mInitSfx : 1;
};
CHECK_SIZEOF(CPlayerRagDoll, 0xe0)

#endif // _CPLAYERRAGDOLL
