#ifndef _CSPORBPOWERBOMB
#define _CSPORBPOWERBOMB

#include "types.h"

#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

// Guessed class name: the projectile a power bomb guardian Sporb lobs, which stops on impact and
// then damages everything inside an expanding sphere.
class CSporbPowerBomb : public CEnergyProjectile {
public:
  // Guessed names; how the projectile came to rest.
  enum EState {
    kS_Invalid = -1,
    kS_HitActor = 0,
    kS_HitWorld = 1,
  };

  CSporbPowerBomb(bool active, const TToken< CWeaponDescription >& description, EWeaponType type,
                  const CTransform4f& xf, EMaterialTypes excludeMaterial, const CDamageInfo& damage,
                  TUniqueId uid, TAreaId areaId, TUniqueId owner, TUniqueId homingTarget,
                  uint attribs, bool underwater, const CVector3f& scale,
                  const CImpactVisorEffect& visorEffect, bool unused, bool playImpactSound,
                  float fuseTime, float startDamageTime, float endDamageTime, float damageWaitTime);

  // CEntity
  ~CSporbPowerBomb() override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager& mgr) const override;

  // CGameProjectile
  CRayCastResult RayCollisionCheckWithWorld(TUniqueId& idOut, const CVector3f& start,
                                            const CVector3f& end, float magnitude,
                                            rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                            CStateManager& mgr,
                                            EStaticGeometryTest staticTest) override;
  void ResolveCollisionWithActor(const CRayCastResult& result, CActor& actor,
                                 CStateManager& mgr) override;

  // CEnergyProjectile
  void ResolveCollisionWithWorld(const CRayCastResult& result, CStateManager& mgr) override;
  rstl::optional_object< TLockedToken< CGenDescription > >
  GetImpactParticle(CStateManager& mgr) override {
    return rstl::optional_object_null();
  }
  CVector3f GetExplosionNormal() const override { return mExplosionNormal; }

private:
  float mFuseTime;
  float mFuseTimer;
  float mElapsedTime;
  float mCurrentRadius;
  float mRadiusGrowthRate;
  float mStartDamageTime;
  float mEndDamageTime;
  CVector3f mExplosionNormal;
  EState mState;
  float mDamageWaitTime;
  float mDamageWaitTimer;
};
CHECK_SIZEOF(CSporbPowerBomb, 0x5a0)

#endif // _CSPORBPOWERBOMB
