#ifndef _CINGBLOBSWARM
#define _CINGBLOBSWARM

#include "types.h"

#include "MetroidPrime/Enemies/CSwarmBasics.hpp"

#include "Kyoto/Animation/CAdvancementDeltas.hpp"

// Guessed class: a swarm of Ing blobs that launch themselves at the player. A limited number of
// boids may attack at once; each attacker plays a one-shot animation on one of a few spare models,
// then keeps flying with the animation of one shared attack model.
class CIngBlobSwarm : public CSwarmBasics {
public:
  CIngBlobSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CVector3f& boundingBoxExtent, const CTransform4f& xf, const CAnimRes& animRes,
                CActorParameters actorParameters, const CBasicSwarmData& data,
                int intoAttackAnimation, int attackAnimation, float maxAttackAngle,
                float intoAttackSpeed, float attackSpeed, float mass, float maxAttackHeight,
                const CVector3f& attackAimOffset);

  // CEntity
  ~CIngBlobSwarm() override;
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
  void LaunchBoid(CBoid& boid, const CVector3f& target); // Guessed name

  int mAttackerCount;     // Guessed name
  float mAttackProximity; // Guessed name
  float mAttackTimer;     // Guessed name
  float mMaxAttackAngle;  // Guessed name; radians
  rstl::vector< SwarmRenderHelpers::CSwarmSkinnedModelState > mAttackModelStates; // Guessed name
  rstl::vector< CModelData > mAttackModels;                                       // Guessed name
  rstl::vector< CAdvancementDeltas > mAttackDeltas;                               // Guessed name
  rstl::vector< bool > mAttackSlotInUse;                                          // Guessed name
  rstl::single_ptr< CModelData > mSharedAttackModel;                              // Guessed name
  rstl::single_ptr< SwarmRenderHelpers::CSwarmSkinnedModelState >
      mSharedAttackState;                                     // Guessed name
  rstl::single_ptr< CAdvancementDeltas > mSharedAttackDeltas; // Guessed name
  uint mSharedAttackBit;                                      // Guessed name
  uint mAttackModelBitBase;                                   // Guessed name
  float mIntoAttackSpeed;                                     // Guessed name
  float mAttackSpeed;                                         // Guessed name
  float mMass;                                                // Guessed name
  float mMaxAttackHeight;                                     // Guessed name
  CVector3f mAttackAimOffset;                                 // Guessed name
  bool mSharedAttackActive : 1;                               // Guessed name
};
CHECK_SIZEOF(CIngBlobSwarm, 0x620)

#endif // _CINGBLOBSWARM
