#ifndef _CMETROID
#define _CMETROID

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

class CPlayer;
class CSpacePirate;
class CStateManager;

// Echoes layout recovered from the Metroid REL copy constructor (0xC8 bytes). Member names follow
// the generated SLdrMetroidAlpha record where it has one.
class CMetroidData {
public:
  CMetroidData(const CDamageVulnerability& frozenVulnerability,
               const CDamageVulnerability& energyDrainVulnerability,
               const CDamageVulnerability& babyMetroidGrowthVulnerability, float x90, float x94,
               float telegraphAttackTime, float babyMetroidScale, float xa0, float xa4, float xa8,
               CAssetId babyMetroidTransformationParticleEffect, float stage2GrowthScale,
               float stage2GrowthEnergy, float explosionGrowthEnergy, float dodgeCheckTimeInterval,
               float chanceToDodge, uint flags)
  : mFrozenVulnerability(frozenVulnerability)
  , mEnergyDrainVulnerability(energyDrainVulnerability)
  , mBabyMetroidGrowthVulnerability(babyMetroidGrowthVulnerability)
  , mEnergyDrainPerSecond(x90)
  , mMaxEnergyDrainAllowed(x94)
  , mTelegraphAttackTime(telegraphAttackTime)
  , mBabyMetroidScale(babyMetroidScale)
  , xa0_(xa0)
  , xa4_(xa4)
  , xa8_(xa8)
  , mBabyMetroidTransformationParticleEffect(babyMetroidTransformationParticleEffect)
  , mStage2GrowthScale(stage2GrowthScale)
  , mStage2GrowthEnergy(stage2GrowthEnergy)
  , mExplosionGrowthEnergy(explosionGrowthEnergy)
  , mDodgeCheckTimeInterval(dodgeCheckTimeInterval)
  , mChanceToDodge(chanceToDodge)
  , xc4_24_(flags & 1)
  , mStartsInWall((flags >> 1) & 1) {}

  CDamageVulnerability mFrozenVulnerability;
  CDamageVulnerability mEnergyDrainVulnerability;
  CDamageVulnerability mBabyMetroidGrowthVulnerability;
  float mEnergyDrainPerSecond;
  float mMaxEnergyDrainAllowed;
  float mTelegraphAttackTime;
  float mBabyMetroidScale;
  float xa0_;
  float xa4_;
  float xa8_;
  CAssetId mBabyMetroidTransformationParticleEffect;
  float mStage2GrowthScale;
  float mStage2GrowthEnergy;
  float mExplosionGrowthEnergy;
  float mDodgeCheckTimeInterval;
  float mChanceToDodge;
  bool xc4_24_ : 1;
  bool mStartsInWall : 1;
};
CHECK_SIZEOF(CMetroidData, 0xC8)

// Original class name from the Wii SEL exports (TypesMatch__8CMetroidCFi, TCastToPtr<8CMetroid>).
class CMetroid : public CPatterned {
public:
  CMetroid(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& mData, const CPatternedInfo& pInfo, const CActorParameters& aParms,
           const CMetroidData& metroidData);
  ~CMetroid() override;

  // CEntity
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool IsListening() const override { return true; }
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  void SetupStateMachine(CStateManager& mgr) override;
  bool CanBeIngPossessed(CStateManager& mgr) const override;

  // CMetroid
  virtual bool ShouldDodge(CStateManager& mgr, const CTriggerData& data) const;
  virtual void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dodge(CStateManager& mgr, EStateMsg msg, float dt);

  void OnDockTouch(CStateManager& mgr); // Guessed name.
  TUniqueId GetAttackTargetId() const { return mAttackTarget; }
  bool IsAttacking() const { return mIsAttacking; } // Guessed name.

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AttackOver(CStateManager& mgr, const CTriggerData& data) const;
  bool LostInterest(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  bool ShotAt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool InAttackPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool InPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InDetectionRange(CStateManager& mgr, const CTriggerData& data) const;
  bool SpotPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool AggressionCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTurn(CStateManager& mgr, const CTriggerData& data) const;
  bool Leash(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldWallHang(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Generate(CStateManager& mgr, EStateMsg msg, float dt);
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float dt);
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt);
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Attack(CStateManager& mgr, EStateMsg msg, float dt);
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt);

  // Code functions
  void SetTargetDest(CStateManager& mgr, float dt);
  void SetPatrolDest(CStateManager& mgr, float dt);

  bool IsSuckingEnergy() const;
  bool IsTargetGettingSucked(const CStateManager& mgr) const;
  bool IsPlayerInFluid(const CPlayer& player, const CStateManager& mgr) const;
  bool IsPirateValidTarget(const CSpacePirate& pirate) const;
  bool CanStartAttack(CStateManager& mgr) const;
  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void UpdateAILogicTimers(float dt, CStateManager& mgr);
  void SuckEnergyFromTarget(float dt, CStateManager& mgr);
  void PreventWorldCollisions(float dt, CStateManager& mgr);
  void RestoreSolidCollision(CStateManager& mgr);
  void DisableSolidCollision(CMetroid& target);
  void ApplyGrowth(float damage, CStateManager& mgr);
  bool ShouldReleaseFromTarget(CStateManager& mgr);
  void InterpolateToPosRot(CStateManager& mgr, float dt);
  void ComputeSuckTargetPosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const;
  void ComputeSuckPlayerPosRot(const CPlayer& player, CStateManager& mgr, CVector3f& pos,
                               CQuaternion& rot) const;
  void ComputeSuckPiratePosRot(CStateManager& mgr, CVector3f& pos, CQuaternion& rot) const;
  float ComputeMorphingPlayerSuckUpPos(const CPlayer& player) const;
  float GetDamageMultiplier() const;
  float GetGrowthStage() const;
  bool AttachToTarget(CStateManager& mgr);
  bool PreDamageSpacePirate(CStateManager& mgr);
  void UpdateAttackTarget(CStateManager& mgr);                             // Guessed name.
  TUniqueId FindNearestTarget(CStateManager& mgr, TUniqueId ignore) const; // Guessed name.
  void SelectNewTarget(CStateManager& mgr);                                // Guessed name.
  void ApplySeparationBehavior(CStateManager& mgr);
  CVector3f GetAttackTargetPos(const CStateManager& mgr) const;
  void SetupExitFaceHugDirection(CActor* actor, CStateManager& mgr, const CVector3f& direction,
                                 const CTransform4f& xf);
  void ApplyDamageGrowth(CStateManager& mgr, TUniqueId sender); // Guessed name.
  void DetachFromTarget(CStateManager& mgr, bool fromDock);     // Guessed second parameter.

protected:
  enum EAIState {
    kAiState_Invalid = -1,
    kAiState_Zero,
    kAiState_One,
    kAiState_Two,
    kAiState_Over,
  };

  CVector3f x7c0_;
  EAIState mState;
  float mAttackChance;
  int mAttackState;
  TUniqueId mTeamAiManagerId;
  pas::EStepDirection mDodgeDirection;
  CMetroidData mMetroidData;
  CCollidableSphere mCollisionPrimitive;
  CPathFindSearch mPathFindSearch;
  TUniqueId mAttackTarget;
  float mTelegraphAttackTime;
  float mEnergyDrained;
  float x9c0_;
  float x9c4_;
  CVector3f mScale1;
  CVector3f mScale2;
  CVector3f mScale3;
  float mGrowthDuration;
  float mGrowthEnergy;
  float mLastGrowthEnergy;
  float mSeekTime;
  float mMaxSeekTime;
  float mLoopAttackDistance;
  CVector3f mDetachPos;
  CDamageVulnerability mStandingFaceHugVulnerability;
  bool mAlert : 1;
  bool mGrowing : 1;
  bool mShotAt : 1;
  bool xa40_27_ : 1; // Set when leaving the wall-hang state.
  bool xa40_28_ : 1;
  bool mIsAttacking : 1;
  bool mRestoreSolidCollision : 1;
  bool mRestoreCharacterCollision : 1;
  bool mIsEnergyDrainVulnerable : 1;
};
CHECK_SIZEOF(CMetroid, 0xA48)

#endif // _CMETROID
