#ifndef _CPLANTSCARABSWARM
#define _CPLANTSCARABSWARM

#include "types.h"

#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"

// Guessed class: a swarm of plant scarabs. A limited number of boids may attack at once; each
// attacker plays a one-shot animation on one of a few spare models, whose animation drops a
// bouncy grenade on the player, and is killed once that animation ends.
class CPlantScarabSwarm : public CSwarmBasics {
public:
  CPlantScarabSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CVector3f& boundingBoxExtent, const CTransform4f& xf,
                    const CAnimRes& animRes, CActorParameters actorParameters,
                    const CBasicSwarmData& data, int intoAttackAnimation, int attackAnimation,
                    float maxAttackAngle, float intoAttackSpeed, float attackSpeed,
                    float grenadeMass, float grenadeLaunchSpeed, float grenadeSpeed,
                    CDamageInfo grenadeDamage, float grenadeExplodePlayerDistance,
                    uint grenadeBounces, CAssetId grenadeExplosionEffect,
                    CAssetId grenadeExplosionXRayEffect, CAssetId grenadeTrailEffect,
                    CAssetId grenadeEffect, ushort grenadeBounceSound, ushort grenadeExplosionSound,
                    float grenadeBounceSoundFallOff, float grenadeBounceMaxAudibleDistance,
                    float grenadeExplosionSoundFallOff, float grenadeExplosionMaxAudibleDistance);

  // CEntity
  ~CPlantScarabSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CSwarmBasics
  void PreRenderBoid(CBoid* boid, uint* drawMask) override;
  void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                  int partitionIndex) override;
  void ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                              const rstl::reserved_vector< CBoid*, 50 >& nearList) override;
  void KillBoid(CBoid& boid, CStateManager& mgr, const CWeaponMode& weapon) override;
  void RenderBoid(CBoid* boid) const override;
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) override;
  void UpdateSwarmAnimations(CStateManager& mgr, float dt) override;
  void UpdateAllBoidMovement(CStateManager& mgr, float dt) override;
  void BoidCollidedCallback(CStateManager& mgr, CBoid& boid) override;
  void BoidCollidedWithPlayerCallback(CStateManager& mgr, CBoid& boid) override;

private:
  CAABox GetGrenadeBounds() const;                                               // Guessed name
  CVector3f GetBoidTopPosition(const CBoid& boid) const;                         // Guessed name
  bool IsSpaceAboveBoidClear(const CStateManager& mgr, const CBoid& boid) const; // Guessed name
  void SpawnGrenades(CStateManager& mgr, CAnimData& animData, int slot);         // Guessed name

  int mAttackerCount;     // Guessed name
  float mAttackProximity; // Guessed name
  float mAttackTimer;     // Guessed name
  float mMaxAttackAngle;  // Guessed name; radians
  rstl::vector< SwarmRenderHelpers::CSwarmSkinnedModelState > mAttackModelStates; // Guessed name
  rstl::vector< CModelData > mAttackModels;                                       // Guessed name
  rstl::vector< CAdvancementDeltas > mAttackDeltas;                               // Guessed name
  rstl::vector< int > mAttackSlotBoids; // Guessed name; boid index per attack model, -1 if free
  rstl::single_ptr< CModelData > mSharedAttackModel; // Guessed name
  rstl::single_ptr< SwarmRenderHelpers::CSwarmSkinnedModelState >
      mSharedAttackState;                                     // Guessed name
  rstl::single_ptr< CAdvancementDeltas > mSharedAttackDeltas; // Guessed name
  uint mSharedAttackBit;                                      // Guessed name
  uint mAttackModelBitBase;                                   // Guessed name
  float mIntoAttackSpeed;                                     // Guessed name
  float mAttackSpeed;                                         // Guessed name
  CBouncyGrenadeData mGrenadeData;                            // Guessed name
  float mGrenadeLaunchSpeed;                                  // Guessed name
  float mGrenadeExplodePlayerDistance;                        // Guessed name
};
CHECK_SIZEOF(CPlantScarabSwarm, 0x660)

#endif // _CPLANTSCARABSWARM
