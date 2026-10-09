#ifndef _CBLOGG
#define _CBLOGG

#include "types.h"

#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CBlogg;
struct SLdrBloggStruct;
class CCollisionActorManager;
class CPlayer;

// Guessed class: the vulnerability of the collision actors on the body of a Blogg.
class CBloggBodyVulnerability : public CNonUniformVulnerability {
public:
  CBloggBodyVulnerability(const CDamageVulnerability& vulnerability,
                          const CDamageVulnerability& otherVulnerability, bool flag);

  // CNonUniformVulnerability
  ~CBloggBodyVulnerability() override {}
  const CDamageVulnerability* GetDamageVulnerability(const CDamageVulnerability* defaultVuln,
                                                     const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  bool GetCollisionResponseType(const CVector3f& position, const CVector3f& direction,
                                const CWeaponMode& mode, int attributes,
                                EWeaponCollisionResponseTypes& response) const override;

  void SetOwner(CBlogg* owner) { mOwner = owner; }

protected:
  CDamageVulnerability mVulnerability;      // Guessed name
  CDamageVulnerability mOtherVulnerability; // Guessed name
  CBlogg* mOwner;                           // Guessed name
  bool mFlag;                               // Guessed name
};

// Guessed class: the vulnerability of the collision actors around the mouth of a Blogg.
class CBloggMouthVulnerability : public CBloggBodyVulnerability {
public:
  CBloggMouthVulnerability(const CDamageVulnerability& vulnerability,
                           const CDamageVulnerability& otherVulnerability, bool flag);

  // CNonUniformVulnerability
  ~CBloggMouthVulnerability() override {}
  const CDamageVulnerability* GetDamageVulnerability(const CDamageVulnerability* defaultVuln,
                                                     const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  bool GetCollisionResponseType(const CVector3f& position, const CVector3f& direction,
                                const CWeaponMode& mode, int attributes,
                                EWeaponCollisionResponseTypes& response) const override;

private:
  mutable EWeaponCollisionResponseTypes mResponseType; // Guessed name
};

// Guessed class: a health phase of a Blogg with a range of values and three tuning floats.
struct SBloggPhaseData {
  SBloggPhaseData(const SLdrBloggStruct& data);

  uchar mMin;      // Guessed name
  uchar mMax;      // Guessed name
  float mUnknownA; // Guessed name
  float mUnknownB; // Guessed name
  float mUnknownC; // Guessed name
};

// Guessed class: a large swimming enemy that charges, bites and spits the morph ball.
class CBlogg : public CPatterned {
public:
  // Guessed names; the value written to the state tag by each state function.
  enum EMaterialAction { kMA_Add, kMA_Remove }; // Guessed names

  enum EBloggState {
    kBS_Patrol,
    kBS_MoveToAttackPosition,
    kBS_MoveToValidPosition,
    kBS_FacePlayer,
    kBS_ProjectileAttack,
    kBS_ChargeTelegraph,
    kBS_ChargeAttack,
    kBS_MeleeAttack,
    kBS_Stunned,
    kBS_Taunt,
    kBS_MoveToPlayer,
    kBS_GrabBall,
    kBS_Thrash,
    kBS_SpitBall,
    kBS_Dead,
    kBS_Dying
  };

  CBlogg(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
         const CModelData& modelData, const CPatternedInfo& patternedInfo,
         const CActorParameters& actorParams, float minAttackAngle, float maxAttackAngle,
         float minProjectileDelay, float maxProjectileDelay, uchar unknown_0xa19d5f62,
         CAssetId projectileParticleEffect, const CDamageInfo& projectileDamage,
         const CDamageVulnerability& armorVulnerability, float bodyDamageMultiplier,
         float mouthDamageMultiplier, float mouthDamageAngle, float chargeDamageRadius,
         float chargeDamage, float biteDamage, float ballSpitDamage, float fishAttractionRadius,
         float fishAttractionPriority, float aggressiveness, float unknown_0x479ccc37,
         float unknown_0x689a803f, float unknown_0x800a2b0d, float chargeTurnSpeed,
         float chargeSpeedMultiplier, float maxMeleeRange, float maxBallDetectionRange,
         float maxPlayerPursuitTime, float maxBallPursuitTime, ushort mouthOpenSound,
         float minDelayBetweenMeleeAttacks, float maxCollisionTime, bool isMegaBlogg,
         float projectileBlurRadius, float projectileBlurTime,
         const CDamageVulnerability& ingPossessedArmorVulnerability, const SBloggPhaseData& phase0,
         const SBloggPhaseData& phase1, const SBloggPhaseData& phase2);

  // CEntity
  ~CBlogg() override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  // CActor
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;

  // CPatterned
  CProjectileInfo* ProjectileInfo() override { return &mProjectileInfo; }
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CDamageInfo GetContactDamage() const override { return mContactDamage; }
  void SetupStateMachine(CStateManager& mgr) override;
  void SetIngPossessed(bool possessed, CStateManager& mgr) override;
  void SetIngPossessed(bool possessed, float duration, CStateManager& mgr) override;
  CVector3f GetIngSnatchingNormal(float t) const override;
  CVector3f GetIngSnatchingPoint(float t) const override;

  // CBlogg
  const CDamageVulnerability* GetIngPossessedArmorVulnerability() const {
    return &mIngPossessedArmorVulnerability;
  } // Guessed name
  bool IsHitInMouthDirection(const CVector3f& direction) const; // Guessed name
  bool IsMouthClosed() const { return mMouthClosed != 0; }      // Guessed name
  void ComputeTauntProbability(CStateManager& mgr, float dt);
  void EndMeleePursuit(CStateManager& mgr, float dt);

  void MoveToValidPosition(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveToAttackPosition(CStateManager& mgr, EStateMsg msg, float dt);
  void FacePlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void ChargeTelegraph(CStateManager& mgr, EStateMsg msg, float dt);
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Stunned(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveToPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  void GrabBall(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void ChargeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Thrash(CStateManager& mgr, EStateMsg msg, float dt);
  void SpitBall(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);

  bool ShouldPatrol(CStateManager& mgr, const CTriggerData& data) const;
  bool AnimOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldPrepareToAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool IsPlayerStunned(CStateManager& mgr, const CTriggerData& data) const;
  bool CollidedWithWall(CStateManager& mgr, const CTriggerData& data) const;
  bool CanBitePlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool InMeleeRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InBiteRange(CStateManager& mgr, const CTriggerData& data) const;
  bool InProjectileRange(CStateManager& mgr, const CTriggerData& data) const;
  bool CantMoveToPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool BallGrabbed(CStateManager& mgr, const CTriggerData& data) const;
  bool CanGrabBall(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAbortBallGrab(CStateManager& mgr, const CTriggerData& data) const;
  bool InAttackPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool InValidPosition(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  bool ProjectileAttackDelay(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldCharge(CStateManager& mgr, const CTriggerData& data) const;
  bool IsChargeOver(CStateManager& mgr, const CTriggerData& data) const;
  bool CanMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool CanRangedAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool CanTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEndPursuit(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEndBallPursuit(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerInBallMode(CStateManager& mgr, const CTriggerData& data) const;
  bool DetectBall(CStateManager& mgr, const CTriggerData& data) const;
  bool IsPlayerReachable(CStateManager& mgr, const CTriggerData& data) const;

private:
  CPlayer* GetPlayer(CStateManager& mgr) const;                              // Guessed name
  bool IsPlayerWithin(CStateManager& mgr, float distance) const;             // Guessed name
  uchar HasCollisionTimeElapsed() const;                                     // Guessed name
  bool IsAtAttackPosition() const;                                           // Guessed name
  bool IsPlayerWithinChargeRange(CStateManager& mgr, CPlayer* player) const; // Guessed name

  CVector3f GetDirectionToPlayer(CStateManager& mgr) const;                     // Guessed name
  void FindFluid(CStateManager& mgr, CAABox& bounds, TUniqueId& waterId) const; // Guessed name
  bool IsInhabitingFluid(CStateManager& mgr, TUniqueId waterId,
                         TUniqueId uid) const;                    // Guessed name
  bool CanReachPlayer(CStateManager& mgr, CPlayer* player) const; // Guessed name
  int GetHealthPhase() const;                                     // Guessed name
  void ChoosePhaseValue(CStateManager& mgr);                      // Guessed name
  void SyncCollisionActorHealth(CStateManager& mgr);              // Guessed name
  void LeaveTeam(CStateManager& mgr);                             // Guessed name
  void JoinTeam(CStateManager& mgr);                              // Guessed name
  void ApplyContactDamage(CStateManager& mgr, CPlayer& player,
                          const CDamageInfo& damage);  // Guessed name
  bool IsPlayerInMouthRange(CStateManager& mgr) const; // Guessed name
  void ApplyCollisionActorDamage(CStateManager& mgr, const TUniqueId& senderId,
                                 float multiplier); // Guessed name
  void ReleaseHints(CStateManager& mgr);            // Guessed name
  TUniqueId FindNearestHint(CStateManager& mgr, const CVector3f& position,
                            bool checkLineOfSight) const; // Guessed name
  void CollectHints(CStateManager& mgr);                  // Guessed name
  void UpdateCollisionActorMaterials(CStateManager& mgr, const CMaterialList& materials,
                                     EMaterialAction action); // Guessed name
  void StopPlayer(CStateManager& mgr);                        // Guessed name
  void AttachPlayerToMouth(CStateManager& mgr);               // Guessed name
  uchar GetNextPositionIndex() const;                         // Guessed name
  void PathToAttackPosition(CStateManager& mgr, float dt);    // Guessed name

  EBloggState mState; // Guessed name
  int mAimAnimLeft;   // Guessed name
  int mAimAnimRight;  // Guessed name
  int mAimAnimUp;     // Guessed name
  int mAimAnimDown;   // Guessed name
  int x7d4_;
  int mUnknown26Anim;                                                // Guessed name
  float mAimWeightLeft;                                              // Guessed name
  float mAimWeightRight;                                             // Guessed name
  float mAimWeightUp;                                                // Guessed name
  float mAimWeightDown;                                              // Guessed name
  CVector3f mLastForward;                                            // Guessed name
  CVector3f mTargetForward;                                          // Guessed name
  CDamageInfo mContactDamage;                                        // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager; // Guessed name
  float mMinAttackAngle;                                             // Guessed name
  float mMaxAttackAngle;                                             // Guessed name
  float mMinAttackRange;                                             // Guessed name
  float mMaxAttackRange;                                             // Guessed name
  float mCurrentAttackAngle;                                         // Guessed name
  float mCurrentAttackRange;                                         // Guessed name
  CVector3f mAttackPosition;                                         // Guessed name
  TUniqueId mHintId;                                                 // Guessed name
  CPathFindSearch mPathFindSearch;                                   // Guessed name
  CVector3f mProjectileDirection;                                    // Guessed name
  float mMinProjectileDelay;                                         // Guessed name
  float mMaxProjectileDelay;                                         // Guessed name
  float mProjectileDelay;                                            // Guessed name
  uchar mUnknown_0xa19d5f62;                                         // Guessed name
  uchar x951_;
  CProjectileInfo mProjectileInfo; // Guessed name
  CVector3f x97c_;
  float mBaseTurnSpeed;                                    // Guessed name
  float mProjectileScale;                                  // Guessed name
  int mMouthClosed;                                        // Guessed name
  TUniqueId mPlayerId;                                     // Guessed name
  TUniqueId mTeamManagerId;                                // Guessed name
  rstl::reserved_vector< CVector3f, 16 > mPositionHistory; // Guessed name
  float mBodyDamageMultiplier;                             // Guessed name
  float mMouthDamageMultiplier;                            // Guessed name
  CDamageVulnerability mArmorVulnerability;                // Guessed name
  CDamageVulnerability mIngPossessedArmorVulnerability;    // Guessed name
  TUniqueId xac4_;
  float mMouthDamageAngle;       // Guessed name
  float mChargeDamageRadius;     // Guessed name
  float mChargeDamage;           // Guessed name
  float mChargeTurnSpeed;        // Guessed name
  float mChargeSpeedMultiplier;  // Guessed name
  float mBiteDamage;             // Guessed name
  float mBallSpitDamage;         // Guessed name
  float mMaxMeleeRange;          // Guessed name
  float mMaxBallDetectionRange;  // Guessed name
  float mMaxPlayerPursuitTime;   // Guessed name
  float mMaxBallPursuitTime;     // Guessed name
  float mBallPursuitTime;        // Guessed name
  float mPlayerPursuitTime;      // Guessed name
  float mFishAttractionRadius;   // Guessed name
  float mFishAttractionPriority; // Guessed name
  float mAggressiveness;         // Guessed name
  float mUnknown_0x479ccc37;     // Guessed name
  float mUnknown_0x689a803f;     // Guessed name
  float mUnknown_0x800a2b0d;     // Guessed name
  int xb14_;
  uchar xb18_;
  float mCollisionTime;                                           // Guessed name
  float mMaxCollisionTime;                                        // Guessed name
  float mBallGrabTime;                                            // Guessed name
  float mLocomotionChangeTimer;                                   // Guessed name
  float mLocomotionChangeInterval;                                // Guessed name
  ushort mMouthOpenSound;                                         // Guessed name
  float mBaseSpeed;                                               // Guessed name
  rstl::vector< TUniqueId > mHintIds;                             // Guessed name
  float mMeleeDelayTimer;                                         // Guessed name
  float mMinDelayBetweenMeleeAttacks;                             // Guessed name
  rstl::ncrc_ptr< CBloggMouthVulnerability > mMouthVulnerability; // Guessed name
  rstl::ncrc_ptr< CBloggBodyVulnerability > mBodyVulnerability;   // Guessed name
  float mProjectileBlurRadius;                                    // Guessed name
  float mProjectileBlurTime;                                      // Guessed name
  float xb68_;
  float xb6c_;
  CLineOfSightTracker mLineOfSightTracker; // Guessed name
  uchar mPhaseValue;                       // Guessed name
  uchar xbb1_;
  uchar xbb2_;
  rstl::vector< SBloggPhaseData > mPhases; // Guessed name
  bool xbc4_24_ : 1;
  bool xbc4_25_ : 1;
  bool mCanBite : 1; // Guessed name
  bool xbc4_27_ : 1;
  bool mBallGrabbed : 1;       // Guessed name
  bool mTauntReady : 1;        // Guessed name
  bool mChargeOver : 1;        // Guessed name
  bool mIsMegaBlogg : 1;       // Guessed name
  bool mMeleePursuitEnded : 1; // Guessed name
  bool mAbortBallGrab : 1;     // Guessed name
  bool xbc5_26_ : 1;
  bool xbc5_27_ : 1;
  bool xbc5_28_ : 1;
  bool xbc5_29_ : 1;
  bool xbc5_30_ : 1;
};
CHECK_SIZEOF(CBlogg, 0xBC8)

#endif // _CBLOGG
