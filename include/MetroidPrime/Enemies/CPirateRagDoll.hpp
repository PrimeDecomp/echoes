#ifndef _CPIRATERAGDOLL
#define _CPIRATERAGDOLL

#include "Kyoto/Animation/CSegId.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "rstl/reserved_vector.hpp"

class CJointData_LinearStorage;
class CPatterned;

// Guessed name. REL-resident CRagDoll for the pirate characters; it corresponds structurally to
// Prime's CPirateRagDoll but is driven by a CPatterned, takes its particle radii from the
// caller and can pin particles to connected AI waypoints.
class CPirateRagDoll : public CRagDoll {
public:
  CPirateRagDoll(CStateManager& mgr, CPatterned* actor, ushort thudSfx, uint flags, float gravity,
                 float floatingGravity, const rstl::reserved_vector< float, 14 >& radii);

  // CRagDoll
  void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData) override;
  void Update(CStateManager& mgr, float dt, float waterTop) override;
  void PreRender(const CVector3f& pos, CModelData& modelData) override;
  void CheckStatic(float dt) override;

  CVector3f& TorsoImpulse() { return mTorsoImpulse; }
  void SetActorAttached(bool attached) { mActorAttached = attached; } // Guessed name.

private:
  // Guessed names.
  void UpdateImpactSfx(CStateManager& mgr);
  void AlignShoulderPad(CJointData_LinearStorage& pose, int shoulder, int elbow,
                        const CSegId& padId, const CVector3f& axis, float side);

  CPatterned* mActor;
  ushort mThudSfx;
  float mSfxTimer;
  CVector3f mLastSfxPos;
  CVector3f mTorsoImpulse;
  rstl::reserved_vector< TUniqueId, 6 > mWaypointIds;
  rstl::reserved_vector< long, 6 > mWaypointParticles;
  rstl::reserved_vector< bool, 6 > mWaypointActive;
  float mMinImpactVelocity;
  float mImpactVolumeScale;
  float mMaxImpactVolume;
  float mSfxInterval;
  float mElapsedTime;
  CSegId mLeftShoulderPadId;
  CSegId mRightShoulderPadId;
  float mPrevWaterTop;
  bool mInitSfx : 1;
  bool mActorAttached : 1;
};
CHECK_SIZEOF(CPirateRagDoll, 0x11c)

#endif // _CPIRATERAGDOLL
