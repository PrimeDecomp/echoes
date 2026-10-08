#ifndef _CAIMANNEDTURRET
#define _CAIMANNEDTURRET

#include "types.h"

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrAIMannedTurret.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

// Guessed class: a turret that tracks the player and fires at him. The turret and the rider that
// manns it are separate actors that this object finds through its script connections.
class CAIMannedTurret : public CAi {
public:
  CAIMannedTurret(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, const CModelData& modelData,
                  const CMaterialList& materials, const CHealthInfo& health,
                  const CDamageVulnerability& vulnerability, const SLdrAIMannedTurretData& data,
                  const CDamageInfo& damage, const SLdrSpline& horizontalSpline,
                  const SLdrSpline& verticalSpline, const CAssetId& telegraphEffect);

  // CEntity
  ~CAIMannedTurret() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CAIMannedTurret
  virtual void SetupStateMachine(CStateManager& mgr); // Guessed name

  void Start(CStateManager& mgr, EStateMsg msg, float dt);              // Guessed name
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);             // Guessed name
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);               // Guessed name
  bool CheckReady(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool CheckPatrol(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool CheckAttack(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool CheckDead(CStateManager& mgr, const CTriggerData& data) const;   // Guessed name
  bool CheckAlive(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  void TookDamage(CStateManager& mgr, float dt);                        // Guessed name

private:
  void AttachToActors(CStateManager& mgr);                   // Guessed name
  void UpdateRider(CStateManager& mgr, float dt);            // Guessed name
  void UpdateAim(CStateManager& mgr, float dt, bool snap);   // Guessed name
  CTransform4f GetMuzzleTransform(CStateManager& mgr) const; // Guessed name
  void Fire(CStateManager& mgr);                             // Guessed name
  void SpawnTelegraph(CStateManager& mgr);                   // Guessed name

  SLdrAIMannedTurretData mData;              // Guessed name
  TUniqueId mTurretId;                       // Guessed name
  TUniqueId mRiderId;                        // Guessed name
  int mAimUpAnim;                            // Guessed name
  int mAimDownAnim;                          // Guessed name
  int mFireAnim;                             // Guessed name
  int mDeathAnim;                            // Guessed name
  int mIdleAnim;                             // Guessed name
  float mTargetElevation;                    // Guessed name
  float mElevation;                          // Guessed name
  float mFireTimer;                          // Guessed name
  float mLeashTimer;                         // Guessed name
  CModelFlags mTurretModelFlags;             // Guessed name
  TToken< CWeaponDescription > mWeaponToken; // Guessed name
  CDamageInfo mDamageInfo;                   // Guessed name
  float mDamageFlashTimer;                   // Guessed name
  CVector3f mInitialPosition;                // Guessed name
  int x54c_;
  CVector3f mAimDirection;                                                   // Guessed name
  rstl::single_ptr< CGenericFSM2State< CPatterned > > mStateMachineState;    // Guessed name
  SLdrSpline mHorizontalSpline;                                              // Guessed name
  SLdrSpline mVerticalSpline;                                                // Guessed name
  float mPatrolTime;                                                         // Guessed name
  float mPatrolDuration;                                                     // Guessed name
  rstl::optional_object< TCachedToken< CGenDescription > > mTelegraphEffect; // Guessed name
  rstl::single_ptr< CElementGen > mTelegraphGen1;                            // Guessed name
  rstl::single_ptr< CElementGen > mTelegraphGen2;                            // Guessed name
  rstl::single_ptr< CElementGen > mTelegraphGen3;                            // Guessed name
  rstl::single_ptr< CElementGen > mTelegraphGen4;                            // Guessed name
  float mTelegraphTimer;                                                     // Guessed name
  bool mReady : 1;                                                           // Guessed name
  bool mDead : 1;                                                            // Guessed name
  bool mTookDamage : 1;                                                      // Guessed name
};
CHECK_SIZEOF(CAIMannedTurret, 0x618)

#endif // _CAIMANNEDTURRET
