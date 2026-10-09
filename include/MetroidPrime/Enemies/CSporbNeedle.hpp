#ifndef _CSPORBNEEDLE
#define _CSPORBNEEDLE

#include "types.h"

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/single_ptr.hpp"

// Guessed class name: the needle a Sporb fires, which flies straight and explodes on impact.
class CSporbNeedle : public CPhysicsActor {
public:
  CSporbNeedle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& modelData,
               const CActorParameters& actorParams, TUniqueId ownerId, CAssetId explosionEffect,
               CAssetId trailEffect, ushort launchSound, ushort flightSound, ushort hitPlayerSound,
               ushort collisionSound, ushort explosionSound, const CDamageInfo& attackDamage,
               float initialSpeed, float mass, float fuseTime);

  // CEntity
  ~CSporbNeedle() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mSphere; }
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

private:
  void PlaySfx(const ushort sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx, 1.f, 0, 0.1f, 50.f, CSegId(0), 0, 0, 0.f, 20, 127,
                      GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  void PlayLoopedSfx(ushort sfx, CStateManager& mgr) {
    ProcessSoundEvent(sfx | 0xa0000000, 1.f, 0, 0.1f, 50.f, CSegId(0), 0, 0, 0.f, 20, 127,
                      GetClosestCameraDistanceSq(mgr), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
  float GetClosestCameraDistanceSq(CStateManager& mgr) const;
  void UpdateFuse(float dt, CStateManager& mgr);
  void Explode(CStateManager& mgr, TUniqueId hitId);
  void UpdateEffects(float dt, CStateManager& mgr);

  TUniqueId mOwnerId;
  float mFuseTime;
  float mFuseTimer;
  rstl::single_ptr< CElementGen > mExplosionGen;
  rstl::single_ptr< CElementGen > mTrailGen;
  ushort mLaunchSound;
  ushort mFlightSound;
  ushort mHitPlayerSound;
  ushort mCollisionSound;
  ushort mExplosionSound;
  CDamageInfo mAttackDamage;
  uchar mThinkCount;
  CCollidableSphere mSphere;
  bool mExploded : 1;
  bool mHitWorld : 1;
  bool mLaunchSoundPlayed : 1;
};
CHECK_SIZEOF(CSporbNeedle, 0x338)

CEntity* REL_LoadSporbNeedle(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSPORBNEEDLE
