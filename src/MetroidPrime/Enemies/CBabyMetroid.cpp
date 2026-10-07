#include "MetroidPrime/Enemies/CBabyMetroid.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"

typedef CPatterned::StateMachine::TriggerFunc TriggerFunc;
typedef CPatterned::StateMachine::StateFunc StateFunc;
typedef CPatterned::StateMachine::CodeFunc CodeFunc;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"StateOver", static_cast< TriggerFunc >(&CMetroid::StateOver)},
    {"AttackOver", static_cast< TriggerFunc >(&CMetroid::AttackOver)},
    {"LostInterest", static_cast< TriggerFunc >(&CMetroid::LostInterest)},
    {"PatternShagged", static_cast< TriggerFunc >(&CMetroid::PatternShagged)},
    {"Attacked", static_cast< TriggerFunc >(&CMetroid::Attacked)},
    {"ShotAt", static_cast< TriggerFunc >(&CMetroid::ShotAt)},
    {"ShouldAttack", static_cast< TriggerFunc >(&CMetroid::ShouldAttack)},
    {"InAttackPosition", static_cast< TriggerFunc >(&CMetroid::InAttackPosition)},
    {"InPosition", static_cast< TriggerFunc >(&CMetroid::InPosition)},
    {"InRange", static_cast< TriggerFunc >(&CMetroid::InRange)},
    {"InDetectionRange", static_cast< TriggerFunc >(&CMetroid::InDetectionRange)},
    {"SpotPlayer", static_cast< TriggerFunc >(&CMetroid::SpotPlayer)},
    {"AggressionCheck", static_cast< TriggerFunc >(&CMetroid::AggressionCheck)},
    {"ShouldTurn", static_cast< TriggerFunc >(&CMetroid::ShouldTurn)},
    {"Leash", static_cast< TriggerFunc >(&CMetroid::Leash)},
    {"ShouldWallHang", static_cast< TriggerFunc >(&CMetroid::ShouldWallHang)},
    {"ShouldDodge", static_cast< TriggerFunc >(&CMetroid::ShouldDodge)},
    {"AnimOver", static_cast< TriggerFunc >(&CBabyMetroid::AnimOver)},
    {"ShouldSeekEnergySource", static_cast< TriggerFunc >(&CBabyMetroid::ShouldSeekEnergySource)},
    {"AbsorbFinished", static_cast< TriggerFunc >(&CBabyMetroid::AbsorbFinished)},
    {"InEnergySourcePosition", static_cast< TriggerFunc >(&CBabyMetroid::InEnergySourcePosition)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Patrol", static_cast< StateFunc >(&CMetroid::Patrol)},
    {"Generate", static_cast< StateFunc >(&CBabyMetroid::Generate)},
    {"SelectTarget", static_cast< StateFunc >(&CMetroid::SelectTarget)},
    {"PathFind", static_cast< StateFunc >(&CMetroid::PathFind)},
    {"TurnAround", static_cast< StateFunc >(&CMetroid::TurnAround)},
    {"TelegraphAttack", static_cast< StateFunc >(&CMetroid::TelegraphAttack)},
    {"Attack", static_cast< StateFunc >(&CBabyMetroid::Attack)},
    {"WallHang", static_cast< StateFunc >(&CMetroid::WallHang)},
    {"Dodge", static_cast< StateFunc >(&CMetroid::Dodge)},
    {"ExitHive", static_cast< StateFunc >(&CBabyMetroid::ExitHive)},
    {"AbsorbEnergy", static_cast< StateFunc >(&CBabyMetroid::AbsorbEnergy)},
    {"TransformIntoMetroid", static_cast< StateFunc >(&CBabyMetroid::TransformIntoMetroid)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"SetTargetDest", static_cast< CodeFunc >(&CMetroid::SetTargetDest)},
    {"SetPatrolDest", static_cast< CodeFunc >(&CMetroid::SetPatrolDest)},
    {"SetEnergySourceDest", static_cast< CodeFunc >(&CBabyMetroid::SetEnergySourceDest)},
};

CBabyMetroid::CBabyMetroid(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& aParms,
                           const CMetroidData& metroidData)
: CMetroid(uid, name, info, xf, mData, pInfo, aParms, metroidData)
, xa48_(metroidData.xa4_)
, xa4c_(metroidData.xa0_)
, xa50_(0.f)
, xa64_(kInvalidUniqueId)
, xa66_(kInvalidUniqueId)
, xa68_(kInvalidUniqueId)
, mInitialScale(mData.GetScale().GetX())
, mBabyMetroidScale(metroidData.mBabyMetroidScale)
, xa74_(metroidData.xa8_)
, xa78_(0.f)
, mChanceToDodge(metroidData.mChanceToDodge)
, mDodgeCheckTimeInterval(metroidData.mDodgeCheckTimeInterval)
, xa84_(0.f)
, mGrowthVulnerability(metroidData.mBabyMetroidGrowthVulnerability)
, mShouldSeekEnergySource(false)
, xac8_25_(false) {
  ModelData()->SetScale(CVector3f(mInitialScale, mInitialScale, mInitialScale));
  const CMaterialFilter& filter = GetMaterialFilter();
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(filter.GetIncludeList(), CMaterialList(kMT_Character)));
  if (metroidData.mBabyMetroidTransformationParticleEffect != kInvalidAssetId) {
    mTransformationParticle = rstl::optional_object< TLockedToken< CGenDescription > >(
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(
            SObjectTag('PART', metroidData.mBabyMetroidTransformationParticleEffect))));
  }
}

CBabyMetroid::~CBabyMetroid() {}

void CBabyMetroid::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CMetroid::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  stateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

void CBabyMetroid::Render(const CStateManager& mgr) const { CMetroid::Render(mgr); }

bool CBabyMetroid::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::AnimOver(mgr, data);
}

bool CBabyMetroid::ShouldSeekEnergySource(CStateManager&, const CTriggerData&) const {
  return mShouldSeekEnergySource;
}

bool CBabyMetroid::AbsorbFinished(CStateManager&, const CTriggerData&) const {
  return xa50_ >= xa4c_;
}

bool CBabyMetroid::InEnergySourcePosition(CStateManager&, const CTriggerData&) const {
  const CVector3f delta = x7c0_ - GetTranslation();
  return delta.MagSquared() < 4.f;
}

void CBabyMetroid::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kAiState_Over;
    break;
  default:
    break;
  }
}

void CBabyMetroid::ApplyContactDamage(CStateManager& mgr, CActor& target, const CDamageInfo& info) {
  if (mCurDamageRemTime <= 0.f) {
    mgr.ApplyDamage(
        GetUniqueId(), target.GetUniqueId(), GetUniqueId(), info,
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Unknown59), CMaterialList()),
        CVector3f::Zero());
    mCurDamageRemTime = mDamageWaitTime;
  }
}

void CBabyMetroid::TryJoinHive(CStateManager& mgr) {
  CScriptCounter* counter = TCastToPtr< CScriptCounter >(mgr.ObjectById(xa66_));
  if (counter != nullptr && counter->GetCurrent() < counter->GetMax()) {
    if (mgr.Random()->Float() <= xa48_) {
      mShouldSeekEnergySource = true;
      counter->AcceptScriptMsg(mgr,
                               CScriptMsg(GetUniqueId(), counter->GetUniqueId(), kSM_Increment));
    }
  }
}

void CBabyMetroid::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mDodgeDirection = mgr.Random()->Float() < 0.5f ? pas::kSD_Left : pas::kSD_Right;
    mShouldDodge = false;
    break;
  }
  CMetroid::Dodge(mgr, msg, dt);
}

void CBabyMetroid::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!mShouldSeekEnergySource) {
    xa84_ = rstl::min_val(xa84_ + dt, mDodgeCheckTimeInterval);
  }
  CMetroid::PathFind(mgr, msg, dt);
}

bool CBabyMetroid::ShouldDodge(CStateManager&, const CTriggerData&) const { return mShouldDodge; }

void CBabyMetroid::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CMetroid::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    BodyController()->SetLocomotionType(pas::kLT_Internal7);
    break;
  case kSM_AreaLoaded: {
    xa54_.reserve(8);
    rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
    for (; it != GetConnectionList().end(); ++it) {
      const EScriptObjectState state = it->state;
      const TUniqueId id = mgr.GetIdForScript(it->objId);
      if (state == kSS_Approach) {
        if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
          xa54_.push_back_unsafe(id);
        } else if (CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(id))) {
          xa64_ = id;
          actor->AddMaterial(kMT_AIPassthrough, mgr);
          actor->RemoveMaterial(kMT_AIBlock, mgr);
          const CMaterialFilter& filter = actor->GetMaterialFilter();
          actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
              filter.GetIncludeList(), CMaterialList(kMT_Character)));
        }
      } else if (state == kSS_MaxReached) {
        if (TCastToConstPtr< CScriptCounter >(mgr.GetObjectById(id))) {
          xa66_ = id;
        } else if (TCastToConstPtr< CScriptEffect >(mgr.GetObjectById(id))) {
          xa68_ = id;
        }
      }
    }
    break;
  }
  case kSM_Damage:
  case kSM_ResistedDamage:
  case kSM_Delete:
  case kSM_Decrement:
  case kSM_Deactivate:
  case kSM_Alert:
    break;
  default:
    break;
  }
}

void CBabyMetroid::Think(float dt, CStateManager& mgr) {
  if (mHitByPlayerProjectile) {
    if (mShouldSeekEnergySource) {
      mShouldSeekEnergySource = false;
      xac8_25_ = false;
      if (CScriptCounter* counter = TCastToPtr< CScriptCounter >(mgr.ObjectById(xa66_))) {
        counter->AcceptScriptMsg(mgr,
                                 CScriptMsg(GetUniqueId(), counter->GetUniqueId(), kSM_Decrement));
      }
      xa78_ = 0.f;
    }
  }
  if (xa84_ >= mDodgeCheckTimeInterval && !mShouldDodge) {
    if (mgr.Random()->Float() < mChanceToDodge) {
      mShouldDodge = true;
    }
    xa84_ = 0.f;
  }
  CPlayer* player = mgr.Player(0);
  const CVector3f delta = player->GetTranslation() - GetTranslation();
  if (delta.MagSquared() < 5.f) {
    ApplyContactDamage(mgr, *player, GetContactDamage());
  }
  if (!mShouldSeekEnergySource) {
    xa78_ = rstl::min_val(xa78_ + dt, xa74_);
    if (xa78_ >= xa74_) {
      TryJoinHive(mgr);
      xa78_ = 0.f;
    }
  }
  CMetroid::Think(dt, mgr);
  if (mHitByPlayerProjectile) {
    mHitByPlayerProjectile = false;
  }
}

CVector3f CBabyMetroid::FindAIHintPosition(CStateManager& mgr, int hintType) {
  rstl::reserved_vector< CScriptAIHint*, 20 > hints;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hintType == 9) {
      if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
          hintType == hint->GetHintType() && hint->GetActive() == true && hints.size() < 20) {
        if (!hint->GetInUse(kInvalidUniqueId)) {
          hints.push_back(hint);
        }
      }
    } else if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
               hintType == hint->GetHintType() && hint->GetActive() == true && hints.size() < 20) {
      hints.push_back(hint);
    }
  }
  if (hints.size() == 0) {
    return CVector3f::Zero();
  }
  return hints[mgr.Random()->Range(0, hints.size() - 1)]->GetTranslation();
}

void CBabyMetroid::SetEnergySourceDest(CStateManager& mgr, float) {
  if (!xac8_25_) {
    BodyController()->SetLocomotionType(pas::kLT_Lurk);
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    x7c0_ = GetTranslation();
    const CVector3f pos = FindAIHintPosition(mgr, 9);
    if (!(pos == CVector3f::Zero())) {
      x7c0_ = pos;
    }
    xac8_25_ = true;
  }
  mPathFindNavigation.SetDestination(x7c0_);
  mPathFindNavigation.SetFaceTarget(kInvalidUniqueId);
}

void CBabyMetroid::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kAiState_Over;
    xa78_ = 0.f;
    mShouldSeekEnergySource = false;
    xac8_25_ = false;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate: {
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Melee, mgr, mTeamAiManagerId, GetUniqueId(),
                                false);
    mAttackChance = GetAverageAttackTime();
    if (mgr.IsRandomAvailable() == true) {
      const float variation = mAttackTimeVariation;
      const float random = mgr.Random()->Float();
      mAttackChance += random * variation;
    }
    mAttackState = 0;
    DetachFromTarget(mgr, true);
    mIsAttacking = false;
    const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle::FromRadians(GetYaw()));
    SetTransform(rotation.BuildTransform4f(GetTranslation()));
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    RemoveMaterial(kMT_Trigger, mgr);
    break;
  }
  }
}

void CBabyMetroid::AbsorbEnergy(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    *DamageVulnerability() = mGrowthVulnerability;
    BodyController()->SetLocomotionType(pas::kLT_Internal8);
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    break;
  case kStateMsg_Update: {
    xa50_ = rstl::min_val(xa50_ + dt, xa4c_);
    const float t = xa50_ / xa4c_;
    const float scale = mInitialScale * (1.f - t) + mBabyMetroidScale * t;
    ModelData()->SetScale(CVector3f(scale, scale, scale));
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CBabyMetroid::ExitHive(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType(9), -1));
    TryJoinHive(mgr);
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType(9), -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CBabyMetroid::TransformIntoMetroid(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SendScriptMsgs(kSS_MaxReached, mgr, GetUniqueId(), kSM_None);
    if (mTransformationParticle) {
      CExplosion* explosion = rs_new CExplosion(
          *mTransformationParticle, mgr.AllocateUniqueId(),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
          rstl::string_l("Baby Metroid Transformation"), GetTransform(), 0,
          GetModelData()->GetScale(), CColor::White(), -1);
      mgr.AddObject(explosion);
    }
    if (CScriptEffect* effect = TCastToPtr< CScriptEffect >(mgr.ObjectById(xa68_))) {
      effect->SetTransform(GetTransform());
    }
    break;
  case kStateMsg_Update:
    StopLoopedSounds();
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  case kStateMsg_Deactivate:
    break;
  }
}
