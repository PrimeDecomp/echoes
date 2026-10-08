#ifndef _CPUDDLESPORE
#define _CPUDDLESPORE

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"

class CCollisionActorManager;

// Guessed name: the Puddle Spore tuning values the loader hands to the constructor.
struct SPuddleSporeData {
  SPuddleSporeData(float chargeTime, float timeOpen, float platformTime, float unknown,
                   float knockOffForce, float hitDetectionSine, float shockWaveHeight,
                   ushort shockWaveSound, const CShockWaveInfo& shockWaveInfo)
  : mChargeTime(chargeTime)
  , mTimeOpen(timeOpen)
  , mPlatformTime(platformTime)
  , mUnknown(unknown)
  , mKnockOffForce(knockOffForce)
  , mHitDetectionSine(hitDetectionSine)
  , mShockWaveHeight(shockWaveHeight)
  , mShockWaveSound(shockWaveSound)
  , mShockWaveInfo(shockWaveInfo) {}

  float mChargeTime;
  float mTimeOpen;
  float mPlatformTime;
  float mUnknown;
  float mKnockOffForce;
  float mHitDetectionSine;
  float mShockWaveHeight;
  ushort mShockWaveSound;
  CShockWaveInfo mShockWaveInfo;
};
CHECK_SIZEOF(SPuddleSporeData, 0x5c)

// Guessed class: a spore plant that opens to fire a shock wave and doubles as a platform.
class CPuddleSpore : public CPatterned {
public:
  enum EPuddleState { kPS_Closed, kPS_Open, kPS_Platform }; // Guessed names

  CPuddleSpore(TUniqueId uid, const rstl::string& name, EFlavorType flavor, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& modelData,
               const CPatternedInfo& patternedInfo, EColliderType collider,
               const CActorParameters& actorParams, const SPuddleSporeData& data);

  // CEntity
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CPuddleSpore
  virtual bool InAttackPosition(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldTurn(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual void InActive(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Active(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Run(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TurnAround(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GetUp(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Attack(CStateManager& mgr, EStateMsg msg, float dt);

private:
  void SetupCollisionManager(CStateManager& mgr);                                    // Guessed name
  void UpdateEffects(CStateManager& mgr);                                            // Guessed name
  void SetState(CStateManager& mgr, EPuddleState state);                             // Guessed name
  void SetCollisionActive(CStateManager& mgr, bool active);                          // Guessed name
  void UpdatePlayerContact(float dt, CStateManager& mgr);                            // Guessed name
  void KnockOffPlayers(float force, CStateManager& mgr);                             // Guessed name
  bool IsOpen() const;                                                               // Guessed name
  bool WeaponHitsWeakSpot(const CVector3f& position, const CWeaponMode& mode) const; // Guessed name

  SPuddleSporeData mData;                                            // Guessed name
  float mStateTimer;                                                 // Guessed name
  float mSecondaryStateTimer;                                        // Guessed name
  CAABox mTouchBounds;                                               // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  CVector3f mAimPosition;                                            // Guessed name
  EPuddleState mState;                                               // Guessed name
  int mAnimPhase;                                                    // Guessed name
  TUniqueId mShockWaveId;                                            // Guessed name
  CSfxHandle mShockWaveSfx;                                          // Guessed name
  bool mStateTimerRunning : 1;                                       // Guessed name
  bool mSecondaryTimerRunning : 1;                                   // Guessed name
  bool mOpen : 1;                                                    // Guessed name
};
CHECK_SIZEOF(CPuddleSpore, 0x860)

#endif // _CPUDDLESPORE
