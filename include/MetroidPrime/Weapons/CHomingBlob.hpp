#ifndef _CHOMINGBLOB
#define _CHOMINGBLOB

#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionCache;
struct SCachedCollisionSlot;
class CElementGen;
class CGenDescription;

// Guessed class and private names; the target creates a light named HomingBlobLight.
class CHomingBlob : public CWeapon {
public:
  enum EModeFlags {
    kMF_FollowPlayerArea = 2,
    kMF_SkipInitialTargets = 4,
  };

  CHomingBlob(const TToken< CGenDescription >& particle, TUniqueId uid, TAreaId areaId,
              TUniqueId owner, bool active, const CAABox& bounds, const CDamageInfo& damage,
              int playerIndex, const rstl::string& name, const CTransform4f& xf, int modeFlags,
              float generatorRate, float collisionRadius, float nearTargetDistance,
              float escapeDistance, float targetSearchRadius, float homingAcceleration);

  // CEntity
  ~CHomingBlob() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  enum EParticleFlag {
    kPF_Damaged = 0,
    kPF_Escaped = 1,
  };
  // Guessed layout: the particle's advanced values are reused as integer homing state.
  struct SParticleState {
    union {
      float mUnassigned;
      int mTargetSlot;
    };
    uint mFlags;

    bool HasFlag(int bit) const { return (mFlags >> bit) & 1; }
    void SetFlag(int bit) { mFlags |= 1 << bit; }
  };

  void UpdateParticles(CStateManager& mgr);
  bool FindNearestTriangle(float radius, const CVector3f& position, CVector3f& closest,
                           const SCachedCollisionSlot*& slot, CVector3f& barycentric);

  CAABox mCollisionBounds;
  rstl::single_ptr< CElementGen > mParticleGen;
  rstl::single_ptr< CCollisionCache > mCollisionCache;
  TUniqueId mLightId;
  CAssetId mParticleAssetId;
  rstl::reserved_vector< TUniqueId, 16 > mTargetIds;
  uint mNextParticleTarget;
  uint mParticleUpdatePhase;
  float mElapsedTime;
  float x220_; // Native initialization is six; the remaining Touch comparisons have no effect.
  float mGeneratorRate;
  float mCollisionRadius;
  float mNearTargetDistance;
  float mEscapeDistanceSquared;
  float mTargetSearchRadius;
  float mHomingAcceleration;
  int mPlayerIndex;
  int mModeFlags;
  bool mFollowPlayerArea : 1;
  bool mHasRenderBounds : 1;
};
CHECK_SIZEOF(CHomingBlob, 0x248)

#endif // _CHOMINGBLOB
