#ifndef _CMEDIUMING
#define _CMEDIUMING

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/ScriptLoader/SLdrMediumIng.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGameLight;

// Guessed class: the properties of the Medium Ing that its tentacles also use.
class CMediumIngData {
public:
  CMediumIngData(const CDamageVulnerability& mistingVulnerability, CAssetId tentacleModel,
                 int tentacleCharacterIndex, const CActorParameters& tentacleActorParameters,
                 int spawnMode, float aggressiveness, const CColor& lightColor,
                 float lightAttenuation, const CDamageInfo& mistDamage, float maxMistAttackRange,
                 const CDamageInfo& meleeDamage, float maxMeleeAttackRange, float minArmAttackRange,
                 float maxArmAttackRange, const CMayaSpline& attackMotion,
                 const CCameraShakerData& attackTentacleImpact,
                 const CDamageInfo& attackTentacleDamage, float noMistDamageThreshold,
                 float tauntChance, const CMayaSpline& unknownSpline, const CMayaSpline& dashSpeed,
                 CAssetId ingSpotBlobFx, float doubleDashChance, float minMistAttackInterval,
                 float minArmAttackInterval, float minTentacleLength, float maxTentacleLength,
                 float armAttackTime, float unknown8f1d597c, float minMeleeAttackInterval,
                 float unknown0e3d3708, ushort ingSpotSound);
  CMediumIngData(const CMediumIngData& other);
  ~CMediumIngData() {}

  CDamageVulnerability mMistingVulnerability; // Guessed name
  CAssetId mTentacleModel;                    // Guessed name
  int mTentacleCharacterIndex;                // Guessed name
  CActorParameters mTentacleActorParameters;  // Guessed name
  int mSpawnMode;                             // Guessed name
  float mAggressiveness;                      // Guessed name
  CColor mLightColor;                         // Guessed name
  float mLightAttenuation;                    // Guessed name
  CAssetId mIngSpotBlobFx;                    // Guessed name
  ushort mIngSpotSound;                       // Guessed name
  float mMaxMistAttackRange;                  // Guessed name
  CDamageInfo mMistDamage;                    // Guessed name
  float mMaxMeleeAttackRange;                 // Guessed name
  CDamageInfo mMeleeDamage;                   // Guessed name
  float mMinArmAttackRange;                   // Guessed name
  float mMaxArmAttackRange;                   // Guessed name
  CMayaSpline mAttackMotion;                  // Guessed name
  CCameraShakerData mAttackTentacleImpact;    // Guessed name
  CDamageInfo mAttackTentacleDamage;          // Guessed name
  CMayaSpline mUnknownSpline;                 // Guessed name
  CMayaSpline mDashSpeed;                     // Guessed name
  float mNoMistDamageThreshold;               // Guessed name
  float mTauntChance;                         // Guessed name
  float mDoubleDashChance;                    // Guessed name
  float mMinMistAttackInterval;               // Guessed name
  float mMinArmAttackInterval;                // Guessed name
  float mMinTentacleLength;                   // Guessed name
  float mMaxTentacleLength;                   // Guessed name
  float mArmAttackTime;                       // Guessed name
  float mUnknown8f1d597c;                     // Guessed name
  float mMinMeleeAttackInterval;              // Guessed name
  float mSafeZoneRangeSq;                     // Guessed name
};
CHECK_SIZEOF(CMediumIngData, 0x300)

// Guessed class: a tentacle that the Medium Ing extends towards its target.
class CMediumIngTentacle : public CActor {
public:
  CMediumIngTentacle(TUniqueId uid, TAreaId areaId, const rstl::string& name,
                     const CModelData& modelData, const CMediumIngData& data);

  // CEntity
  ~CMediumIngTentacle() override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CMediumIngTentacle
  void Extend(CStateManager& mgr, const CVector3f& target); // Guessed name
  void Retract(CStateManager& mgr);                         // Guessed name
  float GetProgress() const;                                // Guessed name

private:
  void UpdateShake(CStateManager& mgr, float dt); // Guessed name

  CMotionSpline mSpline;                      // Guessed name
  CMayaSpline mAttackMotion;                  // Guessed name
  CCameraShakerData mImpactShake;             // Guessed name
  CDamageInfo mDamage;                        // Guessed name
  rstl::optional_object< CAABox > mTipBounds; // Guessed name
  rstl::vector< CVector3f > mSplinePoints;    // Guessed name
  bool mCanDamage : 1;                        // Guessed name
  bool mExtending : 1;                        // Guessed name
  bool mRetracting : 1;                       // Guessed name
  float mElapsedTime;                         // Guessed name
  float mRetractTime;                         // Guessed name
  float mLengthScale;                         // Guessed name
  float mAttackTime;                          // Guessed name
};
CHECK_SIZEOF(CMediumIngTentacle, 0x330)

// Guessed class: an Ing that turns to mist to cross the arena and attacks with tentacles.
class CMediumIng : public CPatterned {
public:
  CMediumIng(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CTransform4f& xf, const CModelData& modelData,
             const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
             const CMediumIngData& data);

  // CEntity
  ~CMediumIng() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void TakeDamage(const CVector3f& direction, float magnitude) override;
  bool CanBeShot(const CStateManager& mgr, int type) override;
  bool IsListening() const override;
  bool Listen(CStateManager& mgr, const CVector3f& pos, EListenNoiseType type) override;

  // CPatterned
  using CPatterned::GetSearchPath;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  void SetupStateMachine(CStateManager& mgr) override;

  // CMediumIng
  void SubStart(CStateManager& mgr, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt);
  void FollowApproachPattern(CStateManager& mgr, EStateMsg msg, float dt);
  void MistIn(CStateManager& mgr, EStateMsg msg, float dt);
  void MistOut(CStateManager& mgr, EStateMsg msg, float dt);
  void ArmAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void MeleeAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void MistAttack(CStateManager& mgr, EStateMsg msg, float dt);
  void Jump(CStateManager& mgr, EStateMsg msg, float dt);
  void Dash(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void WaitForLocomotion(CStateManager& mgr, EStateMsg msg, float dt);
  void Idle(CStateManager& mgr, EStateMsg msg, float dt);
  void SafezoneReaction(CStateManager& mgr, EStateMsg msg, float dt);
  void MoveToSafeZone(CStateManager& mgr, EStateMsg msg, float dt);
  void BackUp(CStateManager& mgr, EStateMsg msg, float dt);
  void Dead(CStateManager& mgr, EStateMsg msg, float dt);
  void SelectTargetState(CStateManager& mgr, EStateMsg msg, float dt);

  void SelectTarget(CStateManager& mgr, float dt);
  void SetLeashTarget(CStateManager& mgr, float dt);
  void SetRetreatDestination(CStateManager& mgr, float dt);
  void SetMeshPathDestination(CStateManager& mgr, float dt);
  void SetDashDestination(CStateManager& mgr, float dt);
  void ResetFrustatedCounter(CStateManager& mgr, float dt);
  void IncrementFrustatedCounter(CStateManager& mgr, float dt);

  bool PathShagged(CStateManager& mgr, const CTriggerData& data) const;
  bool IsOffPath(CStateManager& mgr, const CTriggerData& data) const;
  bool Leash(CStateManager& mgr, const CTriggerData& data) const;
  bool IsMisting(CStateManager& mgr, const CTriggerData& data) const;
  bool IsAggressive(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldGenerate(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldArmAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMeleeAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMistAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDoubleDash(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBackUp(CStateManager& mgr, const CTriggerData& data) const;
  bool HasLineOfSight(CStateManager& mgr, const CTriggerData& data) const;
  bool HasJumpPoint(CStateManager& mgr, const CTriggerData& data) const;
  bool HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool HasApproachPattern(CStateManager& mgr, const CTriggerData& data) const;
  bool PatternOver(CStateManager& mgr, const CTriggerData& data) const;
  bool IsDestInsideCurrentRegion(CStateManager& mgr, const CTriggerData& data) const;
  bool IsFrustated(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDoSafezoneReaction(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldEvaporate(CStateManager& mgr, const CTriggerData& data) const;
  bool IsLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const;
  bool StillLuredBySafeZone(CStateManager& mgr, const CTriggerData& data) const;

private:
  // Guessed names for all private helpers.
  bool HasMist() const;
  void ValidateSafeZone(const CStateManager& mgr);
  bool FindSafeZoneRetreatPoint(const CStateManager& mgr, CVector3f& outPoint);
  void UpdateCurrentSafeZone(const CStateManager& mgr, const CVector3f& point);
  void UpdateMistEffect(CStateManager& mgr);
  void CreateMistEffect(CStateManager& mgr, const TLockedToken< CGenDescription >& desc);
  void SetFlagE1();
  TUniqueId FindClosestRetreatActor(const CStateManager& mgr, const CVector3f& pos) const;
  TUniqueId FindClosestDangerActor(const CStateManager& mgr, const CVector3f& pos) const;
  bool IsPointInSafeZone(const CStateManager& mgr, const CVector3f& pos) const;
  bool IsInSafeZone(const CStateManager& mgr) const;
  CPFArea* GetPathArea(CStateManager& mgr) const;
  void QuitTeam(CStateManager& mgr);
  void JoinTeam(CStateManager& mgr);
  void CalculateSeparation(CStateManager& mgr);

  CPathFindSearch mPathFindSearch;           // Guessed name
  CLineOfSightTracker mLineOfSightTracker;   // Guessed name
  CSteeringBehaviors mSteering;              // Guessed name
  CSurfaceAlignmentHelper mSurfaceAlignment; // Guessed name
  CMediumIngData mData;                      // Guessed name
  TUniqueId mJumpPointId;                    // Guessed name
  TUniqueId mTeamAiMgrId;                    // Guessed name
  TUniqueId mSafeZoneId;                     // Guessed name
  int mMistState;                            // Guessed name
  float mMistAmount;                         // Guessed name
  float mF60;
  float mF64;
  float mAggressionTimer;
  int mI6C;
  float mF70;
  int mI74;
  CVector3f mV78;
  CVector3f mV84;
  CVector3f mV90;
  float mDamageSinceMist;
  uchar mTentacleIndex;
  float mFA4;
  float mFA8;
  float mFAC;
  float mFB0;
  float mFB4;
  float mFB8;
  float mFBC;
  float mFC0;
  float mFC4;
  float mFC8;
  float mFCC;
  float mFD0;
  rstl::reserved_vector< TUniqueId, 4 > mTentacleIds;
  TUniqueId mLightId;
  TUniqueId mMistEffectId;
  CSfxHandle mSfx0;
  CSfxHandle mSfx1;
  CSfxHandle mSfx2;
  int mTauntAnim;
  int mFrustratedCounter;
  int mSafeZoneReactionCount;
  bool mC0 : 1;
  bool mC1 : 1;
  bool mC2 : 1;
  bool mC3 : 1;
  bool mC4 : 1;
  bool mC5 : 1;
  bool mC6 : 1;
  bool mC7 : 1;
  bool mD0 : 1;
  bool mD1 : 1;
  bool mD2 : 1;
  bool mD3 : 1;
  bool mD4 : 1;
  bool mD5 : 1;
  bool mD6 : 1;
  bool mD7 : 1;
  bool mE0 : 1;
  bool mE1 : 1;
  bool mE2 : 1;
};

CHECK_SIZEOF(CMediumIng, 0xD00)

#endif // _CMEDIUMING
