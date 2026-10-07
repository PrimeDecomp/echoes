#ifndef _CATOMICALPHA
#define _CATOMICALPHA

#include "types.h"

#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

// Prime 1 has the same class. The Echoes version flashes red when damaged, follows its patrol
// path with a "Pathfind" state and homes in on every player that charges a beam.
class CAtomicAlpha : public CPatterned {
public:
  CAtomicAlpha(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
               const CPatternedInfo& pInfo, CAssetId bombWeapon, const CDamageInfo& bombDamage,
               float bombDropDelay, float bombReappearDelay, float bombRappearTime, CAssetId cmdl,
               bool invisible, bool applyBeamAttraction);

  // CEntity
  ~CAtomicAlpha() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override { return &mBombProjectile; }
  CPathFindSearch* GetSearchPath() override { return &mPathFind; }
  void SetupStateMachine(CStateManager& mgr) override;

  // States
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Pathfind(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);

  // Triggers
  bool AggressionCheck(CStateManager& mgr, const CTriggerData& data) const;

  bool CanDropBomb() const {
    return mBombTime >= mBombDropDelay &&
           mBombLocators[0].mScaleTime > (mBombReappearDelay + mBombRappearTime);
  }

private:
  enum { kBombCount = 4 };

  struct SBomb {
    rstl::string mLocatorName;
    pas::ELocomotionType mLocomotionType;
    float mScaleTime;

    SBomb(const rstl::string& name, pas::ELocomotionType loco, float scale)
    : mLocatorName(name), mLocomotionType(loco), mScaleTime(scale) {}
  };

  bool mInRange : 1;
  bool mInvisible : 1;
  bool mApplyBeamAttraction : 1;
  float mBombDropDelay;
  float mBombReappearDelay;
  float mBombRappearTime;
  float mBombTime;
  int mCurBomb;
  CPathFindSearch mPathFind;
  CSteeringBehaviors mSteeringBehaviors;
  CProjectileInfo mBombProjectile;
  CModelData mBombModel;
  rstl::reserved_vector< SBomb, kBombCount > mBombLocators;
  TUniqueId mWaypointId;
  float mTouchRadius;
};
CHECK_SIZEOF(CAtomicAlpha, 0x9a8)

#endif // _CATOMICALPHA
