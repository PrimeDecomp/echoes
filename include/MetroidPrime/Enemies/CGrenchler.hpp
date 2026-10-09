#ifndef _CGRENCHLER
#define _CGRENCHLER

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CElectricDescription;
class CScannableObjectInfo;
class CScriptTeamAiMgr;
class CWeaponDescription;
class CScriptWater;
class CPFArea;
class CScannableObjectInfo;
class CScriptAiJumpPoint;
class CWeaponDescription;

// Guessed class: a tailed, bipedal-water enemy (also used as a grapple guardian).
class CGrenchler : public CPatterned {
public:
  // Guessed names; kind of attack queued in the attack pattern.
  enum EAttackType { kAT_Burst, kAT_Beam, kAT_Bite };

  // Guessed name; token holder shared by the attack descriptors.
  struct SParticleToken {
    SParticleToken(CAssetId id) : mId(id), mToken() {}
    CAssetId mId;
    rstl::optional_object< CToken > mToken;
  };

  // Guessed name; reflected-damage tracking record.
  struct SReflectInfo {
    SReflectInfo();
    void Reset();
    float x0_;
    CVector3f x4_;
    CVector3f x10_;
    bool x1c_0_ : 1;
    bool x1c_1_ : 1;
    bool x1c_2_ : 1;
    bool x1c_3_ : 1;
    bool x1c_4_ : 1;
    bool x1c_5_ : 1;
    CDamageVulnerability mVulnerability;
    float x50_;
  };

  // Guessed name; bite attack descriptor.
  struct SBiteAttack {
    SBiteAttack(const CDamageInfo& damage, float minRange, float maxRange, float minPause,
                float maxPause, float damageRadius);
    bool x0_;
    CDamageInfo mDamage;
    float mMinRange;
    float mMaxRange;
    float mMinPause;
    float mMaxPause;
    CTransform4f x30_;
    float mDamageRadius;
    float x64_;
  };

  // Guessed name; shared attack-descriptor header.
  struct SAttackBase {
    SAttackBase(const CDamageInfo& damage, float minRange, float maxRange, float minPause,
                float maxPause);
    CDamageInfo mDamage;
    float mMinRange;
    float mMaxRange;
    float mMinPause;
    float mMaxPause;
  };

  // Guessed name; beam attack descriptor.
  struct SBeamAttack : SAttackBase {
    SBeamAttack(const CDamageInfo& damage, const SLdrAudioPlaybackParms& sound, float minRange,
                float maxRange, float minPause, float maxPause, float maxAngle);
    bool x2c_;
    CDamageInfo mBeamDamage;
    float x4c_;
    CVector3f x50_;
    float mMaxAngle;
    SLdrAudioPlaybackParms mSound;
    CSfxHandle x78_;
    CTransform4f x7c_;
  };

  // Guessed name; burst attack descriptor.
  struct SBurstAttack : SAttackBase {
    SBurstAttack(CAssetId projectile, const CDamageInfo& damage, float minRange, float maxRange,
                 float minPause, float maxPause, float damageRadius);
    bool x2c_;
    CAssetId mProjectile;
    float x34_;
    float mDamageRadius;
    float x3c_;
  };

  // Guessed name; charge attack descriptor.
  struct SChargeAttack {
    SChargeAttack(float minTimeBetweenCharges, float unknown, float minRange, float maxRange);
    CVector3f x0_;
    CVector3f xc_;
    float x18_;
    float x1c_;
    float x20_;
    float x24_;
    float x28_;
    float x2c_;
    float x30_;
  };

  // Guessed name; grapple effect descriptor.
  struct SGrappleEffect {
    explicit SGrappleEffect(CAssetId id);
    int x0_;
    rstl::optional_object< TToken< CGenDescription > > x4_;
  };

  // Guessed name; particle generator holder.
  struct SOwnedParticle {
    SOwnedParticle() {}
    explicit SOwnedParticle(CElementGen* gen) : mGen(gen) {}
    rstl::auto_ptr< CElementGen > mGen;
  };

  // Guessed name; surface ring, electric and beam effects.
  struct SEffectA {
    SEffectA(CAssetId surfaceRings, CAssetId electric, CAssetId beam, CAssetId hitFx,
             CAssetId eyeGlow);
    uint x0_;
    SOwnedParticle x4_;
    uint xc_;
    CToken x10_;
    TUniqueId x18_;
    CToken x1c_;
    uint x24_;
    rstl::optional_object< TLockedToken< CGenDescription > > x28_;
    TUniqueId x38_;
    SOwnedParticle x3c_;
    float x44_;
  };

  // Guessed name; electric effect descriptor.
  struct SEffectB {
    SEffectB(CAssetId a, CAssetId b, float c);
    uchar x0_0_ : 1;
    uchar x0_1_ : 1;
    uchar x0_2_ : 1;
    int x4_;
    float x8_;
    int xc_;
    float x10_;
    float x14_;
    float x18_;
    float x1c_;
    float x20_;
    float x24_;
    int x28_;
    uint x2c_;
    TUniqueId x30_;
    rstl::optional_object< TToken< CGenDescription > > x34_;
  };

  // Guessed name; grapple beam descriptor.
  struct SEffectC {
    SEffectC(CAssetId swoosh, CAssetId beamPart, const CDamageInfo& damage,
             const SLdrAudioPlaybackParms& sound);
    CVector3f x0_;
    float xc_;
    uint x10_;
    rstl::optional_object< TLockedToken< CGenDescription > > x14_;
    SOwnedParticle x24_;
    uint x2c_;
    SOwnedParticle x30_;
    CVector3f x38_;
    float x44_;
    int x48_;
    CVector3f x4c_;
    CDamageInfo x58_;
    float x74_;
    SLdrAudioPlaybackParms x78_;
    CSfxHandle x90_;
  };

  // Guessed name; damage plus effect record.
  struct SDamageEffect {
    SDamageEffect(const CDamageInfo& damage, CAssetId effect);
    CDamageInfo mDamage;
    rstl::optional_object< TToken< CGenDescription > > mEffect;
  };

  // Guessed name; three-vector record.
  struct SVectorTriple {
    SVectorTriple();
    CVector3f x0_;
    CVector3f xc_;
    CVector3f x18_;
  };

  // Guessed name; particle effect with offsets.
  struct SEffectD {
    explicit SEffectD(CAssetId effect);
    SOwnedParticle x0_;
    CVector3f x8_;
    CVector3f x14_;
    float x20_;
  };

  CGrenchler(TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
             const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId fsmId,
             float tailDestroyedHealth, float minTimeBetweenCharges, float unknown_0x7bd1a35f,
             float chargeAttackMinRange, float chargeAttackMaxRange, float biteAttackMinRange,
             float biteAttackMaxRange, float biteAttackMinPause, bool isGrappleGuardian,
             bool hasHealthBar, const CDamageVulnerability& vulnerability, CAssetId tailAncs,
             int tailCharacter, int tailInitialAnim, uint tailWhenUnderwater,
             CAssetId taillessModel, CAssetId taillessSkinRules, CAssetId tailDarkAncs,
             int tailDarkCharacter, int tailDarkInitialAnim, uint tailWhenUnderwaterDark,
             CAssetId taillessModelDark, CAssetId taillessSkinRulesDark, ushort tailHitSound,
             ushort tailDestroyedSound, float biteAttackMaxPause, float biteAttackDamageRadius,
             const CDamageInfo& biteDamage, const CDamageInfo& beamDamage, float beamAttackMinRange,
             float beamAttackMaxRange, float beamAttackMinPause, float beamAttackMaxPause,
             float beamAttackMaxAngle, const SLdrAudioPlaybackParms& beamAttackSound,
             const CDamageInfo& burstDamage, CAssetId burstProjectile, float burstAttackMinRange,
             float burstAttackMaxRange, float burstAttackMinPause, float burstAttackMaxPause,
             float burstAttackDamageRadius, CAssetId surfaceRingsEffect, CAssetId shallowWaterRing,
             CAssetId electricEffect, CAssetId pART, CAssetId grappleSwoosh,
             CAssetId grappleBeamPart, CAssetId grappleHitFx, const CDamageInfo& grappleDamage,
             const SLdrAudioPlaybackParms& grappleBeamSound, CAssetId beamEffect,
             int unknown_0xd4753ff4, float unknown_0x05fc6001, float unknown_0x13e5b580,
             float unknown_0xfc6f199d, CAssetId grappleVisorEffect, const CDamageInfo& damageInfo,
             CAssetId pART_0x54b6bfa1, const SLdrAudioPlaybackParms& audioPlaybackParms,
             CAssetId grappleGuardianEyeGlow, CAssetId alternateScannableInfo,
             const CActorParameters& actorParams);

  // CEntity
  ~CGrenchler() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CVector3f GetScanObjectIndicatorPosition(const CStateManager& mgr) const override;
  void OnScanStateChange(EScanState state, CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  void TakeDamage(const CVector3f& direction, float magnitude) override;
  bool CanBeShot(const CStateManager& mgr, int weaponType) override { return true; }

  // CPatterned
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  CDamageInfo GetContactDamage() const override;
  bool CanBeUnPossessed(CStateManager& mgr) const override;

  // CGrenchler triggers
  virtual bool AbortCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Alerted(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Attacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool AttackPatternOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BeamBadAngle(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BeamHitPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BeamHitSticky(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BeamHitWall(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Bored(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool BreakGrappleLoop(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanBeamAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanBite(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanBurstAttack(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CancelManeuvering(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanCharge(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CanGrapple(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ChargeFinished(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ClearLineOfFire(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ClearPathToPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool CrystalDamaged(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool EmergedFromWater(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingJumpEnd(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool FacingPlayer(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ForceGrapple(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Frustrated(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool GrapplingMorphball(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool HasValidJumpTarget(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InBeamRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InBiteRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InBurstRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool InChargeRange(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool JumpLanded(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool JustBiteAttacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool JustBeamAttacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool JustBurstAttacked(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool JustHit(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ManeuverDone(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PauseOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerBehindMe(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerStuck(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayerSubmerged(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool PlayYellowHitReact(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ReturnToPatrol(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldBackstep(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldSlide(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool ShouldTurn(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SlideOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool SlideStop(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool StopStruggling(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool StruggleOver(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Stuck(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool Submerged(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TailIntact(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TookDamage(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TookKnockback(CStateManager& mgr, const CTriggerData& data) const;
  virtual bool TooMuchTurning(CStateManager& mgr, const CTriggerData& data) const;

  // CGrenchler states
  virtual void Backstep(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BeamAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BiteAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void BurstAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Charge(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ChargeFailed(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleAbort(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleBite(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleBreak(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleLoop(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrapplePull(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleSlide(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleSlideBonk(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void GrappleStruggle(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Maneuver(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void MorphballBite(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Null(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PauseBetweenBeams(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void PauseBetweenBites(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Pursue(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void ShakeOff(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void StopBeamAttack(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void Turn(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void TurnToJumpEnd(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void WalkTowardPlayer(CStateManager& mgr, EStateMsg msg, float dt);
  virtual void YellowHitReact(CStateManager& mgr, EStateMsg msg, float dt);

  // CGrenchler code functions
  virtual void PickJumpTarget(CStateManager& mgr, float dt);
  virtual void SetLastActionAsBeam(CStateManager& mgr, float dt);

  // Guessed names.
  void SetSubmerged(bool submerged);
  void DestroyGrappleBeam(CStateManager& mgr);
  void UpdateBeamTrace(CStateManager& mgr);
  CVector3f GetPlayerTargetPosition(CStateManager& mgr);
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const CBCMeleeAttackCmd& cmd);
  float GetTailHealth() const;

  // Guessed names
  void BeginMorphballCapture(CStateManager& mgr);
  void EndMorphballCapture(CStateManager& mgr);
  void UpdateCapturedPlayer(CStateManager& mgr);
  void UpdateDamageFlash(float dt);
  void IncrementAttached(CStateManager& mgr);
  void DecrementAttached(CStateManager& mgr);
  void SendAttachedMessage(CStateManager& mgr, EScriptObjectMessage msg);
  void PullPlayer(float dt, CStateManager& mgr, const CVector3f& targetPos);
  void RemoveExplosion(CStateManager& mgr);
  void UpdateExplosionHeight(CStateManager& mgr);
  void SpawnBeamHitEffect(CStateManager& mgr);                // Guessed name
  void PickGrappleSide(CStateManager& mgr);                   // Guessed name
  bool IsBeamBlockedByHint(CStateManager& mgr);               // Guessed name
  bool IsNearAvoidHint(CStateManager& mgr);                   // Guessed name
  void PlayTailHitSound();                                    // Guessed name
  void PlayTailDestroyedSound();                              // Guessed name
  void TurnToFaceTarget(CStateManager& mgr);                  // Guessed name
  bool IsNearPath(const CVector3f& pos, float padding) const; // Guessed name
  bool FindJumpTarget(CStateManager& mgr, bool ignoreRange);  // Guessed name
  float GetJumpCost(float weight, const CVector3f& jumpPos, const CVector3f& waypointPos,
                    const CVector3f& targetPos, bool skipEndCheck) const; // Guessed name
  template < typename T >
  void DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd); // Guessed name

  void CreateVisorEffect(CStateManager& mgr);                            // Guessed name
  CVector3f GetHornTargetPosition() const;                               // Guessed name
  void TurnToPlayer(CStateManager& mgr, float dt, float turnSpeed);      // Guessed name
  void ResetSteering();                                                  // Guessed name
  void UpdateChargeSteering();                                           // Guessed name
  bool IsDeeplySubmerged(CStateManager& mgr) const;                      // Guessed name
  bool IsFacingTarget(CStateManager& mgr, float angle) const;            // Guessed name
  CVector3f GetTargetPosition(CStateManager& mgr) const;                 // Guessed name
  bool IsDeeplySubmerged(CStateManager& mgr, const CActor& actor) const; // Guessed name
  float GetWaterSurfaceHeight(const CStateManager& mgr) const;           // Guessed name
  CVector3f GetPlayerTargetPosition(CStateManager& mgr) const;           // Guessed name
  int GetLastAttackState() const {
    return mCollisionActors.empty() ? xa74_ : mCollisionActors.back();
  } // Guessed name
  // Guessed name; the value of each state function that records itself via SetAttackState.
  enum EAction {
    kGA_Invalid = -1,
    kGA_Backstep = 0,
    kGA_BeamAttack = 1,
    kGA_BiteAttack = 2, // Also GrappleBite
    kGA_BurstAttack = 3,
    kGA_Charge = 4,
    kGA_GrappleLoop = 5,
    kGA_GrappleAbort = 6,
    kGA_GrappleBreak = 7,
    kGA_GrapplePull = 8,
    kGA_GrappleSlide = 9, // Also GrappleSlideBonk
    kGA_GrappleStruggle = 10,
    kGA_None = 11,
    kGA_Jump = 12,
    kGA_Lurk = 13,
    kGA_MorphballBite = 14,
    kGA_ShakeOff = 15,
    kGA_Taunt = 16,
    kGA_YellowHitReact = 17,
  };

  // CPatterned
  float GetFadeOnDeathTime() const override;
  void IssueDeathBodyCommand(CStateManager& mgr, const CVector3f& direction) override;

  static int CountAttacks(const rstl::reserved_vector< EAction, 3 >& history, EAction attack) {
    int count = 0;
    for (int i = 0; i < history.size(); ++i) {
      if (history[i] == attack) {
        ++count;
      }
    }
    return count;
  }
  static void PushAttackHistory(rstl::reserved_vector< EAction, 3 >& history, EAction attack);
  CVector3f GetManeuverTarget(CStateManager& mgr) const;
  float GetManeuverRangeWeight(float range, const CVector3f& a, const CVector3f& b) const;
  bool IsPointVisibleToPlayer(CStateManager& mgr, const CVector3f& point) const;
  bool HasClearShotAtPlayer(CStateManager& mgr, float maxAngle) const;
  CTeamAiRole::ETeamAiRole GetTeamRole(CStateManager& mgr) const;
  bool HasMeleeRole(CStateManager& mgr) const {
    bool hasRole = false;
    if (GetTeamRole(mgr) == CTeamAiRole::kTAR_Melee ||
        GetTeamRole(mgr) == CTeamAiRole::kTAR_Initial) {
      hasRole = true;
    }
    return hasRole;
  }
  void QuitTeam(CStateManager& mgr);
  void JoinTeam(CStateManager& mgr);
  CScriptTeamAiMgr* GetTeamAiMgr(CStateManager& mgr) const;
  const CScriptTeamAiMgr* GetTeamAiMgr(const CStateManager& mgr) const;
  void ApplyBiteDamage(CStateManager& mgr);
  void ApplyBurstDamage(CStateManager& mgr);
  void CreateGrappleBeam(CStateManager& mgr);

  void MoveToTarget(CStateManager& mgr, float dt, const CVector3f& target);
  void SetAttackState(EAction action, EStateMsg msg);
  void RemoveVisorEffect(CStateManager& mgr);
  // Collision joint table entry.
  struct SJointInfo {
    const char* mName;
    float mRadius;
    float mGuardianRadius;
    int mVulnerabilityGroup;
    EWeaponCollisionResponseTypes mResponseType;
    bool mUnknown;
  };

  void StartElectricBeam(CStateManager& mgr);
  void UpdateBeamTarget(CStateManager& mgr);
  void CreateDamageVisorEffect(CStateManager& mgr);
  void CreateGrappleHitVisorEffect(CStateManager& mgr);
  void ReleasePlayer(CStateManager& mgr);
  void GrabPlayer(CStateManager& mgr);
  void CreateCollisionManager(CStateManager& mgr);
  void AddJointCollisions(const SJointInfo* joints, int count,
                          rstl::vector< CJointCollisionDescription >& descriptions) const;

  void SetActionState(int action, EStateMsg msg);
  CVector3f GetPlayerTargetPosition(const CStateManager& mgr) const;
  bool IsFacingTarget(const CStateManager& mgr, float angle) const;
  bool IsPlayerLookingAtMe(const CStateManager& mgr) const;
  CVector3f GetPlayerBeamTargetPosition(const CStateManager& mgr) const;
  bool IsDeeplySubmerged(const CStateManager& mgr, const CActor& actor) const;
  bool IsDeeplySubmerged(const CStateManager& mgr) const;
  void SetupBodyVulnerabilities(CStateManager& mgr, const CDamageVulnerability& vulnerability);
  void ResetBodyVulnerabilities(CStateManager& mgr, const CDamageVulnerability& vulnerability);
  float GetVulnerableAngle() const;
  void RestoreDefaultJointVulnerability(CStateManager& mgr);
  void StartHitReaction(bool enable);

  // Guessed names.
  void NotifyFalling(CStateManager& mgr, const TUniqueId& id);
  void LaunchToJumpTarget();
  void SetGroundCollision(CStateManager& mgr, EStateMsg msg);
  void EnableGroundCollision(CStateManager& mgr, bool onGround);
  void DisableGroundCollision(CStateManager& mgr);
  bool IsGuardianCharging() const;
  void ResolveCollision(CStateManager& mgr);
  bool IsPlayerLookingAtMe(CStateManager& mgr) const;
  bool IsFacing(const CVector3f& position, float maxAngle) const;
  bool IsNearAvoidHint(CStateManager& mgr) const;
  void UnmarkPathRegion(CStateManager& mgr);
  void SetupStateMachineHelper(CStateManager& mgr); // Guessed name
  const CGenericFSM2* GetStateMachine() const;      // Guessed name
  float GetWaterSurfaceHeight(CStateManager& mgr);  // Guessed name
  void PreRenderBoneTracking(CStateManager& mgr);   // Guessed name
  CVector3f GetBlendedLocatorPosition(float height, const char* firstLocator,
                                      const char* secondLocator) const; // Guessed name
  void ThinkBoneTracking(float dt, CStateManager& mgr);                 // Guessed name
  bool ShouldTrackPlayer() const;                                       // Guessed name
  CVector3f GetPlayerBeamTargetPosition(CStateManager& mgr) const;      // Guessed name
  void UpdateEffects(float dt, CStateManager& mgr);                     // Guessed name
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  bool IsWadingInWater(CStateManager& mgr);               // Guessed name
  void UpdateFacingDirection();                           // Guessed name
  void UpdateGuardianBlend(float dt, CStateManager& mgr); // Guessed name
  void UpdateTeammateScan(float dt, CStateManager& mgr);  // Guessed name
  void UpdateMovement(float dt, CStateManager& mgr);      // Guessed name
  bool CanCrystalTakeDamage() const;
  void StopElectricBeam(CStateManager& mgr);
  CPFArea* GetPathArea(CStateManager& mgr) const;                         // Guessed name
  void SetPathArea(CStateManager& mgr);                                   // Guessed name
  void MarkPathRegion(CStateManager& mgr);                                // Guessed name
  bool InRange(CStateManager& mgr, float minRange, float maxRange) const; // Guessed name
  void SpawnTail(CStateManager& mgr);                                     // Guessed name
  void DestroyTail(CStateManager& mgr);                                   // Guessed name
  CScriptAiJumpPoint* FindJumpPoint(CStateManager& mgr, const CVector3f& target,
                                    TUniqueId exclude); // Guessed name
  float GetFacingAngleDiff(const CTransform4f& xf) const;
  bool IsCrystalActor(TUniqueId id);
  bool IsBodyActor(TUniqueId id);
  void ResetBodyVulnerabilities(CStateManager& mgr);
  // Guessed name; weak spot in front of the head, reflective everywhere else.
  class SConeVulnerability : public CNonUniformVulnerability {
  public:
    SConeVulnerability(const CVector3f& direction, float maxAngle,
                       EWeaponCollisionResponseTypes insideResponse,
                       EWeaponCollisionResponseTypes outsideResponse)
    : mDirection(direction)
    , mMaxAngle(maxAngle)
    , mInsideResponse(insideResponse)
    , mOutsideResponse(outsideResponse)
    , mResponse(mInsideResponse) {}

    void SetDirection(const CVector3f& direction) { mDirection = direction; }

    const CDamageVulnerability* GetDamageVulnerability(const CDamageVulnerability* vulnerability,
                                                       const CVector3f& point,
                                                       const CVector3f& direction,
                                                       const CDamageInfo& info) override;
    bool GetCollisionResponseType(const CVector3f& point, const CVector3f& direction,
                                  const CWeaponMode& mode, int attributes,
                                  EWeaponCollisionResponseTypes& response) override;

  private:
    CVector3f mDirection;
    float mMaxAngle;
    EWeaponCollisionResponseTypes mInsideResponse;
    EWeaponCollisionResponseTypes mOutsideResponse;
    EWeaponCollisionResponseTypes mResponse;
  };

private:
  CPathFindSearch mPathFindSearch;                                                  // 0x7c0
  float x8ac_;                                                                      // Guessed name
  CVector3f x8b0_;                                                                  // Guessed name
  float x8bc_;                                                                      // Guessed name
  CBoneTracking mBoneTracking;                                                      // Guessed name
  float x8fc_;                                                                      // Guessed name
  float x900_;                                                                      // Guessed name
  float x904_;                                                                      // Guessed name
  int x908_;                                                                        // Guessed name
  bool mX90c_0_ : 1;                                                                // Guessed name
  bool mX90c_1_ : 1;                                                                // Guessed name
  bool mIsGrappleGuardian : 1;                                                      // Guessed name
  bool mHasHealthBar : 1;                                                           // Guessed name
  bool mX90c_4_ : 1;                                                                // Guessed name
  bool mX90c_5_ : 1;                                                                // Guessed name
  bool mX90c_6_ : 1;                                                                // Guessed name
  CSurfaceAlignmentHelper mSurfaceAlignment;                                        // Guessed name
  rstl::single_ptr< TLockedToken< CScannableObjectInfo > > mAlternateScanInfo;      // Guessed name
  CDamageVulnerability mDamageVulnerability;                                        // Guessed name
  rstl::ncrc_ptr< CNonUniformVulnerability > x99c_;                                 // Guessed name
  rstl::optional_object< CAABox > mGuardianBounds;                                  // Guessed name
  rstl::optional_object< CToken > mFsm;                                             // Guessed name
  CAssetId mTaillessModel;                                                          // Guessed name
  CAssetId mTaillessSkinRules;                                                      // Guessed name
  rstl::optional_object< TLockedToken< CSkinnedModel > > mTaillessSkinnedModel;     // Guessed name
  CAssetId mTaillessModelDark;                                                      // Guessed name
  CAssetId mTaillessSkinRulesDark;                                                  // Guessed name
  rstl::optional_object< TLockedToken< CSkinnedModel > > mTaillessSkinnedModelDark; // Guessed name
  bool x9fc_;                                                                       // Guessed name
  float xa00_;                                                                      // Guessed name
  mutable TUniqueId xa04_;                                                          // Guessed name
  mutable bool xa06_;                                                               // Guessed name
  SReflectInfo mReflectInfo;                                                        // Guessed name
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;                     // Guessed name
  rstl::reserved_vector< EAction, 3 > mCollisionActors;                             // Guessed name
  int xa70_;                                                                        // Guessed name
  EAction xa74_;                                                                    // Guessed name
  EAction xa78_;                                                                    // Guessed name
  float xa7c_;                                                                      // Guessed name
  float xa80_;                                                                      // Guessed name
  float xa84_;                                                                      // Guessed name
  float xa88_;                                                                      // Guessed name
  float xa8c_;                                                                      // Guessed name
  float xa90_;                                                                      // Guessed name
  float xa94_;                                                                      // Guessed name
  SBiteAttack mBiteAttack;                                                          // Guessed name
  SBeamAttack mBeamAttack;                                                          // Guessed name
  SBurstAttack mBurstAttack;                                                        // Guessed name
  rstl::optional_object< CModelData > mTaillessModelData;                           // Guessed name
  CTransform4f xc3c_;                                                               // Guessed name
  int xc6c_;                                                                        // Guessed name
  float xc70_;                                                                      // Guessed name
  float mTailHealth;                                                                // Guessed name
  float xc78_;                                                                      // Guessed name
  ushort mTailHitSound;                                                             // Guessed name
  ushort mTailDestroyedSound;                                                       // Guessed name
  uchar xc80_;                                                                      // Guessed name
  SChargeAttack mChargeAttack;                                                      // Guessed name
  float xcb8_;                                                                      // Guessed name
  uchar xcbc_;                                                                      // Guessed name
  float xcc0_;                                                                      // Guessed name
  float xcc4_;                                                                      // Guessed name
  int xcc8_;                                                                        // Guessed name
  TUniqueId xccc_;                                                                  // Guessed name
  CVector3f xcd0_;                                                                  // Guessed name
  float xcdc_;                                                                      // Guessed name
  float xce0_;                                                                      // Guessed name
  uchar xce4_;                                                                      // Guessed name
  float xce8_;                                                                      // Guessed name
  float xcec_;                                                                      // Guessed name
  float xcf0_;                                                                      // Guessed name
  uchar xcf4_;                                                                      // Guessed name
  SGrappleEffect mGrappleEffect;                                                    // Guessed name
  SEffectA mEffectA;                                                                // Guessed name
  SEffectB mEffectB;                                                                // Guessed name
  SEffectC mEffectC;                                                                // Guessed name
  float xe24_;                                                                      // Guessed name
  float xe28_;                                                                      // Guessed name
  CVector3f xe2c_;                                                                  // Guessed name
  CAnimationParameters mTail;                                                       // Guessed name
  uint mTailWhenUnderwater;                                                         // Guessed name
  CAnimationParameters mTailDark;                                                   // Guessed name
  uint mTailWhenUnderwaterDark;                                                     // Guessed name
  float xe58_;                                                                      // Guessed name
  float xe5c_;                                                                      // Guessed name
  float xe60_;                                                                      // Guessed name
  float xe64_;                                                                      // Guessed name
  float xe68_;                                                                      // Guessed name
  int xe6c_;                                                                        // Guessed name
  CTransform4f xe70_;                                                               // Guessed name
  uchar mXea0_0_ : 1;                                                               // Guessed name
  uchar mXea0_1_ : 1;                                                               // Guessed name
  uchar mXea0_2_ : 1;                                                               // Guessed name
  float xea4_;                                                                      // Guessed name
  bool mXea8_0_ : 1;                                                                // Guessed name
  bool mXea8_1_ : 1;                                                                // Guessed name
  int xeac_;                                                                        // Guessed name
  SLdrAudioPlaybackParms mAudioPlaybackParms;                                       // Guessed name
  CSfxHandle xec8_;                                                                 // Guessed name
  SDamageEffect mDamageEffect;                                                      // Guessed name
  SVectorTriple mVectors;                                                           // Guessed name
  pas::ETauntType xf18_;                                                            // Guessed name
  float xf1c_;                                                                      // Guessed name
  TUniqueId xf20_;                                                                  // Guessed name
  SEffectD mEffectD;                                                                // Guessed name
};
CHECK_SIZEOF(CGrenchler, 0xf48)

#endif // _CGRENCHLER
