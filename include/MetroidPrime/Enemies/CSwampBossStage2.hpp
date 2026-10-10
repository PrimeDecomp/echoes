#ifndef _CSWAMPBOSSSTAGE2
#define _CSWAMPBOSSSTAGE2

#include "types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CMissileRepeller.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSwampBossStage2.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollisionActor;
class CInt32POINode;
class CScannableObjectInfo;
class CScriptGrapplePoint;

// Guessed class name; the Swamp Boss (Chykka) flying stage. The constructor allocates 0xF20 bytes
// and calls the CPatterned constructor with kPAI_SwampBossStage2.
class CSwampBossStage2 : public CPatterned {
public:
  CSwampBossStage2(const TUniqueId& uid, const rstl::string& name, CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& modelData,
                   const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                   const SLdrSwampBossStage2Data& swampBossStage2Properties);

  // CEntity
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void OnScanStateChange(EScanState state, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CScannableObjectInfo* GetScannableObjectInfo() const override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;
  CVector3f GetIngSnatchingNormal(float t) const override;
  CVector3f GetIngSnatchingPoint(float t) const override;

  // Code functions
  void ResetAttackPhase(CStateManager& mgr, float dt);

  // Triggers
  bool StateOver(CStateManager& mgr, const CTriggerData& data) const;
  bool AllGrowthsDead(CStateManager& mgr, const CTriggerData& data) const;
  bool OvipositorDead(CStateManager& mgr, const CTriggerData& data) const;
  bool ZeroHealth(CStateManager& mgr, const CTriggerData& data) const;
  bool PlayerInWater(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSpit(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBarrage(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldSwoop(CStateManager& mgr, const CTriggerData& data) const;
  bool SwoopOver(CStateManager& mgr, const CTriggerData& data) const;
  bool CircleDamage(CStateManager& mgr, const CTriggerData& data) const;
  bool CircleOver(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldBlow(CStateManager& mgr, const CTriggerData& data) const;
  bool IsLevel(CStateManager& mgr, const CTriggerData& data) const;
  bool SwitchedPlatform(CStateManager& mgr, const CTriggerData& data) const;
  bool Stunned(CStateManager& mgr, const CTriggerData& data) const;
  bool StunTimeOut(CStateManager& mgr, const CTriggerData& data) const;
  bool OneGrowthDead(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldDropFlyer(CStateManager& mgr, const CTriggerData& data) const;

  // States
  void Start(CStateManager& mgr, EStateMsg msg, float dt);
  void DropFlyer(CStateManager& mgr, EStateMsg msg, float dt);
  void Flinch(CStateManager& mgr, EStateMsg msg, float dt);
  void Recover(CStateManager& mgr, EStateMsg msg, float dt);
  void Circle(CStateManager& mgr, EStateMsg msg, float dt);
  void Level(CStateManager& mgr, EStateMsg msg, float dt);
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt);
  void SpitBarrage(CStateManager& mgr, EStateMsg msg, float dt);
  void SpitWater(CStateManager& mgr, EStateMsg msg, float dt);
  void DeathSequence(CStateManager& mgr, EStateMsg msg, float dt);
  void Reel(CStateManager& mgr, EStateMsg msg, float dt);
  void Blow(CStateManager& mgr, EStateMsg msg, float dt);
  void Swoop(CStateManager& mgr, EStateMsg msg, float dt);
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt);
  void DarkLurk(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeLightFlyer(CStateManager& mgr, EStateMsg msg, float dt);
  void BecomeDarkFlyer(CStateManager& mgr, EStateMsg msg, float dt);

private:
  // Guessed names; the attack kinds selected from the phase tables.
  enum EAttack {
    kAttack_Spit0,
    kAttack_Spit1,
    kAttack_Spit2,
    kAttack_Barrage,
    kAttack_Swoop,
    kAttack_DropFlyer,
    kAttack_Blow,
    kAttack_Taunt,
    kAttack_None,
  };

  void AddAdditiveDirectionalReaction(CStateManager& mgr);                         // Guessed name
  void CreateBubbleTelegraph(const CVector3f& position);                           // Guessed name
  void SetCollisionVulnerabilities(CStateManager& mgr, bool reflect);              // Guessed name
  void MoveTowardsTarget(float dt);                                                // Guessed name
  CActor* FindFirstXc54ActorInFront(CStateManager& mgr);                           // Guessed name
  void UpdateTargetability(CStateManager& mgr);                                    // Guessed name
  void BeginFight(CStateManager& mgr);                                             // Guessed name
  const SLdrSwampBossStage2Phase& GetCurrentPhase() const;                         // Guessed name
  int GetPhaseAttack(const SLdrSwampBossStage2Phase& phase, int index) const;      // Guessed name
  void SelectNextAttack(const SLdrSwampBossStage2Phase& phase);                    // Guessed name
  float SumXd94() const;                                                           // Guessed name
  void UpdateXd94(float dt);                                                       // Guessed name
  const CScriptGrapplePoint* GetOrbitGrapplePoint(const CStateManager& mgr) const; // Guessed name
  void UpdateBossHealth();                                                         // Guessed name
  void SpawnSplashShockWave(CStateManager& mgr, CVector3f position);               // Guessed name
  void SpawnBlowEffect(CStateManager& mgr);                                        // Guessed name
  void SpawnWingDamageEffect(CStateManager& mgr, int index);                       // Guessed name
  void UpdateWaterEffects(CStateManager& mgr, const CVector3f& position,
                          CVector3f& lastSplashPosition);                 // Guessed name
  void UpdateWaterLevel(CStateManager& mgr);                              // Guessed name
  void SelectXc38Target(CStateManager& mgr);                              // Guessed name
  void SelectXdf4Target(CStateManager& mgr);                              // Guessed name
  void HandleCollisionActorDamage(CStateManager& mgr, TUniqueId actorId); // Guessed name
  CActor* FindNearestXc54Actor(CStateManager& mgr) const;                 // Guessed name
  CVector3f GetPlayerCenter(const CStateManager& mgr) const;              // Guessed name
  CVector3f GetXcf0Position(const CStateManager& mgr) const;              // Guessed name
  void AdvanceXcf0(CStateManager& mgr);                                   // Guessed name
  void LaunchSpit(CStateManager& mgr, const CInt32POINode& node);         // Guessed name
  void SendXca4Message(CStateManager& mgr);                               // Guessed name
  void ApplyChainPositions(CStateManager& mgr, const rstl::vector< TUniqueId >& ids,
                           const rstl::vector< CVector3f >& positions); // Guessed name
  void CollectChain(CStateManager& mgr, rstl::vector< TUniqueId >& ids, const CActor& actor,
                    rstl::vector< CVector3f >& positions); // Guessed name
  void TransformChain(CStateManager& mgr, const rstl::vector< TUniqueId >& ids,
                      const CVector3f& origin, const CTransform4f& xf); // Guessed name
  void TransformChainRecursive(CStateManager& mgr, CActor& actor, const CVector3f& origin,
                               const CTransform4f& xf); // Guessed name
  void CollectChainFrom(CStateManager& mgr, TUniqueId id, rstl::vector< TUniqueId >& ids,
                        rstl::vector< CVector3f >& positions); // Guessed name
  void ActivateChain(CStateManager& mgr, TUniqueId id, const rstl::vector< TUniqueId >& ids,
                     const rstl::vector< CVector3f >& positions,
                     const CVector3f& target); // Guessed name
  void PositionChainAtPlayer(CStateManager& mgr, const rstl::vector< TUniqueId >& ids,
                             const CVector3f& gunPosition);               // Guessed name
  void UpdateAdditiveReaction(float dt, CStateManager& mgr);              // Guessed name
  void SetWingEffectState(CStateManager& mgr, bool active);               // Guessed name
  void SetWingsTargetable(CStateManager& mgr, bool targetable);           // Guessed name
  void SetWingTargetable(CStateManager& mgr, int index, bool targetable); // Guessed name
  CVector3f GetXc54SwoopDirection(CStateManager& mgr) const;              // Guessed name
  void SetMoveTarget(const CVector3f& position, float dt);                // Guessed name
  CVector3f GetAimDirection(const CStateManager& mgr) const;              // Guessed name
  CVector3f GetCollisionActorPosition(CStateManager& mgr, int index);     // Guessed name
  void SetupScanInfos();                                                  // Guessed name
  void SetupWingLocators();                                               // Guessed name
  void SetupAdditiveReaction();                                           // Guessed name
  void SetupCollisionActors(CStateManager& mgr);                          // Guessed name
  void SetupCollisionManager(CStateManager& mgr);                         // Guessed name
  CCollisionActor* GetCollisionActor(CStateManager& mgr, uint index);     // Guessed name
  const CVector3f& GetXdf4() const;                                       // Guessed name
  const CVector3f& GetXc64Position(const CStateManager& mgr) const;       // Guessed name
  const CVector3f& GetXc48Position(const CStateManager& mgr) const;       // Guessed name

  // Guessed member names; recovered from the constructor stores and their consumers.
  SLdrSwampBossStage2Data mProperties;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  CVector3f mMouthPosition;
  CVector3f mSpine5Position;
  CVector3f mSpine6Position;
  rstl::reserved_vector< CSegId, 4 > mWingLocators;
  rstl::reserved_vector< rstl::optional_object< CModelData >, 4 > mWingModels;
  rstl::reserved_vector< float, 4 > mWingHealth;
  rstl::reserved_vector< float, 4 > mWingHitTimers;
  int xc34_;
  rstl::vector< TUniqueId > xc38_;
  TUniqueId xc48_;
  float mLightFlyerHealth;
  float mDarkFlyerHealth;
  rstl::vector< TUniqueId > xc54_;
  TUniqueId xc64_;
  TUniqueId xc66_;
  CVector3f xc68_;
  bool xc74_24_ : 1;
  bool xc74_25_ : 1;
  bool xc74_26_ : 1;
  bool xc74_27_ : 1;
  bool xc74_28_ : 1;
  bool xc74_29_ : 1;
  bool xc74_30_ : 1;
  bool xc74_31_ : 1;
  bool xc75_24_ : 1;
  bool xc75_25_ : 1;
  bool xc75_26_ : 1;
  bool xc75_27_ : 1;
  bool xc75_28_ : 1;
  bool xc75_29_ : 1;
  bool xc75_30_ : 1;
  bool xc75_31_ : 1;
  bool xc76_24_ : 1;
  bool xc76_25_ : 1;
  bool xc76_26_ : 1;
  bool xc76_27_ : 1;
  bool xc76_28_ : 1;
  bool xc76_29_ : 1;
  bool xc76_30_ : 1;
  bool xc76_31_ : 1;
  bool xc77_24_ : 1;
  bool xc77_25_ : 1;
  bool xc77_26_ : 1;
  bool xc77_27_ : 1;
  bool xc77_28_ : 1;
  bool xc77_29_ : 1;
  bool xc77_30_ : 1;
  bool xc77_31_ : 1;
  int mAdditiveReactionAnim;
  float xc7c_;
  float xc80_;
  float xc84_;
  float xc88_;
  int xc8c_;
  float xc90_;
  int xc94_;
  int xc98_;
  int mCurrentAttack;
  float xca0_;
  TUniqueId xca4_;
  CProjectileInfo mSpitProjectile;
  rstl::vector< TUniqueId > xcd0_;
  rstl::vector< TUniqueId > xce0_;
  TUniqueId xcf0_;
  int xcf4_;
  float xcf8_;
  TLockedToken< CGenDescription > mSplashEffect;
  TLockedToken< CGenDescription > mWingDamageEffect;
  TLockedToken< CGenDescription > mBlowEffect;
  CVector3f xd20_;
  CVector3f xd2c_;
  float xd38_;
  TUniqueId xd3c_;
  CDamageInfo mSwoopDamage;
  float xd5c_;
  CVector3f xd60_;
  float xd6c_;
  float xd70_;
  float xd74_;
  int xd78_;
  float xd7c_;
  float xd80_;
  int xd84_;
  float xd88_;
  float xd8c_;
  float xd90_;
  rstl::list< rstl::pair< float, float > > xd94_;
  rstl::vector< TUniqueId > xdac_;
  rstl::vector< CVector3f > xdbc_;
  float xdcc_;
  TUniqueId xdd0_;
  rstl::vector< TUniqueId > xdd4_;
  rstl::vector< CVector3f > xde4_;
  CVector3f xdf4_;
  int xe00_;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanInfoLight;
  rstl::single_ptr< TCachedToken< CScannableObjectInfo > > mScanInfoDark;
  CVector3f xe0c_;
  CVector3f xe18_;
  float xe24_;
  float xe28_;
  CDamageVulnerability xe2c_;
  CDamageVulnerability xe5c_;
  rstl::auto_ptr< CParticleGen > xe8c_;
  rstl::optional_object< TLockedToken< CGenDescription > > mSpitVisorEffect;
  CMissileRepeller mMissileRepeller;
  CDamageInfo mBlowDamage;
  TUniqueId xf00_;
  float xf04_;
  CSfxHandle mWaterSfxHandle;
  float xf0c_;
  float xf10_;
  rstl::reserved_vector< bool, 4 > xf14_;
  TUniqueId xf1c_;
};
CHECK_SIZEOF(CSwampBossStage2, 0xf20)

#endif // _CSWAMPBOSSSTAGE2
