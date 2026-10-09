#include "MetroidPrime/Enemies/CChozoGhost.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrChozoGhost.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "REL/REL_Setup.h"

const float CChozoGhost::skGravityConstant = 60.f;
const rstl::string CChozoGhost::skSpeedSwooshName = rstl::string_l("SpeedSwoosh");

static EMaterialTypes SolidMaterial = kMT_Solid;

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::ShouldAttack)},
    {"InRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::InRange)},
    {"ShouldTaunt",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::ShouldTaunt)},
    {"ShouldMove", static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::ShouldMove)},
    {"AIStage", static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::AIStage)},
    {"Leash", static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::Leash)},
    {"ShouldFlinch",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::ShouldFlinch)},
    {"AggressionCheck",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CChozoGhost::AggressionCheck)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Start", &CPatterned::Start},
    {"InActive", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::InActive)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Attack)},
    {"Generate", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Generate)},
    {"Run", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Run)},
    {"SelectTarget",
     static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::SelectTarget)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Dead)},
    {"Deactivate", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Deactivate)},
    {"Shuffle", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Shuffle)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Taunt)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Lurk)},
    {"Hurled", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Hurled)},
    {"Growth", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Growth)},
    {"WallDetach", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::WallDetach)},
    {"Land", static_cast< CPatterned::StateMachine::StateFunc >(&CChozoGhost::Land)},
};

CChozoGhost::CBehaveChance::CBehaveChance(float lurk, float taunt, float attack, float move,
                                          float lurkTime, float chargeAttack, uint numBolts)
: mLurk(lurk)
, mTaunt(taunt)
, mAttack(attack)
, mMove(move)
, mLurkTime(lurkTime)
, mChargeAttack(chargeAttack)
, mNumBolts(numBolts) {
  const float average = 1.f / (mLurk + mTaunt + mAttack + mMove);
  mLurk *= average;
  mTaunt *= average;
  mAttack *= average;
  mMove *= average;
}

CChozoGhost::EBehaveType CChozoGhost::CBehaveChance::GetBehave(const EBehaveType type,
                                                               CStateManager& mgr) const {
  float lurkChance = mLurk;
  float tauntChance = mTaunt;
  float attackChance = mAttack;
  switch (type) {
  case kBT_Lurk: {
    const float delta = lurkChance / 3.f;
    lurkChance = 0.f;
    tauntChance += delta;
    attackChance += delta;
  } break;
  case kBT_Taunt: {
    const float delta = tauntChance / 3.f;
    tauntChance = 0.f;
    lurkChance += delta;
    attackChance += delta;
  } break;
  case kBT_Attack: {
    const float delta = attackChance / 3.f;
    attackChance = 0.f;
    lurkChance += delta;
    tauntChance += delta;
  } break;
  case kBT_Move: {
    const float delta = mMove / 3.f;
    lurkChance += delta;
    tauntChance += delta;
    attackChance += delta;
  } break;
  default:
    break;
  }

  const float rnd = mgr.Random()->Float();
  EBehaveType ret = kBT_Move;
  if (rnd < lurkChance) {
    ret = kBT_Lurk;
  } else if (rnd - lurkChance < tauntChance) {
    ret = kBT_Taunt;
  } else if (rnd - lurkChance - tauntChance < attackChance) {
    ret = kBT_Attack;
  }
  return ret;
}

CChozoGhost::CChozoGhost(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
    const float hearingRadius, const float fadeOutDelay, const float attackDelay,
    const float freezeTime, const CAssetId wpsc1, const CDamageInfo& dInfo1, const CAssetId wpsc2,
    const CDamageInfo& dInfo2, const CBehaveChance& chance1, const CBehaveChance& chance2,
    const CBehaveChance& chance3, const ushort soundImpact, const float f1, const ushort sfxFadeIn,
    const ushort sfxFadeOut, const uint w1, const float f2, const uint w2,
    const float hurlRecoverTime, const CAssetId projectileVisor, const ushort soundProjectileVisor,
    const float f3, const float f4, const uint nearChance, const uint midChance)
: CPatterned(kPAI_ChozoGhost, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_BiPedal, actParms)
, mHearingRadius(hearingRadius)
, mFadeOutDelay(fadeOutDelay)
, mAttackDelay(attackDelay)
, mFreezeTime(freezeTime)
, mProjectileInfo1(wpsc1, dInfo1)
, mProjectileInfo2(wpsc2, dInfo2)
, mBehaveChance1(chance1)
, mBehaveChance2(chance2)
, mBehaveChance3(chance3)
, mSoundImpact(soundImpact)
, x62c_(f1)
, mSfxFadeIn(sfxFadeIn)
, mSfxFadeOut(sfxFadeOut)
, x634_(f2)
, mHurlRecoverTime(hurlRecoverTime)
, x63c_(w2)
, mSoundProjectileVisor(soundProjectileVisor)
, x654_(f3)
, x658_(f4)
, mNearChance(nearChance)
, mMidChance(midChance)
, mBehaviorEnabled(w1 & 1)
, mFlinch((w1 >> 1) & 1)
, mAlert(false)
, mOnGround(false)
, x664_28_(false)
, mFadedIn(false)
, mFadedOut(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, mShouldSwoosh(false)
, mInRange(false)
, mAggressive(false)
, x668_(0.f)
, x66c_(0.f)
, x670_(0.f)
, mCoverPoint(kInvalidUniqueId)
, mFloorLevel(0.f)
, mAttackType(-1)
, mBehaveType(mBehaviorEnabled ? kBT_Attack : kBT_None)
, mLurkDelay(1.f)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 80.f * M_PIF / 180.f,
                CRelAngle::FromDegrees(180.f).AsRadians(), 0)
, mTeamMgr(kInvalidUniqueId)
, mSpaceWarpTime(0.f)
, mSpaceWarpPosition(CVector3f::Zero())
, x6d8_(1) {
  mProjectileInfo1.Token().Lock();
  mProjectileInfo2.Token().Lock();

  const CPASAnimParmData jumpAnimParms(pas::kAS_Jump, CPASAnimParm::FromEnum(3),
                                       CPASAnimParm::FromEnum(0));
  x668_ = GetModelData()->GetScale().GetZ() * GetAnimationDistance(jumpAnimParms);
  const CPASAnimParmData slideAnimParms(pas::kAS_Slide, CPASAnimParm::FromEnum(1),
                                        CPASAnimParm::FromReal32(90.f));
  x66c_ = GetModelData()->GetScale().GetY() * GetAnimationDistance(slideAnimParms);
  const CPASAnimParmData meleeAnimParms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(2),
                                        CPASAnimParm::FromEnum(1));
  x670_ = GetModelData()->GetScale().GetZ() * GetAnimationDistance(meleeAnimParms);

  if (projectileVisor != kInvalidAssetId) {
    mProjectileVisor = gpSimplePool->GetObj(SObjectTag('PART', projectileVisor));
  }

  KnockBackController().EnableBurn(false);
  KnockBackController().EnableLaggedBurnDeath(false);
  KnockBackController().EnableShock(false);
  KnockBackController().EnableFreeze(false);
  SetDrawShadow(false);
}

void CChozoGhost::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_AreaLoaded:
    if (GetActive()) {
      AddToTeam(mgr);
    }
    break;
  case kSM_Activate:
    AddToTeam(mgr);
    break;
  case kSM_Alert:
    if (!mAlert) {
      mAlert = true;
      mHitByPlayerProjectile = true;
    }
    break;
  case kSM_Action:
    if (mFlinch) {
      mAggressive = true;
    }
    break;
  case kSM_Deactivate:
  case kSM_Delete:
    RemoveFromTeam(mgr);
    break;
  case kSM_OffGround:
  case kSM_Launching:
    if (!mVerticalMovement) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    }
    break;
  default:
    break;
  }
}

void CChozoGhost::Touch(CActor& act, CStateManager& mgr) {
  if (IsVisibleEnough(mgr)) {
    if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
      if (mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }
  CPatterned::Touch(act, mgr);
}

bool CChozoGhost::CanBeShot(const CStateManager& mgr, int w1) { return IsVisibleEnough(mgr); }

EWeaponCollisionResponseTypes CChozoGhost::GetCollisionResponseType(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CWeaponMode&, int) const {
  return kWCR_ChozoGhost;
}

const CDamageVulnerability* CChozoGhost::GetDamageVulnerability() const {
  if (x665_24_) {
    return &CDamageVulnerability::PassThroughVulnerabilty();
  }

  return CPatterned::GetDamageVulnerability();
}

uchar CChozoGhost::GetModelAlphau8(const CStateManager& mgr) const {
  uchar ret = 255;
  if (!GetAlive()) {
    ret = mColor.GetAlphau8();
  }

  return ret & 0xFF;
}

void CChozoGhost::PreRender(CStateManager& mgr) {
  const bool echo = mgr.GetPlayerState(0)->GetActiveVisor(mgr) == CPlayerState::kPV_Echo;
  mDrawParticles = !echo;
  if (mgr.GetPlayerState(0)->GetActiveVisor(mgr) == CPlayerState::kPV_Echo) {
    SetCalculateLighting(false);
    ActorLights()->BuildConstantAmbientLighting(CColor::White());
  } else {
    SetCalculateLighting(true);
  }
  CColor color = mColor;
  const uchar alpha = GetModelAlphau8(mgr);
  if (alpha < 255 || color.GetRedu8() != 0) {
    if (color.GetRedu8() != 0) {
      const uchar value = rstl::max_val(255 - 2 * color.GetRedu8(), 0);
      color.SetRed(static_cast< uchar >(255));
      color.SetGreen(value);
      color.SetBlue(value);
    } else {
      color = CColor::White();
    }
    SetModelFlags(CModelFlags::AlphaBlended(
        CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), alpha)));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }
  CActor::PreRender(mgr);
  mBoneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                          GetModelData()->GetScale(), *mBodyController);
}

void CChozoGhost::Render(const CStateManager& mgr) const {
  if (mSpaceWarpTime > 0.f) {
    mgr.DrawSpaceWarp(mSpaceWarpPosition, CMath::FastSinR(M_PIF * mSpaceWarpTime / mFadeOutDelay));
  }
  CPatterned::Render(mgr);
}

bool CChozoGhost::IsVisibleEnough(const CStateManager& mgr) const {
  return GetModelAlphau8(mgr) > 31;
}

CVector3f CChozoGhost::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                 const CVector3f& aimPos) const {
  return GetTranslation();
}

void CChozoGhost::AddToTeam(CStateManager& mgr) {
  if (mTeamMgr == kInvalidUniqueId) {
    mTeamMgr = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }

  if (mTeamMgr == kInvalidUniqueId) {
    return;
  }

  if (CScriptTeamAiMgr* teamMgr = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
    teamMgr->JoinTeam(*this, CTeamAiRole::ETeamAiRole(2), CTeamAiRole::ETeamAiRole(3),
                      CTeamAiRole::ETeamAiRole(-1));
  }
}

void CChozoGhost::RemoveFromTeam(CStateManager& mgr) {
  if (mTeamMgr == kInvalidUniqueId) {
    return;
  }

  CScriptTeamAiMgr* teamMgr = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamMgr));
  if (teamMgr && teamMgr->IsPartOfTeam(GetUniqueId())) {
    teamMgr->QuitTeam(GetUniqueId());
    mTeamMgr = kInvalidUniqueId;
  }
}

void CChozoGhost::FloatToLevel(const float f1, const float dt) {
  CVector3f translation = GetTranslation();
  const float floatAmt = ((f1 - translation.GetZ()) * 4.f);
  translation.SetZ(translation.GetZ() + floatAmt * dt);
  SetTranslation(translation);
}

bool CChozoGhost::IsOnGround() const { return mOnGround; }

static EMaterialTypes AnchorFloorMaterial = kMT_Floor;

void CChozoGhost::FindBestAnchor(CStateManager& mgr) {
  float bestScore = 3.4028235e38f;
  mPlayerInLeashRange = false;
  CScriptCoverPoint* target = nullptr;
  CObjectList& waypoints = mgr.ObjectListById(kOL_AiWaypoint);
  const int random = mgr.Random()->Next() % 100;
  const int range = random < mNearChance ? 0 : (random < mNearChance + mMidChance ? 1 : 2);
  float nearWeight = 10.f * x658_;
  float midWeight = nearWeight;
  float farWeight = nearWeight;
  switch (range) {
  case 0:
    farWeight *= 10.f;
    midWeight *= 5.f;
    break;
  case 1:
    nearWeight *= 10.f;
    farWeight *= 5.f;
    break;
  case 2:
    nearWeight *= 10.f;
    midWeight *= 5.f;
    break;
  }
  for (int i = waypoints.GetFirstObjectIndex(); i != -1; i = waypoints.GetNextObjectIndex(i)) {
    if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(waypoints[i])) {
      if (cover->GetActive() && !cover->GetInUse(kInvalidUniqueId) &&
          cover->GetCurrentAreaId() == GetCurrentAreaId()) {
        const float distance = (cover->GetTranslation() - GetTranslation()).Magnitude();
        if (!(distance < 2.f * x66c_)) {
          float score = rstl::max_val(x654_ - distance, 0.f);
          CVector3f delta = cover->GetTranslation() - mgr.GetPlayer(0)->GetTranslation();
          const float playerDistance = delta.Magnitude();
          if (!(playerDistance < mMinAttackRange)) {
            if (CMath::AbsF(delta.GetZ()) / playerDistance > 0.2f) {
              score += (20.f * x658_) * (CMath::AbsF(delta.GetZ()) / playerDistance - 0.2f);
            }
            if (playerDistance < x654_) {
              score += nearWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer(0)->GetTransform().GetForward(), delta));
              }
            } else if (playerDistance < x658_) {
              score += midWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer(0)->GetTransform().GetForward(), delta));
              }
            } else {
              score += farWeight;
            }
            if (score < bestScore) {
              score += x658_ * mgr.Random()->Float();
              if (score < bestScore) {
                bestScore = score;
                target = cover;
                mPlayerInLeashRange = playerDistance > mLeashRadius;
              }
            }
          }
        }
      }
    }
  }
  if (target) {
    mDestObj = target->GetUniqueId();
    SetDestPos(target->GetTranslation());
    ReleaseCoverPoint(mgr, mCoverPoint, true);
    SetCoverPoint(target, mCoverPoint);
  } else if (mgr.GetPlayer(0)->GetCurrentAreaId() == GetCurrentAreaId()) {
    mDestObj = mgr.GetPlayer(0)->GetUniqueId();
    CVector3f destPos =
        mgr.GetPlayer(0)->GetTranslation() -
        x654_ * (mgr.GetPlayer(0)->GetTranslation() - GetTranslation()).AsNormalized();
    const CRayCastResult result =
        mgr.RayStaticIntersection(destPos, CVector3f::Down(), 8.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(AnchorFloorMaterial)));
    if (result.IsValid()) {
      destPos = result.GetPoint();
    }
    SetDestPos(destPos);
  } else {
    mDestObj = kInvalidUniqueId;
    mDestPos = GetTranslation();
  }
}

const CChozoGhost::CBehaveChance& CChozoGhost::ChooseBehaveChanceRange(CStateManager& mgr) const {
  const float dist = (GetTranslation() - mgr.GetPlayer(0)->GetTranslation()).Magnitude();
  if (dist < x654_) {
    return mBehaveChance1;
  }
  if (dist < x658_) {
    return mBehaveChance2;
  }

  return mBehaveChance3;
}

static EMaterialTypes WarpSolidMaterial = kMT_Solid;

void CChozoGhost::SetWarpPosition(CStateManager& mgr, const CVector3f& dir) {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  float distance = 8.f;
  const CRayCastResult result =
      mgr.RayStaticIntersection(center + distance * dir, -dir, distance,
                                CMaterialFilter::MakeInclude(CMaterialList(WarpSolidMaterial)));
  if (result.IsValid()) {
    mSpaceWarpPosition = result.GetPoint();
  } else {
    mSpaceWarpPosition = center + dir;
  }
}

void CChozoGhost::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    if (!mBodyController->GetIsActive()) {
      mBodyController->Activate(mgr, pas::kAS_Invalid);
    }

    if (x63c_ == 3) {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
      mColor.SetAlpha(1.f);
    } else {
      mBodyController->SetLocomotionType(pas::kLT_Relaxed);
      mColor.SetAlpha(0.f);
    }
    RemoveMaterial(kMT_Solid, mgr);
    SetMomentumWR(CVector3f::Zero());
    x665_24_ = true;
  } break;
  case kStateMsg_Update:
    break;
  default:
    break;
  }
}

bool CChozoGhost::AIStage(CStateManager& mgr, const CTriggerData& data) const {
  const float arg = data.GetFloat();
  return static_cast< int >(arg) == x63c_;
}

void CChozoGhost::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateMachine->SetDelay(mFadeOutDelay);
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    mAlphaDelta = 1.f;
    mFadedIn = true;
    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Up());
    }
  } break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x665_24_ = false;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    break;
  }
}

static EMaterialTypes GenerateFloorMaterial = kMT_Floor;

void CChozoGhost::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateMachine->SetDelay(mFadeOutDelay);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mOnGround = false;
    const CRayCastResult result = mgr.RayStaticIntersection(
        GetTranslation(), CVector3f::Down(), 100.f,
        CMaterialFilter::MakeInclude(CMaterialList(GenerateFloorMaterial)));
    if (result.IsValid()) {
      mFloorLevel = result.GetPoint().GetZ();
    } else {
      mFloorLevel = mgr.GetPlayer(0)->GetTranslation().GetZ();
    }

    mAlphaDelta = 1.f;
    mFadedIn = true;

    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Down());
    }
  } break;
  case kStateMsg_Update: {
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Jump)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCJumpCmd(mDestPos, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    switch (mAnimationState.GetState()) {
    case CAnimationState::kAS_Repeat: {
      mBodyController->SetLocomotionType(pas::kLT_Crouch);
      if (mOnGround) {
        break;
      }

      if (GetTranslation().GetZ() < mFloorLevel + x668_) {
        CVector3f newPos = GetTranslation();
        newPos.SetZ(mFloorLevel + x668_);
        SetTranslation(newPos);
        mOnGround = true;
      }
    } break;
    case CAnimationState::kAS_Over: {
      mBoneTracking.SetActive(true);
      mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
      FloatToLevel(mFloorLevel, arg);
    } break;
    default:
      break;
    }
  } break;
  case kStateMsg_Deactivate: {
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    x665_24_ = false;
    mOnGround = false;
  } break;
  }
}

void CChozoGhost::WallDetach(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateMachine->SetDelay(mFadeOutDelay);
    mAlphaDelta = 1.f;
    mFadedIn = false;

    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, GetTransform().GetForward());
    }

    const CActor* wp = nullptr;
    const TUniqueId wpId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    if (wpId != kInvalidUniqueId) {
      wp = TCastToConstPtr< CActor >(mgr.GetObjectById(wpId));
    }

    if (wp) {
      SetDestPos(wp->GetTranslation());
    } else {
      SetDestPos(GetTranslation() + (2.f * x66c_) * GetTransform().GetForward());
    }

    SendScriptMsgs(kSS_Attack, mgr, kInvalidUniqueId, kSM_Follow);
  } break;
  case kStateMsg_Update: {

  } break;
  case kStateMsg_Deactivate: {
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    x665_24_ = false;
    mBehaveType = kBT_Move;
  } break;
  }
}

void CChozoGhost::Run(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    mHitByPlayerProjectile = false;
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
    mInRange = false;
  } break;
  case kStateMsg_Update: {
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(mSteeringBehaviors.Seek(*this, mDestPos), CVector3f::Zero(), 1.f));
    if (!mShouldSwoosh) {
      break;
    }

    mFloorLevel = mDestPos.GetZ();
    FloatToLevel(mFloorLevel, arg);
    AnimationData()->SetEffectState(skSpeedSwooshName, true, mgr);
    x665_24_ = false;
    if (mInRange) {
      break;
    }

    const float movement = arg * GetVelocityWR().Magnitude();
    const float range = x66c_ + 2.5f * movement;
    const CVector3f& delta = GetTranslation() - mDestPos;
    mInRange = delta.MagSquared() < range * range;
  } break;
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(pas::kLT_Crouch);
    SetDestPos(mgr.GetPlayer(0)->GetTranslation());
    AnimationData()->SetEffectState(skSpeedSwooshName, false, mgr);
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, true);
    mInRange = false;
    break;
  }
}

bool CChozoGhost::InRange(CStateManager& mgr, const CTriggerData& data) const { return mInRange; }

void CChozoGhost::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    FindBestAnchor(mgr);
    break;
  }
  }
}

bool CChozoGhost::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mBehaveType == kBT_Attack;
}

static EMaterialTypes AttackSolidMaterial = kMT_Solid;

void CChozoGhost::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamMgr, GetUniqueId());
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    switch (x6d8_) {
    case 1:
      mAttackType = 3;
      break;
    case 2:
      mAttackType = 4;
      break;
    case 3:
      mAttackType = 5;
      break;
    }
    if (x665_25_) {
      const CRayCastResult result = mgr.RayStaticIntersection(
          GetTranslation() + 0.5f * CVector3f::Up(), CVector3f::Up(), x670_,
          CMaterialFilter::MakeInclude(CMaterialList(AttackSolidMaterial)));
      if (!result.IsValid()) {
        mAttackType = 2;
        KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, false);
      }
    }
    SetMomentumWR(CVector3f::Zero());
    SetConstantForceWR(CVector3f::Zero());
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_MeleeAttack)) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCMeleeAttackCmd(static_cast< pas::ESeverity >(mAttackType)));
    }
    mBodyController->CommandMgr().SetTargetVector(mgr.GetPlayer(0)->GetTranslation() -
                                                  GetTranslation());
    if (mAttackType != 2) {
      FloatToLevel(mFloorLevel, arg);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mShouldSwoosh = false;
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, true);
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamMgr, GetUniqueId(),
                                true);
    break;
  }
}

void CChozoGhost::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Update: {
    FloatToLevel(mFloorLevel, arg);
    if (CMath::AbsF(mFloorLevel - GetTranslation().GetZ()) < 0.05f) {
      StateMachineState().SetCodeTrigger();
    }
    break;
  }
  }
}

void CChozoGhost::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CBehaveChance& chance = ChooseBehaveChanceRange(mgr);
    const CTeamAiRole* role = CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId());
    if (role && (role->GetTeamAiRole() != CTeamAiRole::kTAR_Projectile ||
                 !CScriptTeamAiMgr::CanStartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamMgr,
                                                   GetUniqueId()))) {
      mBehaveType = kBT_Attack;
    }
    mBehaveType = ChooseBehaveChanceRange(mgr).GetBehave(mBehaveType, mgr);
    switch (mBehaveType) {
    case kBT_Lurk:
      mLurkDelay = chance.GetLurkTime();
      break;
    case kBT_Attack:
      x665_25_ = mgr.Random()->Float() < chance.GetChargeAttack();
      x6d8_ = mgr.Random()->Next() % chance.GetNumBolts() + 1;
      break;
    default:
      break;
    }
    x664_31_ = false;
    mPlayerInLeashRange = false;
    break;
  }
  }
}

bool CChozoGhost::ShouldTaunt(CStateManager& mgr, const CTriggerData& data) const {
  return mBehaveType == kBT_Taunt;
}

void CChozoGhost::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Taunt)) {
      mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
    }
    FloatToLevel(mFloorLevel, arg);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mShouldSwoosh = false;
    break;
  }
}

static EMaterialTypes HurledFloorMaterial = kMT_Floor;

void CChozoGhost::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mVerticalMovement = false;
    mOnGround = false;
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    mAlphaDelta = 2.f;
    if (!mOnGround) {
      if (GetVelocityWR().GetZ() < 0.f) {
        const CRayCastResult result = mgr.RayStaticIntersection(
            GetTranslation() + CVector3f::Up(), CVector3f::Down(), 2.f,
            CMaterialFilter::MakeInclude(CMaterialList(HurledFloorMaterial)));
        if (result.IsValid() && result.GetTime() < 1.05f) {
          mOnGround = true;
          SetMomentumWR(CVector3f::Zero());
          SetVelocityWR(CVector3f(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f));
          mFloorLevel = result.GetPoint().GetZ();
          StateMachineState().SetCodeTrigger();
        }
      }
      if (!mOnGround && mStateMachine->GetTime() > mHurlRecoverTime) {
        mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
        mBodyController->SetLocomotionType(pas::kLT_Lurk);
        StateMachineState().SetCodeTrigger();
      }
    }
    break;
  case kStateMsg_Deactivate:
    mVerticalMovement = true;
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

void CChozoGhost::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateMachine->SetDelay(mLurkDelay);
    break;
  case kStateMsg_Update:
    FloatToLevel(mFloorLevel, arg);
    break;
  }
}

bool CChozoGhost::ShouldMove(CStateManager& mgr, const CTriggerData& data) const {
  return mBehaveType == kBT_Move;
}

bool CChozoGhost::Leash(CStateManager& mgr, const CTriggerData& data) const {
  return mPlayerInLeashRange || CPatterned::Leash(mgr, data);
}

bool CChozoGhost::ShouldFlinch(CStateManager& mgr, const CTriggerData& data) const {
  return mFlinch;
}

bool CChozoGhost::AggressionCheck(CStateManager& mgr, const CTriggerData& data) const {
  return mAggressive;
}

void CChozoGhost::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking.SetActive(false);
    ReleaseCoverPoint(mgr, mCoverPoint, true);
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, pas::kAS_Generate)) {
      mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Repeat) {
      mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CChozoGhost::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    ReleaseCoverPoint(mgr, mCoverPoint, true);
    mAlphaDelta = 4.f;
    mFadedOut = false;
    mFadedIn = false;
    mBoneTracking.SetActive(false);
    Stop();
    break;
  case kStateMsg_Update:
    Stop();
    SetMomentumWR(CVector3f::Zero());
    break;
  }
}

CProjectileInfo* CChozoGhost::ProjectileInfo() {
  if (mAttackType == 2) {
    return &mProjectileInfo1;
  }
  return &mProjectileInfo2;
}

void CChozoGhost::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
    const CTransform4f projectileXf = CTransform4f::LookAt(locator.GetTranslation(), aimPos);
    switch (mAttackType) {
    case 2: {
      CEnergyProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 2, CWeapon::kPA_BigStrike | CWeapon::kPA_StaticInterference, true,
          CImpactVisorEffect::ParticleEffect(mProjectileVisor, mSoundProjectileVisor, false),
          CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    default: {
      CEnergyProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 5, CWeapon::kPA_DamageFalloff | CWeapon::kPA_StaticInterference, true,
          CImpactVisorEffect::ParticleEffect(mProjectileVisor, mSoundProjectileVisor, false),
          CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        const float speed = ProjectileInfo()->GetProjectileSpeed();
        if (speed > 0.f) {
          projectile->SetDamageFalloffSpeed(80.f / speed);
        }
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    }
    handled = true;
    break;
  }
  case kUE_FadeIn:
    if (mFadedOut) {
      mAlphaDelta = 2.f;
      CSfxManager::AddEmitter(mSfxFadeIn, GetTranslation(), GetCurrentAreaId().Value(), true,
                              false);
    }
    AddMaterial(kMT_Target, mgr);
    mFadedOut = false;
    mFadedIn = true;
    handled = true;
    break;
  case kUE_FadeOut:
    if (mFadedIn) {
      mAlphaDelta = -2.f;
      CSfxManager::AddEmitter(mSfxFadeOut, GetTranslation(), GetCurrentAreaId().Value(), true,
                              false);
    }
    RemoveMaterial(kMT_Target, mgr);
    mFadedIn = false;
    mFadedOut = true;
    mShouldSwoosh = true;
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
  if (type == kUE_Delete) {
    mAlphaDelta = -1.f;
  }
}

void CChozoGhost::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (!GetAlive()) {
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Hurled, false);
  } else if (!KnockBackController().IsAnimReactionEnabled(CKnockBackMgr::kAR_KnockBack) &&
             info.GetDamageInfo().GetWeaponMode().IsCharged()) {
    KnockBackController().SetAnimReactionRange(CKnockBackMgr::kAR_Hurled, CKnockBackMgr::kAR_Fall);
  }
  CPatterned::KnockBack(mgr, info);
  KnockBackController().SetAnimReactionRange(CKnockBackMgr::kAR_Flinch, CKnockBackMgr::kAR_Fall);
  if (GetAlive()) {
    if (KnockBackController().GetAnimReaction() == CKnockBackMgr::kAR_Hurled) {
      mStateMachine->SetState(mgr, *this, rstl::string_l("Hurled"));
    }
  } else {
    Stop();
    SetMomentumWR(CVector3f::Zero());
  }
}

void CChozoGhost::PreThink(float dt, CStateManager& mgr) {
  mBoneTracking.PreThink(*AnimationData());
  CPatterned::PreThink(dt, mgr);
}

void CChozoGhost::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  mBoneTracking.Think(dt);
  mSpaceWarpTime = rstl::max_val(mSpaceWarpTime - dt, 0.f);
  SetValidTarget(0, IsVisibleEnough(mgr));
}

void CChozoGhost::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

CEntity* LoadChozoGhost(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrChozoGhost sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrChozoGhost.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CChozoGhost::CBehaveChance nearChance(
      sldrThis.near.lurk, sldrThis.near.heckle, sldrThis.near.attack, sldrThis.near.move,
      sldrThis.near.lurkTime, sldrThis.near.chargeAttack, sldrThis.near.numBolts);
  const CChozoGhost::CBehaveChance midChance(
      sldrThis.mid.lurk, sldrThis.mid.heckle, sldrThis.mid.attack, sldrThis.mid.move,
      sldrThis.mid.lurkTime, sldrThis.mid.chargeAttack, sldrThis.mid.numBolts);
  const CChozoGhost::CBehaveChance farChance(
      sldrThis.far.lurk, sldrThis.far.heckle, sldrThis.far.attack, sldrThis.far.move,
      sldrThis.far.lurkTime, sldrThis.far.chargeAttack, sldrThis.far.numBolts);

  return rs_new CChozoGhost(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.hearingRadius,
      sldrThis.fadeOutDelay, sldrThis.attackDelay, sldrThis.freezeTime, sldrThis.unknown_0x54151870,
      LdrToDamageInfo(sldrThis.damageInfo), sldrThis.unknown_0x3a58089c,
      LdrToDamageInfo(sldrThis.damageInfo_0x1ff047a9), nearChance, midChance, farChance,
      sldrThis.sound_Impact, sldrThis.disablePlayerGunTime, sldrThis.sound_PhazeIn,
      sldrThis.sound_PhazeOut, sldrThis.unknown_0xec76940c, sldrThis.projectileStopHomingRange,
      sldrThis.unknown_0xfe9eac26, sldrThis.hurlRecoverTime, sldrThis.projectileVisorEffect,
      sldrThis.sound_ProjectileVisor, sldrThis.nearToMidDistance, sldrThis.midToFarDistance,
      sldrThis.nearChance, sldrThis.midChance);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SChozoGhost_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadChozoGhost;
  SetSChozoGhost_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSChozoGhost_FuncPtrs(nullptr); }
#endif
