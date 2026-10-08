#ifndef _CEMPERORINGSTAGE2TENTACLE
#define _CEMPERORINGSTAGE2TENTACLE

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
struct SLdrEmperorIngStage2TentacleData;

// Guessed name: one of the tentacles of the Emperor Ing's second stage. It lies retracted until the
// player lingers nearby, then lashes out with a collision-actor chain along its joints.
class CEmperorIngStage2Tentacle : public CPatterned {
public:
  CEmperorIngStage2Tentacle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                            const CTransform4f& xf, const CModelData& modelData,
                            const CActorParameters& actorParams,
                            const CPatternedInfo& patternedInfo,
                            const SLdrEmperorIngStage2TentacleData& data);

  // CEntity
  ~CEmperorIngStage2Tentacle() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CEmperorIngStage2Tentacle
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool ShouldRetract(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  void Retracted(CStateManager& mgr, EStateMsg msg, float dt);            // Guessed name
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void UpdateCollisionActors(float dt, CStateManager& mgr); // Guessed name
  void DestroyCollisionActors(CStateManager& mgr);          // Guessed name
  void SetupCollisionActors(CStateManager& mgr);            // Guessed name

  float mSpotTime;                                                   // Guessed name
  float mLostTime;                                                   // Guessed name
  float mActiveTime;                                                 // Guessed name
  float mAttackTime;                                                 // Guessed name
  bool mAttacking : 1;                                               // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  float mDetectionTime;                                              // Guessed name
  float mForgetTime;                                                 // Guessed name
};
CHECK_SIZEOF(CEmperorIngStage2Tentacle, 0x7e0)

#endif // _CEMPERORINGSTAGE2TENTACLE
