#ifndef _CWALLWALKER
#define _CWALLWALKER

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CWallCrawler.hpp"

#include "Kyoto/TToken.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CGenDescription;
class CWeaponDescription;

// Guessed name. Loader-built bundle of the WallWalker properties.
class CWallWalkerData {
public:
  CWallWalkerData(float stickyReach, float floorTurnSpeed, float waypointApproachDistance,
                  float visibleDistance, float projectileInterval, float projectileStopHomingRange,
                  const CDamageVulnerability& legVulnerability,
                  const TLockedToken< CWeaponDescription >& projectile,
                  const TLockedToken< CGenDescription >& projectileVisorParticle,
                  const CDamageInfo& projectileDamage, const CCameraShakerData& projectileShakeData,
                  const CBouncyGrenadeData& grenadeData)
  : mStickyReach(stickyReach)
  , mFloorTurnSpeed(floorTurnSpeed)
  , mWaypointApproachDistance(waypointApproachDistance)
  , mVisibleDistance(visibleDistance)
  , mProjectileInterval(projectileInterval)
  , mProjectileStopHomingRange(projectileStopHomingRange)
  , mLegVulnerability(legVulnerability)
  , mProjectile(projectile)
  , mProjectileVisorParticle(projectileVisorParticle)
  , mProjectileDamage(projectileDamage)
  , mProjectileShakeData(projectileShakeData)
  , mGrenadeData(grenadeData) {}

  float mStickyReach;
  float mFloorTurnSpeed;
  float mWaypointApproachDistance;
  float mVisibleDistance;
  float mProjectileInterval;
  float mProjectileStopHomingRange;
  CDamageVulnerability mLegVulnerability;
  TLockedToken< CWeaponDescription > mProjectile;
  TLockedToken< CGenDescription > mProjectileVisorParticle;
  CDamageInfo mProjectileDamage;
  CCameraShakerData mProjectileShakeData;
  CBouncyGrenadeData mGrenadeData;
};
CHECK_SIZEOF(CWallWalkerData, 0x1c0)

// Wii SEL class name (TypesMatch__11CWallWalkerCFi). The legged, grenade-dropping wall walker.
class CWallWalker : public CWallCrawler {
public:
  enum EPatrolState { kPS_Patrol, kPS_StartGenerate, kPS_Generate }; // Guessed names.

  CWallWalker(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
              const CModelData& mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, const CWallWalkerData& data);

  // CEntity
  ~CWallWalker() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CWallWalker
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);

  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void LeftLegHitReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void RightLegHitReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void ShootProjectile(CStateManager& mgr, EStateMsg msg, float dt);

  bool IsLeftLegHit(CStateManager& mgr, const CTriggerData& data) const;
  bool IsRightLegHit(CStateManager& mgr, const CTriggerData& data) const;
  bool AreBothLegsHit(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldShootProjectile(CStateManager& mgr, const CTriggerData& data) const;

  void SetNumberShots(CStateManager& mgr, float arg);

private:
  void LaunchProjectiles(const CTransform4f& xf, CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void DestroyCollisionManager(CStateManager& mgr);
  void UpdateCollisionManager(float dt, CStateManager& mgr);

  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;
  EPatrolState mPatrolState;
  CDamageVulnerability mLegVulnerability;
  CBouncyGrenadeData mGrenadeData;
  bool mLeftLegHit : 1;
  bool mRightLegHit : 1;
  bool mExploded : 1;
  bool mLegHitByMissile : 1;
  TLockedToken< CWeaponDescription > mProjectile;
  TLockedToken< CGenDescription > mProjectileVisorParticle;
  CDamageInfo mProjectileDamage;
  CCameraShakerData mProjectileShakeData;
  int mNumShots;
  float mProjectileTimer;
  float mProjectileInterval;
  float mProjectileStopHomingRange;
  float mDeathTime;
  TUniqueId mGrenadeId;
};
CHECK_SIZEOF(CWallWalker, 0xa38)

#endif // _CWALLWALKER
