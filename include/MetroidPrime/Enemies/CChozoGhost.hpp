#ifndef _CCHOZOGHOST
#define _CCHOZOGHOST

#include "types.h"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CGenDescription;

class CChozoGhost : public CPatterned {
public:
  enum EBehaveType {
    kBT_Lurk,
    kBT_Taunt,
    kBT_Attack,
    kBT_Move,
    kBT_None,
  };

  class CBehaveChance {
  public:
    CBehaveChance(float lurk, float taunt, float attack, float move, float lurkTime,
                  float chargeAttack, uint numBolts);

    EBehaveType GetBehave(EBehaveType type, CStateManager& mgr) const;
    float GetLurk() const { return mLurk; }
    float GetTaunt() const { return mTaunt; }
    float GetAttack() const { return mAttack; }
    float GetMove() const { return mMove; }
    float GetLurkTime() const { return mLurkTime; }
    float GetChargeAttack() const { return mChargeAttack; }
    uint GetNumBolts() const { return mNumBolts; }

  private:
    float mLurk;
    float mTaunt;
    float mAttack;
    float mMove;
    float mLurkTime;
    float mChargeAttack;
    uint mNumBolts;
  };

  CChozoGhost(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
              const CPatternedInfo& pInfo, float hearingRadius, float fadeOutDelay,
              float attackDelay, float freezeTime, CAssetId wpsc1, const CDamageInfo& dInfo1,
              CAssetId wpsc2, const CDamageInfo& dInfo2, const CBehaveChance& chance1,
              const CBehaveChance& chance2, const CBehaveChance& chance3, ushort soundImpact,
              float f1, ushort sfxFadeIn, ushort sfxFadeOut, uint w1, float f2, uint w2,
              float hurlRecoverTime, CAssetId projectileVisor, ushort soundProjectileVisor,
              float f3, float f4, uint nearChance, uint midChance);

  // CEntity
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f& position,
                                                         const CVector3f& direction,
                                                         const CWeaponMode& mode,
                                                         int attributes) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CAi
  void KnockBack(CStateManager& mgr, const CKnockBackInfo& info) override;
  bool CanBeShot(const CStateManager& mgr, int w1) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CPatterned
  uchar GetModelAlphau8(const CStateManager& mgr) const override;
  CProjectileInfo* ProjectileInfo() override;
  void SetupStateMachine(CStateManager& mgr) override;
  bool IsOnGround() const override;
  float GetGravityConstant() const override { return skGravityConstant; }

  // State functions
  void Dead(CStateManager& mgr, EStateMsg msg, float arg);
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg);
  void Run(CStateManager& mgr, EStateMsg msg, float arg);
  void Generate(CStateManager& mgr, EStateMsg msg, float arg);
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg);
  void Attack(CStateManager& mgr, EStateMsg msg, float arg);
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg);
  void InActive(CStateManager& mgr, EStateMsg msg, float arg);
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg);
  void Hurled(CStateManager& mgr, EStateMsg msg, float arg);
  void WallDetach(CStateManager& mgr, EStateMsg msg, float arg);
  void Growth(CStateManager& mgr, EStateMsg msg, float arg);
  void Land(CStateManager& mgr, EStateMsg msg, float arg);
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg);

  // Transition functions
  bool Leash(CStateManager& mgr, const CTriggerData& data) const;
  bool InRange(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldAttack(CStateManager& mgr, const CTriggerData& data) const;
  bool AggressionCheck(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldFlinch(CStateManager& mgr, const CTriggerData& data) const;
  bool ShouldMove(CStateManager& mgr, const CTriggerData& data) const;
  bool AIStage(CStateManager& mgr, const CTriggerData& data) const;

private:
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
  void FloatToLevel(float f1, float dt);
  const CBehaveChance& ChooseBehaveChanceRange(CStateManager& mgr) const;
  bool IsVisibleEnough(const CStateManager& mgr) const;
  void SetWarpPosition(CStateManager& mgr, const CVector3f& dir);
  void FindBestAnchor(CStateManager& mgr);

  float mHearingRadius;
  float mFadeOutDelay;
  float mAttackDelay;
  float mFreezeTime;
  CProjectileInfo mProjectileInfo1;
  CProjectileInfo mProjectileInfo2;
  CBehaveChance mBehaveChance1;
  CBehaveChance mBehaveChance2;
  CBehaveChance mBehaveChance3;
  ushort mSoundImpact;
  float x62c_;
  ushort mSfxFadeIn;
  ushort mSfxFadeOut;
  float x634_;
  float mHurlRecoverTime;
  int x63c_;
  rstl::optional_object< TLockedToken< CGenDescription > > mProjectileVisor;
  ushort mSoundProjectileVisor;
  float x654_;
  float x658_;
  int mNearChance;
  int mMidChance;
  uchar mBehaviorEnabled : 1;
  bool mFlinch : 1;
  bool mAlert : 1;
  bool mOnGround : 1;
  bool x664_28_ : 1;
  bool mFadedIn : 1;
  bool mFadedOut : 1;
  bool x664_31_ : 1;
  bool x665_24_ : 1;
  bool x665_25_ : 1;
  bool mShouldSwoosh : 1;
  bool mPlayerInLeashRange : 1;
  bool mInRange : 1;
  bool mAggressive : 1;
  float x668_;
  float x66c_;
  float x670_;
  TUniqueId mCoverPoint;
  float mFloorLevel;
  int mAttackType;
  EBehaveType mBehaveType;
  float mLurkDelay;
  CSteeringBehaviors mSteeringBehaviors;
  CBoneTracking mBoneTracking;
  TUniqueId mTeamMgr;
  float mSpaceWarpTime;
  CVector3f mSpaceWarpPosition;
  int x6d8_;

  static const float skGravityConstant; // Guessed name.
  static const rstl::string skSpeedSwooshName;
};
CHECK_SIZEOF(CChozoGhost, 0x930)

#endif // _CCHOZOGHOST
