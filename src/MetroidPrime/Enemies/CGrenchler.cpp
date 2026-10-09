#include "MetroidPrime/Enemies/CGrenchler.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CLineSeg.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CGenericFSM2.hpp"
#include "MetroidPrime/Enemies/CGrenchlerTail.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGrenchler.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CElectricBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "REL/REL_Setup.h"
#include "rstl/math.hpp"

#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "REL/REL_Setup.h"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/math.hpp"
#include <float.h>
#include <math.h>

CVector3f gGrenchlerAimOffset(0.f, 0.f, 0.5f); // Guessed name

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"AbortCharge", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::AbortCharge)},
    {"Alerted", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Alerted)},
    {"Attacked", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Attacked)},
    {"AttackPatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::AttackPatternOver)},
    {"BeamBadAngle",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::BeamBadAngle)},
    {"BeamHitPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::BeamHitPlayer)},
    {"BeamHitSticky",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::BeamHitSticky)},
    {"BeamHitWall", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::BeamHitWall)},
    {"Bored", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Bored)},
    {"BreakGrappleLoop",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::BreakGrappleLoop)},
    {"CanBeamAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CanBeamAttack)},
    {"CanBite", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CanBite)},
    {"CanBurstAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CanBurstAttack)},
    {"CancelManeuvering",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CancelManeuvering)},
    {"CanCharge", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CanCharge)},
    {"CanGrapple", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CanGrapple)},
    {"ChargeFinished",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ChargeFinished)},
    {"ClearLineOfFire",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ClearLineOfFire)},
    {"ClearPathToPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ClearPathToPlayer)},
    {"CrystalDamaged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::CrystalDamaged)},
    {"EmergedFromWater",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::EmergedFromWater)},
    {"FacingJumpEnd",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::FacingJumpEnd)},
    {"FacingPlayer",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::FacingPlayer)},
    {"ForceGrapple",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ForceGrapple)},
    {"Frustrated", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Frustrated)},
    {"JumpLanded", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::JumpLanded)},
    {"JustBeamAttacked",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::JustBeamAttacked)},
    {"JustBiteAttacked",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::JustBiteAttacked)},
    {"JustBurstAttacked",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::JustBurstAttacked)},
    {"JustHit", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::JustHit)},
    {"GrapplingMorphball",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::GrapplingMorphball)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::HasAttackPattern)},
    {"HasValidJumpTarget",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::HasValidJumpTarget)},
    {"InBeamRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::InBeamRange)},
    {"InBiteRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::InBiteRange)},
    {"InBurstRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::InBurstRange)},
    {"InChargeRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::InChargeRange)},
    {"ManeuverDone",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ManeuverDone)},
    {"PauseOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::PauseOver)},
    {"PlayerBehindMe",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::PlayerBehindMe)},
    {"PlayerStuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::PlayerStuck)},
    {"PlayerSubmerged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::PlayerSubmerged)},
    {"PlayYellowHitReact",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::PlayYellowHitReact)},
    {"ReturnToPatrol",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ReturnToPatrol)},
    {"ShouldBackstep",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ShouldBackstep)},
    {"ShouldSlide", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ShouldSlide)},
    {"ShouldTurn", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::ShouldTurn)},
    {"SlideOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::SlideOver)},
    {"SlideStop", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::SlideStop)},
    {"StopStruggling",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::StopStruggling)},
    {"StruggleOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::StruggleOver)},
    {"Stuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Stuck)},
    {"Submerged", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::Submerged)},
    {"TailIntact", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::TailIntact)},
    {"TookDamage", static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::TookDamage)},
    {"TookKnockback",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::TookKnockback)},
    {"TooMuchTurning",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CGrenchler::TooMuchTurning)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Backstep", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Backstep)},
    {"BeamAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::BeamAttack)},
    {"BiteAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::BiteAttack)},
    {"BurstAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::BurstAttack)},
    {"Charge", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Charge)},
    {"ChargeFailed", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::ChargeFailed)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Dead)},
    {"FollowAttackPattern",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::FollowAttackPattern)},
    {"GrappleAbort", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleAbort)},
    {"GrappleBite", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleBite)},
    {"GrappleBreak", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleBreak)},
    {"GrappleLoop", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleLoop)},
    {"GrapplePull", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrapplePull)},
    {"GrappleSlide", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleSlide)},
    {"GrappleSlideBonk",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleSlideBonk)},
    {"GrappleStruggle",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::GrappleStruggle)},
    {"Jump", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Jump)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Lurk)},
    {"Maneuver", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Maneuver)},
    {"MorphballBite",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::MorphballBite)},
    {"Null", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Null)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Patrol)},
    {"PauseBetweenBeams",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::PauseBetweenBeams)},
    {"PauseBetweenBites",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::PauseBetweenBites)},
    {"Pursue", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Pursue)},
    {"ShakeOff", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::ShakeOff)},
    {"StopBeamAttack",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::StopBeamAttack)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Taunt)},
    {"Turn", static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::Turn)},
    {"TurnToJumpEnd",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::TurnToJumpEnd)},
    {"WalkTowardPlayer",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::WalkTowardPlayer)},
    {"YellowHitReact",
     static_cast< CPatterned::StateMachine::StateFunc >(&CGrenchler::YellowHitReact)},
};

static CPatterned::StateMachine::SCodeFunction skCodeFuncs[] = {
    {"PickJumpTarget",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CGrenchler::PickJumpTarget)},
    {"SetLastActionAsBeam",
     static_cast< CPatterned::StateMachine::CodeFunc >(&CGrenchler::SetLastActionAsBeam)},
};

CGrenchler::SReflectInfo::SReflectInfo()
: x4_(CVector3f::Zero())
, x10_(CVector3f::Zero())
, mVulnerability(CDamageVulnerability::ReflectVulnerabilty())
, x50_(-1000.f) {
  Reset();
}

CGrenchler::SBiteAttack::SBiteAttack(const CDamageInfo& damage, float minRange, float maxRange,
                                     float minPause, float maxPause, float damageRadius)
: x0_(false)
, mDamage(damage)
, mMinRange(minRange)
, mMaxRange(maxRange)
, mMinPause(minPause)
, mMaxPause(maxPause)
, x30_(CTransform4f::Identity())
, mDamageRadius(damageRadius)
, x64_(-1000.f) {}

CGrenchler::SAttackBase::SAttackBase(const CDamageInfo& damage, float minRange, float maxRange,
                                     float minPause, float maxPause)
: mDamage(damage)
, mMinRange(minRange)
, mMaxRange(maxRange)
, mMinPause(minPause)
, mMaxPause(maxPause) {}

CGrenchler::SBeamAttack::SBeamAttack(const CDamageInfo& damage, const SLdrAudioPlaybackParms& sound,
                                     float minRange, float maxRange, float minPause, float maxPause,
                                     float maxAngle)
: SAttackBase(damage, minRange, maxRange, minPause, maxPause)
, x2c_(false)
, mBeamDamage(damage)
, x4c_(-1000.f)
, x50_(CVector3f::Zero())
, mMaxAngle(maxAngle)
, mSound(sound)
, x78_()
, x7c_(CTransform4f::Identity()) {}

CGrenchler::SBurstAttack::SBurstAttack(CAssetId projectile, const CDamageInfo& damage,
                                       float minRange, float maxRange, float minPause,
                                       float maxPause, float damageRadius)
: SAttackBase(damage, minRange, maxRange, minPause, maxPause)
, x2c_(false)
, mProjectile(projectile)
, x34_(0.f)
, mDamageRadius(damageRadius)
, x3c_(-1000.f) {}

CGrenchler::SChargeAttack::SChargeAttack(float minTimeBetweenCharges, float unknown, float minRange,
                                         float maxRange)
: x0_(CVector3f::Zero())
, xc_(CVector3f::Zero())
, x18_(minRange)
, x1c_(maxRange)
, x24_(-1000.f)
, x28_(1.f)
, x2c_(minTimeBetweenCharges)
, x30_(unknown) {
  x0_ = xc_ = CVector3f::Zero();
  x20_ = 0.f;
}

CGrenchler::SGrappleEffect::SGrappleEffect(CAssetId id)
: x0_(id)
, x4_(id != kInvalidAssetId ? rstl::optional_object< TToken< CGenDescription > >(
                                  gpSimplePool->GetObj(SObjectTag('PART', id)))
                            : rstl::optional_object< TToken< CGenDescription > >()) {}

CGrenchler::SEffectA::SEffectA(CAssetId surfaceRings, CAssetId electric, CAssetId beam,
                               CAssetId hitFx, CAssetId eyeGlow)
: x0_(surfaceRings)
, x4_(rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', surfaceRings)),
                         CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, xc_(electric)
, x10_(gpSimplePool->GetObj(SObjectTag('ELSC', xc_)))
, x18_(kInvalidUniqueId)
, x1c_(gpSimplePool->GetObj(SObjectTag('WPSC', beam)))
, x24_(hitFx)
, x28_(hitFx != kInvalidAssetId ? rstl::optional_object< TLockedToken< CGenDescription > >(
                                      gpSimplePool->GetObj(SObjectTag('PART', hitFx)))
                                : rstl::optional_object< TLockedToken< CGenDescription > >())
, x38_(kInvalidUniqueId)
, x3c_(eyeGlow == kInvalidAssetId
           ? nullptr
           : rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', eyeGlow)),
                                CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x44_(0.f) {
  x10_.Lock();
  x1c_.Lock();
}

CGrenchler::SEffectB::SEffectB(CAssetId a, CAssetId b, float c)
: xc_(a)
, x10_(c)
, x1c_(-1000.f)
, x28_(-1)
, x2c_(b)
, x30_(kInvalidUniqueId)
, x34_(b != kInvalidAssetId ? rstl::optional_object< TToken< CGenDescription > >(
                                  gpSimplePool->GetObj(SObjectTag('PART', b)))
                            : rstl::optional_object< TToken< CGenDescription > >()) {
  x0_0_ = x0_1_ = x0_2_ = false;
  x8_ = x14_ = x18_ = x20_ = x24_ = 0.f;
  x4_ = 0;
}

CGrenchler::SEffectC::SEffectC(CAssetId swoosh, CAssetId beamPart, const CDamageInfo& damage,
                               const SLdrAudioPlaybackParms& sound)
: x0_(CVector3f::Zero())
, xc_(0.f)
, x10_(swoosh)
, x2c_(beamPart)
, x38_(CVector3f::Zero())
, x44_(0.f)
, x48_(0)
, x4c_(CVector3f::Zero())
, x58_(damage)
, x78_(sound)
, x90_() {}

CGrenchler::SDamageEffect::SDamageEffect(const CDamageInfo& damage, CAssetId effect)
: mDamage(damage)
, mEffect(effect != kInvalidAssetId ? rstl::optional_object< TToken< CGenDescription > >(
                                          gpSimplePool->GetObj(SObjectTag('PART', effect)))
                                    : rstl::optional_object< TToken< CGenDescription > >()) {}

CGrenchler::SVectorTriple::SVectorTriple()
: x0_(CVector3f::Zero()), xc_(CVector3f::Zero()), x18_(CVector3f::Zero()) {}

CGrenchler::SEffectD::SEffectD(CAssetId effect)
: x0_(effect != kInvalidAssetId
          ? rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', effect)),
                               CElementGen::kMOT_Normal, CElementGen::kOSF_One)
          : nullptr)
, x8_(CVector3f::Zero())
, x14_(CVector3f::Zero())
, x20_(0.f) {}

static const CGrenchler::SJointInfo skJointInfo[] = {
    {"Skeleton_Root", 1.3f, 1.6f, 0, kWCR_Unknown75, true},
    {"R_hip", 0.9f, 0.9f, 0, kWCR_Unknown75, true},
    {"L_hip", 0.9f, 0.9f, 0, kWCR_Unknown75, true},
    {"tailbone_1", 0.6f, 0.9f, 0, kWCR_Unknown75, true},
    {"tailbone_2", 0.45f, 0.55f, 0, kWCR_Unknown75, false},
    {"horn_LCTR", 0.8f, 0.7f, 1, kWCR_Unknown105, true},
    {"eye", 0.8f, 0.9f, 1, kWCR_Unknown105, true},
    {"jaw", 1.2f, 1.1f, 2, kWCR_Unknown105, true},
    {"R_knee", 0.9f, 0.9f, 2, kWCR_Unknown105, true},
    {"L_knee", 0.9f, 0.9f, 2, kWCR_Unknown105, true},
};

// Guessed name; per-joint collision actor table (shared with the constructor).
struct SCollisionJoint {
  const char* x0_;
  float x4_;
  float x8_;
  int xc_;
  EWeaponCollisionResponseTypes x10_;
  bool mAffectsBounds;
};

static const SCollisionJoint skCollisionJoints[10] = {
    {"Skeleton_Root", 1.3f, 1.6f, 0, kWCR_Unknown75, true},
    {"R_hip", 0.9f, 0.9f, 0, kWCR_Unknown75, true},
    {"L_hip", 0.9f, 0.9f, 0, kWCR_Unknown75, true},
    {"tailbone_1", 0.6f, 0.9f, 0, kWCR_Unknown75, true},
    {"tailbone_2", 0.45f, 0.55f, 0, kWCR_Unknown75, false},
    {"horn_LCTR", 0.8f, 0.7f, 1, kWCR_Unknown105, true},
    {"eye", 0.8f, 0.9f, 1, kWCR_Unknown105, true},
    {"jaw", 1.2f, 1.1f, 2, kWCR_Unknown105, true},
    {"R_knee", 0.9f, 0.9f, 2, kWCR_Unknown105, true},
    {"L_knee", 0.9f, 0.9f, 2, kWCR_Unknown105, true},
};

static EMaterialTypes sSolidMaterial = kMT_Solid; // Guessed name

CAABox CGrenchler::GetSortingBounds(const CStateManager& mgr) const {
  if (!mIsGrappleGuardian || !mGuardianBounds) {
    return CActor::GetSortingBounds(mgr);
  }
  return *mGuardianBounds;
}

void CGrenchler::PreRenderAllViewports(CStateManager& mgr) {
  CPatterned::PreRenderAllViewports(mgr);
  if (mIsGrappleGuardian == 1 && mCollisionManager.get()) {
    mGuardianBounds = CAABox::MakeMaxInvertedBox();
    for (uint i = 0; i < 10; ++i) {
      if (skCollisionJoints[i].mAffectsBounds) {
        const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
        if (const CCollisionActor* actor =
                TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(desc.GetCollisionActorId()))) {
          CAABox bounds = *actor->GetTouchBounds();
          CVector3f halfExtent = bounds.GetMaxPoint() - bounds.GetCenterPoint();
          halfExtent *= 0.85f;
          CVector3f max = bounds.GetCenterPoint() + halfExtent;
          CVector3f min = bounds.GetCenterPoint() - halfExtent;
          mGuardianBounds->Include(CAABox(min, max));
        }
      }
    }
  }
}

void CGrenchler::OnScanStateChange(EScanState state, CStateManager& mgr) {
  switch (state) {
  case kSS_Done:
    mX90c_5_ = true;
    break;
  default:
    break;
  }
  CActor::OnScanStateChange(state, mgr);
}

CDamageInfo CGrenchler::GetContactDamage() const {
  if (mIsGrappleGuardian == 1 && GetAlive() == 1 && xc6c_ == 1) {
    return mDamageEffect.mDamage;
  }
  return CPatterned::GetContactDamage();
}

void CGrenchler::EndMorphballCapture(CStateManager& mgr) {
  if (mXea0_1_) {
    mXea0_1_ = false;
    SendScriptMsgs(kSS_InternalState01, mgr, GetUniqueId(), kSM_None);
  }
}

void CGrenchler::BeginMorphballCapture(CStateManager& mgr) {
  if (mXea0_1_ != 1) {
    mXea0_1_ = true;
    SendScriptMsgs(kSS_InternalState00, mgr, GetUniqueId(), kSM_None);
  }
}

void CGrenchler::UpdateCapturedPlayer(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (!mXea0_2_) {
    const CVector3f mouth = mBeamAttack.x7c_.GetTranslation();
    const CVector3f anchor = xe70_.GetTranslation();
    if ((mouth - anchor).MagSquared() < (mouth - player->GetTranslation()).MagSquared()) {
      mXea0_2_ = true;
    }
  }
  if (mXea0_2_) {
    CTransform4f xf = xe70_;
    xf.AddTranslationX(0.f);
    xf.AddTranslationY(0.f);
    xf.AddTranslationZ(-0.5f);
    player->Teleport(xf, mgr, false);
  }
}

void CGrenchler::MorphballBite(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_MorphballBite, msg);
  SetSubmerged(false);
  CPlayer* player = mgr.GetPlayer(0);
  switch (msg) {
  case kStateMsg_Activate:
    mX90c_0_ = false;
    mXea0_0_ = false;
    mXea0_2_ = false;
    xe6c_ = 0;
    player->EnableLeaveMorphBall(false);
    break;
  case kStateMsg_Update:
    if (mXea0_0_ == 1) {
      UpdateCapturedPlayer(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    EndMorphballCapture(mgr);
    mX90c_0_ = true;
    xeac_ = 0;
    mEffectB.x1c_ = x8ac_;
    player->EnableLeaveMorphBall(true);
    break;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Six));
}

bool CGrenchler::GrapplingMorphball(CStateManager& mgr, const CTriggerData& data) const {
  if (xa78_ != 8) {
    return false;
  }
  return mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
}

bool CGrenchler::Stuck(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::Stuck(mgr, data);
}

bool CGrenchler::PlayerStuck(CStateManager& mgr, const CTriggerData& data) const {
  return mEffectB.x18_ > 1.5f;
}

bool CGrenchler::CrystalDamaged(CStateManager& mgr, const CTriggerData& data) const {
  if (mEffectB.x0_2_ != 1) {
    return false;
  }
  return xe5c_ > xe64_;
}

void CGrenchler::UpdateDamageFlash(float dt) {
  if (xe58_ > 0.f) {
    xe58_ = rstl::max_val(xe58_ - dt, 0.f);
    const float t = CMath::Min(xe58_ / skDamageHitTime, 1.f);
    const CColor& color = CColor::Lerp(CColor::Black(), CColor::Yellow(), t);
    mColor.SetRed(color.GetRedu8());
    mColor.SetGreen(color.GetGreenu8());
    mColor.SetBlue(color.GetBlueu8());
  }
}

bool CGrenchler::Frustrated(CStateManager& mgr, const CTriggerData& data) const {
  return xa7c_ > 1.f;
}

void CGrenchler::TakeDamage(const CVector3f& direction, float magnitude) {
  mDamageCooldownTimer = skDamageHitTime;
  if (!mIsGrappleGuardian && xc78_ + 2.f > x8ac_) {
    HealthInfo()->SetHP(GetTailHealth());
  }
}

bool CGrenchler::TookKnockback(CStateManager& mgr, const CTriggerData& data) const {
  return GetBodyController()->GetCurrentStateId() == pas::kAS_KnockBack;
}

bool CGrenchler::StruggleOver(CStateManager& mgr, const CTriggerData& data) const {
  if (mEffectB.x0_1_ == 1) {
    return true;
  }
  return TookKnockback(mgr, data);
}

bool CGrenchler::StopStruggling(CStateManager& mgr, const CTriggerData& data) const {
  if (mEffectB.x4_ >= mEffectB.xc_) {
    return true;
  }
  return mEffectB.x10_ + GetHealthInfo()->GetHP() < mEffectB.x8_;
}

void CGrenchler::GrappleAbort(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrappleAbort, msg);
  BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  if (msg == kStateMsg_Activate) {
    DestroyGrappleBeam(mgr);
    mX90c_0_ = false;
  } else if (msg == kStateMsg_Deactivate) {
    mX90c_0_ = true;
    xeac_ = 0;
    mEffectB.x1c_ = x8ac_;
  }
}

void CGrenchler::GrappleBreak(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrappleBreak, msg);
  BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  if (msg == kStateMsg_Activate && mEffectC.x48_ == 2) {
    DestroyGrappleBeam(mgr);
  }
  if (msg == kStateMsg_Deactivate) {
    mX90c_0_ = true;
    DestroyGrappleBeam(mgr);
    xeac_ = 0;
    mEffectB.x1c_ = x8ac_;
  }
}

void CGrenchler::IncrementAttached(CStateManager& mgr) { SendAttachedMessage(mgr, kSM_Increment); }

void CGrenchler::DecrementAttached(CStateManager& mgr) { SendAttachedMessage(mgr, kSM_Decrement); }

void CGrenchler::SendAttachedMessage(CStateManager& mgr, EScriptObjectMessage msg) {
  rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
  for (; it != GetConnectionList().end(); ++it) {
    if (it->state == kSS_Connect && it->msg == kSM_Attach) {
      if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(it->objId)))) {
        actor->AcceptScriptMsg(mgr, CScriptMsg(GetUniqueId(), actor->GetUniqueId(), msg));
      }
    }
  }
}

void CGrenchler::GrapplePull(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrapplePull, msg);
  BodyController()->SetLocomotionType(pas::kLT_Internal12);
  BodyController()->CommandMgr().ClearLocomotionCmds();
  CPlayer* player = mgr.GetPlayer(0);
  switch (msg) {
  case kStateMsg_Activate:
    mEffectC.x38_ = mBeamAttack.x7c_.GetTranslation();
    mEffectB.x18_ = 0.f;
    mEffectB.x14_ = (player->GetTranslation() - mBeamAttack.x7c_.GetTranslation()).Magnitude();
    IncrementAttached(mgr);
    player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    mX90c_0_ = false;
    xe5c_ = 0.f;
    xe6c_ = 0;
    break;
  case kStateMsg_Update: {
    const CVector3f pos = mBeamAttack.x7c_.GetTranslation();
    const float distance = (player->GetTranslation() - pos).Magnitude();
    const float lastDistance = (player->GetTranslation() - mEffectC.x38_).Magnitude();
    if (distance + 0.1f < mEffectB.x14_) {
      mEffectB.x14_ = lastDistance;
      mEffectB.x18_ = 0.f;
    } else {
      mEffectB.x18_ += dt;
    }
    const float delta = distance - lastDistance;
    CVector3f direction = mBeamAttack.x7c_.GetTranslation() - player->GetTranslation();
    direction.SetZ(0.f);
    if (direction.CanBeNormalized() == 1) {
      CTransform4f xf = player->GetTransform();
      CVector3f pull = direction.AsNormalized();
      pull *= 1.9f * dt;
      if (distance > lastDistance) {
        CVector3f extra = direction.Normalize() * (1.5f * delta);
        if (player->GetBackwardInput() > 0.1f) {
          extra *= 0.35f;
        }
        xf.AddTranslation(pull);
        xf.AddTranslation(extra);
      } else {
        if (player->GetBackwardInput() > 0.1f) {
          pull *= 0.35f;
        }
        xf.AddTranslation(pull);
      }
      PullPlayer(dt, mgr, xf.GetTranslation());
      mEffectC.x74_ += dt;
      while (mEffectC.x74_ > 0.25f) {
        mEffectC.x74_ -= 0.25f;
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), mEffectC.x58_,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial), CMaterialList()),
            CVector3f::Zero());
      }
    }
    mEffectC.x0_ = GetPlayerTargetPosition(mgr);
    mEffectC.xc_ = (GetPlayerTargetPosition(mgr) - pos).Magnitude();
    if (mgr.GetPlayer(0)->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      mEffectC.xc_ += 1.f;
    }
    mEffectC.x38_ = pos;
    break;
  }
  case kStateMsg_Deactivate:
    DecrementAttached(mgr);
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mX90c_0_ = true;
    if (mgr.GetPlayer(0)->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      DestroyGrappleBeam(mgr);
    }
    break;
  }
}

void CGrenchler::PullPlayer(float dt, CStateManager& mgr, const CVector3f& targetPos) {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f facing = player->GetTransform().GetForward();
  facing.SetZ(0.f);
  if (player->IsOnGround() == 1) {
    player->Stop();
    if (facing.CanBeNormalized()) {
      facing.Normalize();
      player->SetTransform(CTransform4f::LookAt(CVector3f::Zero(), facing, CVector3f::Up()));
      player->SetTranslation(targetPos);
    }
  } else {
    player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  const CRelAngle maxTurn = CRelAngle::FromDegrees(180.f * dt);
  CVector3f toPlayer = mBeamAttack.x7c_.GetTranslation() - player->GetAimPosition(mgr, 0.f);
  if (toPlayer.CanBeNormalized() == 1) {
    const CVector3f dir = toPlayer.AsNormalized();
    const CVector3f forward = player->GetTransform().GetForward();
    CQuaternion rotation =
        CQuaternion::LookAt(CUnitVector3f(dir.GetX(), dir.GetY(), dir.GetZ()),
                            CUnitVector3f(forward.GetX(), forward.GetY(), forward.GetZ()), maxTurn);
    player->RotateInOneFrameOR(
        CQuaternion(rotation.GetScalar(),
                    player->GetTransform().TransposeRotate(rotation.GetVector())),
        dt);
  }
}

void CGrenchler::GrappleStruggle(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrappleStruggle, msg);
  BodyController()->SetLocomotionType(pas::kLT_Internal11);
  BodyController()->CommandMgr().ClearLocomotionCmds();
  switch (msg) {
  case kStateMsg_Activate:
    mEffectB.x8_ = GetHealthInfo()->GetHP();
    mX90c_0_ = false;
    SendScriptMsgs(kSS_Locked, mgr, kInvalidUniqueId, kSM_None);
    xe6c_ = 0;
    break;
  case kStateMsg_Update:
    UpdateBeamTrace(mgr);
    break;
  case kStateMsg_Deactivate:
    SendScriptMsgs(kSS_Unlocked, mgr, kInvalidUniqueId, kSM_None);
    xeac_ = 0;
    break;
  }
}

bool CGrenchler::BeamHitSticky(CStateManager& mgr, const CTriggerData& data) const {
  return mEffectC.x48_ == 3;
}

bool CGrenchler::BeamHitPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return mEffectC.x48_ == 1;
}

bool CGrenchler::BeamBadAngle(CStateManager& mgr, const CTriggerData& data) const {
  if (mEffectB.x24_ > 4.f) {
    return true;
  }
  if (mEffectC.x24_.mGen.get() != nullptr) {
    CVector3f toTarget = mEffectC.x0_ - mBeamAttack.x7c_.GetTranslation();
    if (toTarget.CanBeNormalized() == 1) {
      toTarget.Normalize();
      const CVector2f targetDir = CVector2f(toTarget.GetX(), toTarget.GetY());
      const CVector2f facing =
          CVector2f(GetTransform().GetForward().GetX(), GetTransform().GetForward().GetY());
      return CVector2f::GetAngleDiff(facing, targetDir) > 0.87266463f;
    }
  }
  return false;
}

bool CGrenchler::BeamHitWall(CStateManager& mgr, const CTriggerData& data) const {
  if (mEffectC.xc_ > 90.f) {
    return true;
  }
  return mEffectC.x48_ == 2;
}

void CGrenchler::RemoveExplosion(CStateManager& mgr) {
  if (mEffectA.x38_ != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mEffectA.x38_);
    mEffectA.x38_ = kInvalidUniqueId;
  }
}

void CGrenchler::UpdateExplosionHeight(CStateManager& mgr) {
  if (mEffectA.x38_ != kInvalidUniqueId) {
    if (CExplosion* explosion = TCastToPtr< CExplosion >(mgr.ObjectById(mEffectA.x38_))) {
      CVector3f position = explosion->GetTranslation();
      position.SetZ(mBeamAttack.x7c_.GetTranslation().GetZ());
      explosion->SetTranslation(position);
    }
  }
}

void CGrenchler::SpawnBeamHitEffect(CStateManager& mgr) {
  if (mEffectA.x24_ == kInvalidAssetId) {
    return;
  }

  RemoveExplosion(mgr);
  mEffectA.x38_ = mgr.AllocateUniqueId();
  CTransform4f xf(GetTransform());
  xf.SetTranslation(mEffectC.x4c_);
  CExplosion* explosion = rs_new CExplosion(
      *mEffectA.x28_, mEffectA.x38_,
      CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true),
      rstl::string_l("CollisionEffect"), xf, 0, GetModelData()->GetScale(), CColor::White(), -1);
  mgr.AddObject(explosion);
}

void CGrenchler::UpdateBeamTrace(CStateManager& mgr) {
  static const CMaterialFilter skSolidFilter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_Player, kMT_CollisionActor));

  CVector3f direction = mEffectC.x0_ - mBeamAttack.x7c_.GetTranslation();
  if (!direction.CanBeNormalized()) {
    direction = GetTransform().GetForward();
  }
  direction.Normalize();

  if (mEffectC.x48_ != 1) {
    static const CMaterialFilter skPlayerFilter =
        CMaterialFilter::MakeInclude(CMaterialList(kMT_Player));
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, mBeamAttack.x7c_.GetTranslation(), direction, mEffectC.xc_,
                      skPlayerFilter, nullptr);
    TUniqueId hitId = kInvalidUniqueId;
    const CRayCastResult result =
        CGameCollision::RayDynamicIntersection(mgr, hitId, mBeamAttack.x7c_.GetTranslation(),
                                               direction, mEffectC.xc_, skPlayerFilter, nearList);
    if (hitId != kInvalidUniqueId) {
      mEffectC.x48_ = 1;
      mEffectC.x4c_ = GetHornTargetPosition();
      mEffectC.xc_ += 3.f;
      CreateVisorEffect(mgr);
      return;
    }
  }

  if (mEffectC.x48_ == 0) {
    const CRayCastResult staticResult = mgr.RayStaticIntersection(
        mBeamAttack.x7c_.GetTranslation(), direction, 0.9f * mEffectC.xc_, skSolidFilter);
    if (staticResult.IsValid()) {
      mEffectC.x48_ = 2;
      mEffectC.x4c_ = GetHornTargetPosition();
      mEffectC.xc_ += 3.f;
      return;
    }

    CVector3f sideA(direction.GetY(), -direction.GetX(), 0.f);
    CVector3f sideB(-direction.GetY(), direction.GetX(), 0.f);
    if (sideA.CanBeNormalized() == true) {
      sideA.Normalize();
    }
    if (sideB.CanBeNormalized() == true) {
      sideB.Normalize();
    }

    CVector3f origins[3] = {CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero()};
    origins[0] = mBeamAttack.x7c_.GetTranslation() + sideA * 3.f;
    origins[1] = mBeamAttack.x7c_.GetTranslation() + sideB * 3.f;
    origins[2] = mBeamAttack.x7c_.GetTranslation();
    for (int i = 0; i < 3; ++i) {
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      mgr.BuildNearList(nearList, origins[i], direction, mEffectC.xc_, skSolidFilter, nullptr);
      TUniqueId hitId = kInvalidUniqueId;
      const CRayCastResult result = CGameCollision::RayDynamicIntersection(
          mgr, hitId, origins[i], direction, mEffectC.xc_ - 1.f, skSolidFilter, nearList);
      if (hitId != kInvalidUniqueId) {
        if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(hitId))) {
          mEffectC.x4c_ = result.GetPoint();
          mEffectC.xc_ += 3.f;
          SpawnBeamHitEffect(mgr);
          const CWeaponMode mode(kWT_PoisonWater1, false, false, false);
          if (actor->GetDamageVulnerability()->GetVulnerability(mode).mEffect ==
              CWeaponTypeVulnerability::kE_Immune) {
            mEffectC.x48_ = 3;
          } else {
            mEffectC.x48_ = 2;
          }
          return;
        }
      }
    }
  }
}

void CGrenchler::GrappleLoop(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrappleLoop, msg);
  switch (msg) {
  case kStateMsg_Activate: {
    mEffectC.x74_ = 0.f;
    mEffectC.x44_ = 0.f;
    mEffectC.x48_ = 0;
    mEffectC.x4c_ = CVector3f::Zero();
    mEffectC.xc_ = 1.5f;
    mEffectB.x0_0_ = mEffectB.x0_1_ = mEffectB.x0_2_ = false;
    mEffectB.x24_ = 0.f;
    mEffectB.x20_ = 0.f;
    mEffectB.x18_ = 0.f;
    mEffectB.x14_ = 0.f;
    mEffectB.x8_ = 0.f;
    mEffectB.x4_ = 0;
    mEffectB.x20_ = mgr.Random()->Range(mBeamAttack.mMinPause, mBeamAttack.mMaxPause);
    BodyController()->SetLocomotionType(pas::kLT_Internal10);
    mX90c_0_ = false;
    xe60_ = 0.f;
    PickGrappleSide(mgr);
    break;
  }
  case kStateMsg_Update: {
    if (mEffectC.x24_.mGen.get() == nullptr) {
      TurnToPlayer(mgr, dt, 1.5707964f);
      mEffectB.x24_ += dt;
    } else {
      if (mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        mEffectC.xc_ += 50.f * dt;
      } else {
        mEffectC.xc_ += 35.f * dt;
      }
      UpdateBeamTrace(mgr);
      if (mEffectC.xc_ < 1.f) {
        mEffectB.x24_ += dt;
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    mEffectB.x1c_ = x8ac_;
    if (xe6c_ > 0) {
      --xe6c_;
    }
    break;
  }
  BodyController()->CommandMgr().ClearLocomotionCmds();
}

void CGrenchler::PickGrappleSide(CStateManager& mgr) {
  switch (mEffectB.x28_) {
  case -1:
    if (mgr.Random()->Range(0.f, 1.f) < 0.5f) {
      mEffectB.x28_ = 0;
    } else {
      mEffectB.x28_ = 1;
    }
    break;
  case 0:
    mEffectB.x28_ = 1;
    break;
  case 1:
    mEffectB.x28_ = 0;
    break;
  }
}

bool CGrenchler::BreakGrappleLoop(CStateManager&, const CTriggerData&) const { return false; }

bool CGrenchler::IsBeamBlockedByHint(CStateManager& mgr) {
  CVector3f start = mBeamAttack.x7c_.GetTranslation();
  const CVector3f& end = GetPlayerTargetPosition(mgr);
  const CLineSeg line(start, end);
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetActive() && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() == CScriptAIHint::kHT_Unknown13) {
      CVector3f direction(end - start);
      if (direction.CanBeNormalized()) {
        const float length = direction.Magnitude();
        direction.Normalize();
        const float radiusSq = hint->GetRadius() * hint->GetRadius();
        for (float dist = 0.f; dist < length; dist += 2.f) {
          const CVector3f point(start.GetX() + direction.GetX() * dist,
                                start.GetY() + direction.GetY() * dist,
                                hint->GetTranslation().GetZ());
          if ((point - hint->GetTranslation()).MagSquared() < radiusSq) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool CGrenchler::IsNearAvoidHint(CStateManager& mgr) {
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CScriptAIHint* hint = TCastToPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetActive() && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetHintType() == CScriptAIHint::kHT_Unknown12) {
      CVector3f offset = GetTranslation() - hint->GetTranslation();
      offset.SetZ(0.f);
      if (offset.Magnitude() < hint->GetRadius()) {
        return true;
      }
    }
  }
  return false;
}

bool CGrenchler::CanGrapple(CStateManager&, const CTriggerData&) const { return false; }

void CGrenchler::PlayTailHitSound() {
  CSfxManager::SfxStart(mTailHitSound, 127, 64, GetCurrentAreaId().Value(), true, false,
                        CSfxManager::kMedPriority);
}

void CGrenchler::PlayTailDestroyedSound() {
  CSfxManager::SfxStart(mTailDestroyedSound, 127, 64, GetCurrentAreaId().Value(), true, false,
                        CSfxManager::kMedPriority);
}

void CGrenchler::ShakeOff(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_ShakeOff, msg);
  xa00_ = -1000.f;
  xeac_ = 0;
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_One));
}

bool CGrenchler::TailIntact(CStateManager&, const CTriggerData&) const { return xc6c_ != 1; }

bool CGrenchler::EmergedFromWater(CStateManager& mgr, const CTriggerData&) const {
  if (x9fc_ == true) {
    return false;
  }
  if (xa00_ <= 0.f || xa00_ > x8ac_) {
    return false;
  }
  if (xa04_ == kInvalidUniqueId) {
    return false;
  }
  return GetTranslation().GetZ() > GetWaterSurfaceHeight(mgr) - 0.5f;
}

void CGrenchler::FollowAttackPattern(CStateManager& mgr, EStateMsg msg, float dt) {
  bool submerged = IsDeeplySubmerged(mgr);
  if (msg == kStateMsg_Activate || x9fc_ != submerged) {
    SetSubmerged(submerged);
  }
  mWaypointNavigation.Patrol(mgr, msg, dt, *this);
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId destination = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    mWaypointNavigation.SetDestination(destination);
    const CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(destination));
    if (waypoint != nullptr) {
      const CVector3f delta = waypoint->GetTranslation() - GetTranslation();
      if (CVector3f::Dot(delta, GetTransform().GetForward()) <= 0.f) {
        mWaypointNavigation.SetInPosition(true);
      }
    }
    ResetSteering();
    break;
  }
  case kStateMsg_Update: {
    const CScriptAIWaypoint* waypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (BodyController()->GetCurrentStateId() == pas::kAS_Jump) {
      bool hasNext = true;
      if (waypoint != nullptr) {
        if (waypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next) != kInvalidUniqueId) {
          hasNext = false;
        }
      }
      if (hasNext) {
        BodyController()->CommandMgr().SetTargetVector(GetTargetPosition(mgr) - GetTranslation());
      }
    }
    mReflectInfo.x10_ = mWaypointNavigation.GetDestinationPosition();
    break;
  }
  case kStateMsg_Deactivate:
    UpdateChargeSteering();
    break;
  }
}

bool CGrenchler::AttackPatternOver(CStateManager&, const CTriggerData&) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CGrenchler::HasAttackPattern(CStateManager& mgr, const CTriggerData&) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CGrenchler::PauseOver(CStateManager&, const CTriggerData&) const { return x8ac_ > xce0_; }

void CGrenchler::PauseBetweenBeams(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    xce0_ = x8ac_ + mBeamAttack.mMinPause;
    xce0_ += (mBeamAttack.mMaxPause - mBeamAttack.mMinPause) * mgr.Random()->Float();
  } else if (msg == kStateMsg_Deactivate) {
    xce0_ = 0.f;
    mBeamAttack.x4c_ = -1000.f;
  }
  TurnToFaceTarget(mgr);
}

void CGrenchler::PauseBetweenBites(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    float minPause = mBiteAttack.mMinPause;
    float maxPause = mBiteAttack.mMaxPause;
    if (xc6c_ == 1) {
      minPause *= 0.3f;
      maxPause *= 0.3f;
    }
    xce0_ = x8ac_ + minPause;
    xce0_ += (maxPause - minPause) * mgr.Random()->Float();
  } else if (msg == kStateMsg_Deactivate) {
    xce0_ = 0.f;
    mBiteAttack.x64_ = -1000.f;
  }
  TurnToFaceTarget(mgr);
}

void CGrenchler::TurnToFaceTarget(CStateManager& mgr) {
  CVector3f faceVec = CVector3f::Zero();
  if (!IsFacingTarget(mgr, 0.61086524f)) {
    faceVec = mgr.GetPlayer(0)->GetTranslation() - GetTranslation();
  }
  BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(CVector3f::Zero(), faceVec, 1.f));
}

bool CGrenchler::JustBurstAttacked(CStateManager&, const CTriggerData&) const {
  if (GetLastAttackState() == 3) {
    if (0.2f + mBurstAttack.x3c_ > x8ac_) {
      return true;
    }
  }
  return false;
}

bool CGrenchler::JustBeamAttacked(CStateManager&, const CTriggerData&) const {
  if (GetLastAttackState() == 1) {
    float interval;
    if (xc6c_ == 1) {
      interval = 0.1f;
    } else {
      interval = 0.2f;
    }
    if (mBeamAttack.x4c_ + interval > x8ac_) {
      return true;
    }
  }
  return false;
}

bool CGrenchler::JustBiteAttacked(CStateManager&, const CTriggerData&) const {
  if (GetLastAttackState() == 2) {
    if (0.2f + mBiteAttack.x64_ > x8ac_) {
      return true;
    }
  }
  return false;
}

bool CGrenchler::JustHit(CStateManager& mgr, const CTriggerData&) const {
  if (!mIsGrappleGuardian) {
    if (0.2f + x900_ > x8ac_) {
      if (!IsFacingTarget(mgr, 1.5882496f)) {
        return true;
      }
    }
    return false;
  }
  if (!IsFacingTarget(mgr, 1.675516f)) {
    return true;
  }
  return 0.2f + x900_ > x8ac_;
}

bool CGrenchler::TookDamage(CStateManager&, const CTriggerData&) const {
  return GetHealthInfo()->GetHP() < mChargeAttack.x20_;
}

void CGrenchler::ChargeFailed(CStateManager& mgr, EStateMsg msg, float dt) {
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Zero));
  if (msg == kStateMsg_Deactivate) {
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AbortScripted));
  }
}

bool CGrenchler::IsNearPath(const CVector3f& pos, float padding) const {
  return mPathFindSearch.NearlyOnPath(pos, padding) == CPathFindSearch::kR_Success;
}

bool CGrenchler::FindJumpTarget(CStateManager& mgr, bool ignoreRange) {
  mReflectInfo.Reset();
  mReflectInfo.x10_ = GetTranslation();
  mReflectInfo.x1c_1_ = x9fc_;
  const CVector3f target = GetTargetPosition(mgr);
  const bool playerSubmerged = IsDeeplySubmerged(mgr, *mgr.GetPlayer(0));
  const bool selfSubmerged = IsDeeplySubmerged(mgr, *this);
  float maxCost;
  bool frustrated;
  if (ignoreRange == true) {
    maxCost = 1000.f;
    frustrated = true;
  } else {
    maxCost = (target - GetTranslation()).Magnitude();
    if (selfSubmerged != playerSubmerged) {
      maxCost *= 3.f;
    }
    frustrated = Frustrated(mgr, CTriggerData(0.f));
  }
  float bestCost = 10000.f * maxCost;
  const CScriptAiJumpPoint* bestJumpPoint = nullptr;
  const CScriptWaypoint* bestWaypoint = nullptr;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAiJumpPoint* jumpPoint = TCastToConstPtr< CScriptAiJumpPoint >(list[i]);
    if (jumpPoint != nullptr && jumpPoint->GetActive() &&
        jumpPoint->GetInUse(kInvalidUniqueId) != true &&
        jumpPoint->GetCurrentAreaId() == GetCurrentAreaId()) {
      const TUniqueId waypointId = jumpPoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next);
      if (waypointId != kInvalidUniqueId) {
        const CScriptWaypoint* waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId));
        if (waypoint != nullptr && playerSubmerged == IsDeeplySubmerged(mgr, *waypoint)) {
          const float cost = GetJumpCost(0.3f, jumpPoint->GetTranslation(),
                                         waypoint->GetTranslation(), target, frustrated);
          if (!(cost < 0.f || cost > maxCost) && cost < bestCost) {
            bestCost = cost;
            bestJumpPoint = jumpPoint;
            bestWaypoint = waypoint;
          }
        }
      }
    }
  }
  if (bestJumpPoint != nullptr) {
    mReflectInfo.x4_ = bestJumpPoint->GetTranslation();
    mReflectInfo.x0_ = bestJumpPoint->GetJumpApex();
    mReflectInfo.x10_ = bestWaypoint->GetTranslation();
    mReflectInfo.x1c_2_ = true;
    return true;
  }
  return false;
}

void CGrenchler::SReflectInfo::Reset() {
  x0_ = 2.f;
  x1c_0_ = x1c_1_ = x1c_4_ = x1c_2_ = x1c_3_ = x1c_5_ = false;
  x10_ = CVector3f::Zero();
  x4_ = x10_;
}

void CGrenchler::PickJumpTarget(CStateManager& mgr, float dt) { FindJumpTarget(mgr, false); }

bool CGrenchler::HasValidJumpTarget(CStateManager&, const CTriggerData&) const {
  return mReflectInfo.x1c_2_;
}

float CGrenchler::GetJumpCost(float weight, const CVector3f& jumpPos, const CVector3f& waypointPos,
                              const CVector3f& targetPos, bool skipEndCheck) const {
  const CVector3f toJump(jumpPos - GetTranslation());
  const float jumpDistance = toJump.Magnitude();
  if (jumpDistance > 5.f) {
    return -1.f;
  }
  const CVector3f toWaypoint = waypointPos - GetTranslation();
  if (toWaypoint.Magnitude() < 5.f) {
    return -1.f;
  }
  const CVector3f waypointToTarget = targetPos - waypointPos;
  const float targetDistance = waypointToTarget.Magnitude();
  if (!skipEndCheck) {
    const CVector3f waypointToJump = jumpPos - waypointPos;
    if (waypointToJump.Magnitude() < 0.9f * targetDistance) {
      return -1.f;
    }
  }
  const CVector3f jumpToWaypoint = waypointPos - jumpPos;
  return jumpToWaypoint.Magnitude() * weight + (jumpDistance + targetDistance);
}

void CGrenchler::SetLastActionAsBeam(CStateManager& mgr, float dt) {
  if (GetLastAttackState() != 1) {
    PushAttackHistory(mCollisionActors, kGA_BeamAttack);
  }
}

void CGrenchler::PushAttackHistory(rstl::reserved_vector< EAction, 3 >& history, EAction attack) {
  if (history.size() == 3) {
    history.erase(history.begin());
  }
  history.push_back(attack);
}

bool CGrenchler::CancelManeuvering(CStateManager& mgr, const CTriggerData& data) const {
  if (x8ac_ > 6.f + xcdc_) {
    return true;
  }
  return IsDeeplySubmerged(mgr, *mgr.GetPlayer(0)) != IsDeeplySubmerged(mgr, *this);
}

bool CGrenchler::ManeuverDone(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsGrappleGuardian == true) {
    return true;
  }
  if (Stuck(mgr, CTriggerData(0)) == true) {
    return true;
  }
  return HasClearShotAtPlayer(mgr, 1.7453293f);
}

CVector3f CGrenchler::GetManeuverTarget(CStateManager& mgr) const {
  const float heightOffset = mgr.Random()->Range(1.f, 2.f);
  float bestScore = 0.f;
  const CScriptAIHint* bestHint = nullptr;
  const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetActive() == true &&
        hint->GetHintType() == CScriptAIHint::kHT_Maneuver &&
        !(CMath::AbsF(hint->GetTranslation().GetZ() - GetTranslation().GetZ()) > 4.1f)) {
      float score = GetManeuverRangeWeight(10.f, hint->GetTranslation(), GetTranslation());
      score *= GetManeuverRangeWeight(12.f, hint->GetTranslation(), playerPos);
      if (score > bestScore) {
        const CVector3f hintPoint = hint->GetTranslation() + CVector3f(0.f, 0.f, heightOffset);
        if (!IsPointVisibleToPlayer(mgr, hintPoint)) {
          score *= 0.1f;
        }
        if (score > bestScore) {
          bestScore = score;
          bestHint = hint;
        }
      }
    }
  }
  if (bestHint == nullptr) {
    return GetTargetPosition(mgr);
  }
  return bestHint->GetTranslation();
}

float CGrenchler::GetManeuverRangeWeight(float range, const CVector3f& a,
                                         const CVector3f& b) const {
  const float distance = CVector3f(a - b).Magnitude();
  if (distance < range) {
    return distance / range;
  }
  if (distance > 2.f * range) {
    return 0.01f;
  }
  return 1.f - (distance - range) / range;
}

bool CGrenchler::ClearLineOfFire(CStateManager& mgr, const CTriggerData& data) const {
  if (IsFacingTarget(mgr, 0.61086524f) == false) {
    return false;
  }
  return HasClearShotAtPlayer(mgr, 0.87266463f);
}

bool CGrenchler::IsPointVisibleToPlayer(CStateManager& mgr, const CVector3f& point) const {
  const CVector3f target = GetTargetPosition(mgr) + CVector3f(0.f, 0.f, 0.5f);
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_Player, kMT_CollisionActor));
  const CVector3f end = target + CVector3f(0.f, 0.f, 0.5f);
  return mgr.RayCollideWorld(point, end, filter, nullptr);
}

bool CGrenchler::HasClearShotAtPlayer(CStateManager& mgr, float maxAngle) const {
  const CVector3f origin = mBeamAttack.x7c_.GetTranslation();
  const CVector3f target = GetTargetPosition(mgr) + CVector3f(0.f, 0.f, 0.5f);
  CVector3f direction = target - origin;
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
    const CVector2f directionXY = CVector2f(direction.GetX(), direction.GetY());
    const CVector2f forwardXY =
        CVector2f(GetTransform().GetForward().GetX(), GetTransform().GetForward().GetY());
    if (CVector2f::GetAngleDiff(forwardXY, directionXY) > maxAngle) {
      return false;
    }
  }
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_Player, kMT_CollisionActor));
  return mgr.RayCollideWorld(mBeamAttack.x7c_.GetTranslation(), target + CVector3f(0.f, 0.f, 0.5f),
                             filter, nullptr);
}

bool CGrenchler::CanBurstAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (!x9fc_) {
    return false;
  }
  if (!IsDeeplySubmerged(mgr, *mgr.GetPlayer(0))) {
    return false;
  }
  if (x8ac_ < mBurstAttack.x34_) {
    return false;
  }
  const CTeamAiRole::ETeamAiRole role = GetTeamRole(mgr);
  if (role == CTeamAiRole::kTAR_Projectile) {
    return true;
  }
  return role == CTeamAiRole::kTAR_Initial;
}

bool CGrenchler::CanBeamAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsGrappleGuardian == true) {
    return false;
  }
  if (x9fc_ == true) {
    return false;
  }
  if (IsDeeplySubmerged(mgr, *mgr.GetPlayer(0)) == true) {
    return false;
  }
  if (xa04_ != kInvalidUniqueId) {
    const float z = GetTranslation().GetZ();
    if (z + 1.5f < GetWaterSurfaceHeight(mgr)) {
      return false;
    }
  }
  const CTeamAiRole::ETeamAiRole role = GetTeamRole(mgr);
  if (role == CTeamAiRole::kTAR_Projectile) {
    return true;
  }
  if (xa88_ < 2.f) {
    if (role != CTeamAiRole::kTAR_Initial) {
      return false;
    }
    const int maxBeamAttacks = (xc6c_ == 1 ? 1 : 0) + 3;
    if (CountAttacks(mCollisionActors, kGA_BeamAttack) >= maxBeamAttacks) {
      return false;
    }
  }
  return true;
}

bool CGrenchler::CanBite(CStateManager& mgr, const CTriggerData& data) const {
  return HasMeleeRole(mgr);
}

bool CGrenchler::CanCharge(CStateManager& mgr, const CTriggerData& data) const {
  if (mgr.GetPlayer(0)->GetCurrentAreaId() != GetCurrentAreaId()) {
    return false;
  }
  if (xa04_ != kInvalidUniqueId) {
    const float z = GetTranslation().GetZ();
    if (z < GetWaterSurfaceHeight(mgr)) {
      return false;
    }
  }
  float pause = mChargeAttack.x2c_;
  if (mChargeAttack.x30_ >= 0.f && !mX90c_5_) {
    pause = mChargeAttack.x30_;
  }
  return mChargeAttack.x24_ + pause > x8ac_ ? false : HasMeleeRole(mgr);
}

CTeamAiRole::ETeamAiRole CGrenchler::GetTeamRole(CStateManager& mgr) const {
  const CScriptTeamAiMgr* teamMgr = GetTeamAiMgr(static_cast< const CStateManager& >(mgr));
  if (teamMgr == nullptr || teamMgr->GetRoleCount() < 2) {
    return CTeamAiRole::kTAR_Initial;
  }
  return teamMgr->GetTeamRole(GetUniqueId());
}

void CGrenchler::QuitTeam(CStateManager& mgr) {
  if (xccc_ != kInvalidUniqueId) {
    CScriptTeamAiMgr* teamMgr = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(xccc_));
    if (teamMgr != nullptr && teamMgr->IsPartOfTeam(GetUniqueId()) == true) {
      teamMgr->QuitTeam(GetUniqueId());
    }
  }
}

void CGrenchler::JoinTeam(CStateManager& mgr) {
  if (xccc_ != kInvalidUniqueId) {
    CScriptTeamAiMgr* teamMgr = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(xccc_));
    if (teamMgr != nullptr) {
      teamMgr->JoinTeam(*this, CTeamAiRole::kTAR_Melee, CTeamAiRole::kTAR_Projectile,
                        CTeamAiRole::kTAR_Unknown);
      teamMgr->SetPlayerForwardProjectionDistance(5.f);
    }
  }
}

CScriptTeamAiMgr* CGrenchler::GetTeamAiMgr(CStateManager& mgr) const {
  return TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(xccc_));
}

const CScriptTeamAiMgr* CGrenchler::GetTeamAiMgr(const CStateManager& mgr) const {
  return TCastToConstPtr< CScriptTeamAiMgr >(mgr.GetObjectById(xccc_));
}

bool CGrenchler::PlayerSubmerged(CStateManager& mgr, const CTriggerData& data) const {
  const CPlayer* player = mgr.GetPlayer(0);
  if (player->IsOnGround() == true) {
    xa06_ = IsDeeplySubmerged(mgr, *player);
  }
  return xa06_;
}

bool CGrenchler::ReturnToPatrol(CStateManager& mgr, const CTriggerData& data) const {
  return mX90c_4_;
}

float CGrenchler::GetFadeOnDeathTime() const {
  if (x9fc_ == false) {
    return CPatterned::GetFadeOnDeathTime();
  }
  return 8.f;
}
void CGrenchler::TurnToPlayer(CStateManager& mgr, float dt, float turnSpeed) {
  RotateToPoint(mgr.GetPlayer(0)->GetTranslation(), dt, turnSpeed);
}

void CGrenchler::UpdateChargeSteering() {
  CBodyController* controller = BodyController();
  const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
  const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
  float ratio = walkSpeed / runSpeed;
  if (mIsGrappleGuardian == true && xc6c_ != 1) {
    ratio *= 0.05f;
  }
  BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
  BodyController()->CommandMgr().SetSteeringSpeedRange(ratio, ratio);
}

void CGrenchler::ResetSteering() {
  BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
  BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
}

void CGrenchler::ApplyBiteDamage(CStateManager& mgr) {
  ++xeac_;
  if (InBiteRange(mgr, CTriggerData(0.f)) == true) {
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(),
                    mBiteAttack.mDamage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

void CGrenchler::ApplyBurstDamage(CStateManager& mgr) {
  const CVector3f offset = mgr.GetPlayer(0)->GetTranslation() - mBeamAttack.x7c_.GetTranslation();
  if (offset.Magnitude() < mBurstAttack.mDamageRadius &&
      PlayerSubmerged(mgr, CTriggerData(0)) == true) {
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(),
                    mBurstAttack.mDamage,
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
  }
}

void CGrenchler::IssueDeathBodyCommand(CStateManager& mgr, const CVector3f& direction) {
  if (x9fc_ == true) {
    BodyController()->SetLocomotionType(pas::kLT_Internal9);
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
  } else if (BodyController()->ShouldPlayDeathAnims()) {
    if (BodyController()->HasBodyState(pas::kAS_Hurled) &&
        BodyController()->GetBodyType() == kBT_Flyer) {
      BodyController()->CommandMgr().DeliverCmd(CBCHurledCmd(-direction, CVector3f::Zero()));
    } else if (BodyController()->HasBodyState(pas::kAS_Fall)) {
      const EScriptObjectState state = IsIngPossessed() ? kSS_DarkXDamage : kSS_XDamage;
      if (!mPendingMassiveDeath || CheckConnectedObject(mgr, state, kSM_None) == kInvalidUniqueId) {
        pas::ESeverity severity = pas::kS_One;
        const float surfaceHeight = GetWaterSurfaceHeight(mgr);
        if (GetTranslation().GetZ() + 0.2f < surfaceHeight) {
          severity = pas::kS_Six;
        }
        BodyController()->CommandMgr().DeliverCmd(CBCKnockDownCmd(-direction, severity));
      }
    }
  }
}

void CGrenchler::DestroyGrappleBeam(CStateManager& mgr) {
  mEffectC.x24_.mGen = rstl::auto_ptr< CElementGen >();
  mEffectC.x30_.mGen = rstl::auto_ptr< CElementGen >();
  RemoveExplosion(mgr);
  RemoveVisorEffect(mgr);
  if (mEffectC.x90_) {
    CSfxManager::RemoveEmitter(mEffectC.x90_);
    mEffectC.x90_.Clear();
  }
}

void CGrenchler::CreateGrappleBeam(CStateManager& mgr) {
  DestroyGrappleBeam(mgr);
  mEffectC.x0_ = GetPlayerTargetPosition(mgr);
  mEffectC.x24_.mGen = rstl::auto_ptr< CElementGen >(
      rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', mEffectC.x10_)),
                         CElementGen::kMOT_Normal, CElementGen::kOSF_One));
  mEffectC.x24_.mGen->SetParticleEmission(true);
  mEffectC.x30_.mGen = rstl::auto_ptr< CElementGen >(
      rs_new CElementGen(gpSimplePool->GetObj(SObjectTag('PART', mEffectC.x2c_)),
                         CElementGen::kMOT_Normal, CElementGen::kOSF_One));
  mEffectC.x30_.mGen->SetParticleEmission(false);
  mEffectC.x90_ = PlayCustomSound(mBeamAttack.x7c_.GetTranslation(), GetTransform().GetForward(),
                                  mEffectC.x78_, true);
}

bool CGrenchler::ChargeFinished(CStateManager& mgr, const CTriggerData& data) const {
  CVector3f totalOffset = mChargeAttack.xc_ - mChargeAttack.x0_;
  totalOffset.SetZ(0.f);
  CVector3f traveledOffset = GetTranslation() - mChargeAttack.x0_;
  traveledOffset.SetZ(0.f);
  return traveledOffset.Magnitude() > 0.9f * totalOffset.Magnitude();
}

void CGrenchler::Charge(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_Charge, msg);
  switch (msg) {
  case kStateMsg_Activate: {
    mChargeAttack.x28_ = mSpeed;
    mSpeed = 1.f;
    mChargeAttack.xc_ = GetTargetPosition(mgr);
    SetSubmerged(false);
    ResetSteering();
    const CVector3f target = GetTargetPosition(mgr);
    CVector3f direction = target - GetTranslation();
    direction.SetZ(0.f);
    direction.Normalize();
    const float health = GetHealthInfo()->GetHP();
    mChargeAttack.xc_ = CVector3f::Zero();
    mChargeAttack.x0_ = mChargeAttack.xc_;
    mChargeAttack.x20_ = health;
    mChargeAttack.x0_ = GetTranslation();
    mChargeAttack.xc_ = target + 1.f * direction;
    MoveToTarget(mgr, dt, mChargeAttack.xc_);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move;
    if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      move = BodyController()->CommandMgr().GetMoveVector();
    } else {
      move = mSteeringBehaviors.Arrival(*this, mChargeAttack.xc_, 5.f);
    }
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    mSpeed = mChargeAttack.x28_;
    UpdateChargeSteering();
    mChargeAttack.x24_ = x8ac_;
    break;
  }
}

void CGrenchler::Maneuver(CStateManager& mgr, EStateMsg msg, float dt) {
  if (mIsGrappleGuardian == true) {
    return;
  }
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f zero = CVector3f::Zero();
    xcd0_ = zero;
    xcdc_ = 0.f;
    xa94_ = BodyController()->GetTurnSpeed();
    if (!HasClearShotAtPlayer(mgr, 1.7453293f)) {
      xcdc_ = x8ac_;
      xcd0_ = GetManeuverTarget(mgr);
      MoveToTarget(mgr, dt, xcd0_);
      CBodyController* controller = BodyController();
      const float runSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
      const float walkSpeed = controller->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk);
      const float ratio = walkSpeed / runSpeed;
      const float speed = 0.5f * (1.f + ratio);
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyController()->CommandMgr().SetSteeringSpeedRange(speed, speed);
    }
    BodyController()->SetTurnSpeed(2.f * xa94_);
    break;
  }
  case kStateMsg_Update: {
    CVector3f move = CVector3f::Zero();
    if (!PathShagged(mgr, CTriggerData(0.f)) && !mPathFindSearch.IsOver()) {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      move = BodyController()->CommandMgr().GetMoveVector();
    } else {
      xcd0_ = GetManeuverTarget(mgr);
      MoveToTarget(mgr, dt, xcd0_);
    }
    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    BodyController()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    BodyController()->SetTurnSpeed(xa94_);
    break;
  }
}

void CGrenchler::StopElectricBeam(CStateManager& mgr) {
  if (mEffectA.x18_ != kInvalidUniqueId) {
    CEntity* beam = mgr.ObjectById(mEffectA.x18_);
    if (beam != nullptr) {
      beam->SetActive(false);
      mgr.DeleteObjectRequest(mEffectA.x18_);
    }
    mEffectA.x18_ = kInvalidUniqueId;
    CSfxManager::RemoveEmitter(mBeamAttack.x78_);
    mBeamAttack.x78_.Clear();
  }
}

void CGrenchler::StartElectricBeam(CStateManager& mgr) {
  if (mEffectA.x18_ != kInvalidUniqueId) {
    StopElectricBeam(mgr);
  }

  const CVector3f origin = mBeamAttack.x7c_.GetTranslation();
  const CVector3f target = mBeamAttack.x50_;
  const float radius = mBeamAttack.mBeamDamage.GetRadius();
  const float length = 2.f * (target - origin).Magnitude();
  const CElectricBeamInfo beamInfo(mEffectA.x10_, length, radius, 20.f, kInvalidAssetId, 0.5f, 0.f);
  CElectricBeamProjectile* beam = rs_new CElectricBeamProjectile(
      mEffectA.x1c_, kWT_AI, beamInfo, CTransform4f::Identity(), kMT_Character,
      mBeamAttack.mBeamDamage, mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), 0);
  mgr.AddObject(beam);
  mEffectA.x18_ = beam->GetUniqueId();

  const CTransform4f xf =
      CTransform4f::LookAt(mBeamAttack.x7c_.GetTranslation(), target, CVector3f::Up());
  beam->SetActive(true);
  beam->Fire(xf, mgr, false);
  mBeamAttack.x78_ = PlayCustomSound(mBeamAttack.x7c_.GetTranslation(),
                                     mBeamAttack.x7c_.GetForward(), mBeamAttack.mSound, false);
}

void CGrenchler::UpdateBeamTarget(CStateManager& mgr) {
  mBeamAttack.x50_ = GetPlayerBeamTargetPosition(mgr);
}

void CGrenchler::RemoveVisorEffect(CStateManager& mgr) {
  if (mEffectB.x30_ != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(mEffectB.x30_);
    mEffectB.x30_ = kInvalidUniqueId;
  }
}

void CGrenchler::CreateVisorEffect(CStateManager& mgr) {
  if (mEffectB.x2c_ != kInvalidAssetId) {
    RemoveVisorEffect(mgr);
    const float nearClip = CHUDBillboardEffect::GetNearClipDistance(mgr, 0);
    const CVector3f scale = CHUDBillboardEffect::GetScaleForPOV(mgr);
    if (mEffectB.x34_) {
      mEffectB.x30_ = mgr.AllocateUniqueId();
      CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
          mEffectB.x34_, rstl::optional_object_null(), mEffectB.x30_, true,
          rstl::string_l("Visor Grenchler Grapple Effect"), nearClip, scale, 0, CColor::White(),
          CVector3f::One(), CVector3f::Zero(), false);
      mgr.AddObject(effect);
    }
  }
}

void CGrenchler::CreateDamageVisorEffect(CStateManager& mgr) {
  const float nearClip = CHUDBillboardEffect::GetNearClipDistance(mgr, 0);
  const CVector3f scale = CHUDBillboardEffect::GetScaleForPOV(mgr);
  if (mDamageEffect.mEffect) {
    CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
        mDamageEffect.mEffect, rstl::optional_object_null(), mgr.AllocateUniqueId(), true,
        rstl::string_l("Visor Electric Contact Damage"), nearClip, scale, 0, CColor::White(),
        CVector3f::One(), CVector3f::Zero(), false);
    mgr.AddObject(effect);
  }
}

void CGrenchler::CreateGrappleHitVisorEffect(CStateManager& mgr) {
  if (IsPlayerLookingAtMe(mgr)) {
    const CVector3f offset(mgr.GetPlayer(0)->GetTranslation() - GetTranslation());
    if (offset.Magnitude() <= 23.f) {
      const float nearClip = CHUDBillboardEffect::GetNearClipDistance(mgr, 0);
      const CVector3f scale = CHUDBillboardEffect::GetScaleForPOV(mgr);
      if (mGrappleEffect.x4_) {
        CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
            mGrappleEffect.x4_, rstl::optional_object_null(), mgr.AllocateUniqueId(), true,
            rstl::string_l("Visor Grenchler Shakeoff Splotches"), nearClip, scale, 0,
            CColor::White(), CVector3f::One(), CVector3f::Zero(), false);
        mgr.AddObject(effect);
      }
    }
  }
}

void CGrenchler::ReleasePlayer(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  if (player->GetAttachedActorId() == GetUniqueId()) {
    player->DetachActorFromPlayer();
  }
  player->AddMaterial(kMT_Solid, mgr);
  player->EnableLeaveMorphBall(true);
  mXea0_0_ = false;

  CVector3f direction = GetTransform().GetForward();
  direction.SetZ(0.f);
  direction.Normalize();
  direction *= 40.f;
  direction.SetZ(2.f);
  player->Stop();
  player->ApplyImpulseWR(player->GetMass() * direction, CAxisAngle::Identity());
  player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
}

void CGrenchler::GrabPlayer(CStateManager& mgr) {
  CPlayer* player = mgr.GetPlayer(0);
  player->Stop();
  player->AttachActorToPlayer(GetUniqueId(), true);
  player->RemoveMaterial(kMT_Solid, mgr);
  player->EnableLeaveMorphBall(false);
  player->GetMorphBall()->DisableHalfPipeStatus();
  mXea0_0_ = true;
}

void CGrenchler::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  switch (type) {
  case kUE_Projectile:
    switch (xa78_) {
    case kGA_BurstAttack:
      ApplyBurstDamage(mgr);
      break;
    case kGA_BiteAttack:
      ApplyBiteDamage(mgr);
      break;
    }
    return;
  case kUE_AlignTargetPos:
    switch (xa78_) {
    case kGA_BeamAttack:
      UpdateBeamTarget(mgr);
      break;
    }
    break;
  case kUE_EffectOn:
    switch (xa78_) {
    case kGA_ShakeOff:
      CreateGrappleHitVisorEffect(mgr);
      break;
    case kGA_BeamAttack:
      StartElectricBeam(mgr);
      break;
    case kGA_GrappleLoop:
    case kGA_GrapplePull:
    case kGA_GrappleStruggle:
      CreateGrappleBeam(mgr);
      break;
    }
    return;
  case kUE_EffectOff:
    switch (xa78_) {
    case kGA_BeamAttack:
      StopElectricBeam(mgr);
      break;
    case kGA_GrappleLoop:
    case kGA_GrappleAbort:
    case kGA_GrappleBreak:
    case kGA_GrapplePull:
    case kGA_GrappleSlide:
    case kGA_GrappleStruggle:
      DestroyGrappleBeam(mgr);
      break;
    case kGA_MorphballBite:
      DestroyGrappleBeam(mgr);
      BeginMorphballCapture(mgr);
      break;
    }
    return;
  case kUE_EventStart:
    switch (xa78_) {
    case kGA_MorphballBite:
      GrabPlayer(mgr);
      break;
    }
    return;
  case kUE_EventStop:
    if (mXea0_0_ == 1) {
      EndMorphballCapture(mgr);
      ReleasePlayer(mgr);
    }
    return;
  case kUE_EndAction:
    switch (xa78_) {
    case kGA_GrappleAbort:
    case kGA_GrappleBreak:
    case kGA_GrappleStruggle:
      mEffectB.x0_1_ = true;
      break;
    case kGA_GrappleSlide:
      mXea8_0_ = true;
      break;
    }
    return;
  case kUE_BecomeShootThrough:
    for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
      const TUniqueId id = mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
      CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id));
      if (collisionActor != nullptr) {
        collisionActor->AddMaterial(kMT_ProjectilePassthrough, mgr);
      }
    }
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    return;
  case kUE_EggLay:
    switch (xa78_) {
    case kGA_GrappleStruggle:
      ++mEffectB.x4_;
      break;
    }
    return;
  case kUE_FadeIn:
    xc80_ = true;
    return;
  case kUE_FadeOut:
    xc80_ = false;
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    StartHitReaction(false);
    return;
  case kUE_ObjectDrop:
    ApplyScreenShake(mgr, GetTranslation(), kInvalidUniqueId);
    return;
  case kUE_ObjectPickUp:
    return;
  case kUE_Landing:
    mReflectInfo.x1c_3_ = true;
    break;
  case kUE_Unknown46:
    if (!IsDeeplySubmerged(mgr, *this)) {
      SendScriptMsgs(kSS_InternalState10, mgr, kInvalidUniqueId, kSM_None);
    }
    mReflectInfo.x1c_5_ = true;
    break;
  case kUE_Unknown37:
    if (mIsGrappleGuardian == 1) {
      mXea8_1_ = true;
    }
    break;
  case kUE_Unknown42:
    if (mIsGrappleGuardian == 1) {
      mEffectB.x0_2_ = true;
    }
    break;
  case kUE_Unknown45:
    if (mEffectC.x24_.mGen.get() == nullptr) {
      CreateGrappleBeam(mgr);
    }
    break;
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CGrenchler::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mEffectC.x24_.mGen.get() != nullptr) {
    mEffectC.x24_.mGen->Render();
  }
  if (mEffectC.x30_.mGen.get() != nullptr) {
    mEffectC.x30_.mGen->Render();
  }
  if (mEffectA.x3c_.mGen.get() != nullptr && mEffectA.x44_ > 0.f) {
    mEffectA.x3c_.mGen->Render();
  }
}

void CGrenchler::AddToRenderer(const CStateManager& mgr) const {
  CPatterned::AddToRenderer(mgr);
  if (GetAlive() == true) {
    if (mEffectA.x4_.mGen.get() != nullptr && mEffectA.x4_.mGen->GetParticleEmission() == true) {
      gpRender->AddParticleGen(*mEffectA.x4_.mGen);
    }
    if (mEffectD.x0_.mGen.get() != nullptr) {
      gpRender->AddParticleGen(*mEffectD.x0_.mGen);
    }
    if (mEffectC.x24_.mGen.get() != nullptr) {
      gpRender->AddParticleGen(*mEffectC.x24_.mGen);
    }
    if (mEffectC.x30_.mGen.get() != nullptr) {
      gpRender->AddParticleGen(*mEffectC.x30_.mGen);
    }
  }
}

void CGrenchler::BurstAttack(CStateManager& mgr, EStateMsg msg, float) {
  SetAttackState(kGA_BurstAttack, msg);
  if (msg == kStateMsg_Activate) {
    mBurstAttack.x2c_ = true;
    mBeamAttack.x2c_ = true;
    SetSubmerged(true);
  } else if (msg == kStateMsg_Deactivate) {
    mBurstAttack.x3c_ = x8ac_;
    mBurstAttack.x34_ = x8ac_ + mBurstAttack.mMinPause;
    if (mgr.IsRandomAvailable() == true) {
      mBurstAttack.x34_ = mBurstAttack.x34_ +
                          (mBurstAttack.mMaxPause - mBurstAttack.mMinPause) * mgr.Random()->Float();
    }
    xeac_ = 0;
  }
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Eleven));
}

bool CGrenchler::PlayerBehindMe(CStateManager& mgr, const CTriggerData&) const {
  if (mBeamAttack.x2c_) {
    return false;
  }
  return !IsFacingTarget(mgr, M_PIF / 2.f);
}

void CGrenchler::StopBeamAttack(CStateManager&, EStateMsg, float) {
  BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
}

void CGrenchler::BeamAttack(CStateManager& mgr, EStateMsg msg, float) {
  SetAttackState(kGA_BeamAttack, msg);
  if (msg == kStateMsg_Activate) {
    mBurstAttack.x2c_ = false;
    mBeamAttack.x2c_ = false;
    SetSubmerged(false);
    mBoneTracking.SetMaxBoneRotation(0.43633232f);
    if (xc6c_ == 1) {
      mSpeed = 1.2f;
    }
  } else if (msg == kStateMsg_Deactivate) {
    StopElectricBeam(mgr);
    mBoneTracking.SetMaxBoneRotation(0.87266463f);
    mBeamAttack.x4c_ = x8ac_;
    xeac_ = 0;
    mSpeed = 1.f;
    mBeamAttack.x78_ = CSfxHandle();
  } else if (BodyController()->IsFrozen() == true) {
    StopElectricBeam(mgr);
  }
  const CVector3f target = GetPlayerTargetPosition(mgr) + CVector3f(0.f, 0.f, 2.5f);
  DeliverCommand(msg, pas::kAS_ProjectileAttack,
                 CBCProjectileAttackCmd(pas::kS_One, target, false));
}

static EMaterialTypes sCollisionIncludeMaterial = kMT_Solid;           // Guessed name
static EMaterialTypes sCollisionExcludeMaterial0 = kMT_CollisionActor; // Guessed name
static EMaterialTypes sCollisionExcludeMaterial1 = kMT_AIPassthrough;  // Guessed name
static EMaterialTypes sCollisionExcludeMaterial2 = kMT_Player;         // Guessed name

void CGrenchler::CreateCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > descriptions;
  descriptions.reserve(10);
  AddJointCollisions(skJointInfo, 10, descriptions);
  mCollisionManager = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                    descriptions, GetActive());

  CVector3f direction = GetTransform().GetForward();
  direction.SetZ(0.f);
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
  }
  if (x99c_.GetPtr() == nullptr) {
    x99c_ =
        rs_new SConeVulnerability(direction, GetVulnerableAngle(), kWCR_Unknown75, kWCR_Unknown105);
  }

  for (uint i = 0; i < 10; ++i) {
    const SJointInfo& joint = skJointInfo[i];
    CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(
        mgr.ObjectById(mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()));
    CHealthInfo* health = collisionActor->HealthInfo();
    health->SetKnockbackResistance(GetHealthInfo()->GetKnockBackResistance());
    health->SetHP(10000.f);
    if (joint.mVulnerabilityGroup == 0) {
      collisionActor->SetDamageVulnerability(
          CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
    } else {
      collisionActor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
    if (joint.mVulnerabilityGroup != 1 || !mIsGrappleGuardian) {
      collisionActor->SetNonUniformVulnerability(x99c_);
    }
  }

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(CMaterialList(sCollisionIncludeMaterial),
                                                        CMaterialList(sCollisionExcludeMaterial0,
                                                                      sCollisionExcludeMaterial1,
                                                                      sCollisionExcludeMaterial2)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
}

void CGrenchler::AddJointCollisions(
    const SJointInfo* joints, int count,
    rstl::vector< CJointCollisionDescription >& descriptions) const {
  const CAnimData* animData = GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId segId = animData->GetLocatorSegId(rstl::string_l(joints[i].mName));
    if (segId != CSegId::Invalid()) {
      const float radius = mIsGrappleGuardian ? joints[i].mGuardianRadius : joints[i].mRadius;
      CJointCollisionDescription description = CJointCollisionDescription::SphereCollision(
          segId, CVector3f::Zero(), radius, rstl::string_l(joints[i].mName), 10000.f);
      descriptions.push_back(description);
    }
  }
}

bool CGrenchler::Alerted(CStateManager&, const CTriggerData&) const { return mX90c_1_; }
// v2

bool CGrenchler::Attacked(CStateManager&, const CTriggerData&) const {
  return mHitByPlayerProjectile;
}

bool CGrenchler::JumpLanded(CStateManager&, const CTriggerData&) const {
  return mReflectInfo.x1c_3_;
}

void CGrenchler::RestoreDefaultJointVulnerability(CStateManager& mgr) {
  ResetBodyVulnerabilities(mgr, *CPatterned::GetDamageVulnerability());
  mReflectInfo.x1c_0_ = false;
}

void CGrenchler::Jump(CStateManager& mgr, EStateMsg msg, float) {
  SetAttackState(kGA_Jump, msg);
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    SetupBodyVulnerabilities(mgr, CDamageVulnerability::ReflectVulnerabilty());
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_WorldUp);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(CBCJumpCmd(
          mReflectInfo.x10_, mReflectInfo.x1c_1_ == 1 ? pas::kJT_Normal : pas::kJT_Ambush,
          pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Over) {
      mReflectInfo.x1c_3_ = true;
    }
    if (!mReflectInfo.x1c_4_) {
      if (!mReflectInfo.x1c_1_) {
        if (IsDeeplySubmerged(mgr) == true) {
          SetVelocityWR(GetVelocityWR() * 0.33f);
          mReflectInfo.x1c_4_ = true;
        }
      } else if (!IsDeeplySubmerged(mgr)) {
        mReflectInfo.x1c_4_ = true;
      }
    }
    if (mReflectInfo.x1c_4_ == 1) {
      CScriptWater* water =
          TCastToPtr< CScriptWater >(const_cast< CEntity* >(mgr.GetObjectById(xa04_)));
      if (water != nullptr) {
        const CVector3f splashPosition(GetTranslation().GetX(), GetTranslation().GetY(),
                                       GetWaterSurfaceHeight(mgr));
        mgr.GetFluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, *water, splashPosition, 1.f,
                                                 true);
      }
    }
    break;
  case kStateMsg_Deactivate:
    SetSubmerged(IsDeeplySubmerged(mgr));
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mReflectInfo.Reset();
    mReflectInfo.x50_ = x8ac_;
    break;
  }
}

// Guessed name; offset from the model transform to the aim point.
static CVector3f skAimOffset(0.f, 0.f, 0.5f);

static const char* const skBodyBubblesEffect = "BodyBubbles"; // Guessed name
static const char* const skElectricEffects[] = {
    "Electric", "ElectricBody", "ElectricHead", "ElectricLLeg", "ElectricRLeg",
}; // Guessed name

void CGrenchler::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (xa78_ != 12 && GetBodyController()->IsFrozen() != 1 &&
      (mIsGrappleGuardian != 1 || (!(x8fc_ + 3.f > x8ac_) && xc80_ != 1))) {
    mKnockBackController.SetAdditiveFlinchWeight(mgr.Random()->Range(0.55f, 0.8f));
    CPatterned::KnockBack(mgr, info);
    x8fc_ = x8ac_;
  }
}

bool CGrenchler::FacingJumpEnd(CStateManager& mgr, const CTriggerData& data) const {
  return IsFacing(mReflectInfo.x10_, M_PIF / 12.f);
}

void CGrenchler::TurnToJumpEnd(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    if (x9fc_ == 1) {
      DisableGroundCollision(mgr);
    }
  } else if (msg == kStateMsg_Deactivate && x9fc_ == 1) {
    EnableGroundCollision(mgr, true);
  }

  CVector3f toTarget = mReflectInfo.x10_ - GetTranslation();
  toTarget.SetZ(0.f);
  if (toTarget.CanBeNormalized() == true) {
    BodyController()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
  }
}

void CGrenchler::NotifyFalling(CStateManager& mgr, const TUniqueId& id) {
  if (GetBodyController()->GetCurrentStateId() != pas::kAS_Hurled) {
    CPatterned::AcceptScriptMsg(mgr, CScriptMsg(id, GetUniqueId(), kSM_Falling));
    SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
    LaunchToJumpTarget();
  }
}

void CGrenchler::LaunchToJumpTarget() {
  if (mReflectInfo.x1c_0_ == 1) {
    return;
  }
  CVector3f velocity = CVector3f::Zero();
  const float dx = mReflectInfo.x10_.GetX() - GetTranslation().GetX();
  const float dy = mReflectInfo.x10_.GetY() - GetTranslation().GetY();
  const float gravity = GetGravityConstant();
  const float z = GetTranslation().GetZ();
  const float apex = mReflectInfo.x0_ + rstl::max_val(z, mReflectInfo.x10_.GetZ());
  const float climbSpeed = CMath::SqrtF(2.f * gravity * (apex - z));
  velocity.SetZ(climbSpeed);
  float flightTime = climbSpeed / gravity;
  flightTime += CMath::SqrtF(2.f * (apex - mReflectInfo.x10_.GetZ()) / gravity);
  const float invTime = 1.f / flightTime;
  velocity.SetX(invTime * dx);
  velocity.SetY(invTime * dy);
  SetVelocityWR(velocity);
  mReflectInfo.x1c_0_ = true;
}

void CGrenchler::SetGroundCollision(CStateManager& mgr, EStateMsg msg) {
  if (msg == kStateMsg_Activate) {
    DisableGroundCollision(mgr);
  } else if (msg == kStateMsg_Deactivate) {
    EnableGroundCollision(mgr, true);
  }
}

void CGrenchler::EnableGroundCollision(CStateManager& mgr, bool onGround) {
  mVerticalMovement = false;
  mOnGround = onGround;
  AddMaterial(kMT_GroundCollider, mgr);
}

void CGrenchler::DisableGroundCollision(CStateManager& mgr) {
  mVerticalMovement = true;
  mOnGround = false;
  RemoveMaterial(kMT_GroundCollider, mgr);
}

void CGrenchler::SetupBodyVulnerabilities(CStateManager& mgr,
                                          const CDamageVulnerability& vulnerability) {
  if (mCollisionManager.get() != nullptr) {
    for (uint i = 0; i < 10; ++i) {
      const SCollisionJoint& joint = skCollisionJoints[i];
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(
          mCollisionManager.get()->GetCollisionDescFromIndex(i).GetCollisionActorId()));
      if (actor != nullptr) {
        if (xc6c_ == 1 && (mIsGrappleGuardian == 1 || joint.xc_ == 0)) {
          if (joint.xc_ == 1 && mIsGrappleGuardian == 1) {
            actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          } else {
            actor->SetDamageVulnerability(mDamageVulnerability.MakeIgnoreRadius());
          }
          actor->SetResponseType(kWCR_Unknown45);
          if (joint.xc_ != 1 || !mIsGrappleGuardian) {
            actor->SetNonUniformVulnerability(x99c_);
          }
        } else {
          if (joint.xc_ == 0) {
            actor->SetDamageVulnerability(vulnerability.MakeIgnoreRadius());
          } else if (joint.xc_ == 1 && mIsGrappleGuardian == 1) {
            actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          } else {
            actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
          }
          actor->SetResponseType(joint.x10_);
          if (joint.xc_ != 1 || !mIsGrappleGuardian) {
            actor->SetNonUniformVulnerability(x99c_);
          }
        }
      }
    }
  }
}

void CGrenchler::ResetBodyVulnerabilities(CStateManager& mgr,
                                          const CDamageVulnerability& vulnerability) {
  if (mCollisionManager.get() != nullptr) {
    for (uint i = 0; i < 10; ++i) {
      CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(
          mCollisionManager.get()->GetCollisionDescFromIndex(i).GetCollisionActorId()));
      actor->SetDamageVulnerability(vulnerability.MakeIgnoreRadius());
      actor->ResetNonUniformVulnerability();
    }
  }
}

bool CGrenchler::TooMuchTurning(CStateManager& mgr, const CTriggerData& data) const {
  if (IsGuardianCharging() == true) {
    return true;
  }
  if (xcf4_ == 1) {
    return AnimOver(mgr, data);
  }
  return xce8_ > 0.f && 2.f + xce8_ < x8ac_;
}

bool CGrenchler::IsGuardianCharging() const { return mIsGrappleGuardian == 1 && xe6c_ == 4; }

void CGrenchler::Turn(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    xcf4_ = 0;
    xce8_ = x8ac_;
    xcec_ = GetBodyController()->GetTurnSpeed();
    if (x9fc_ == 1) {
      xcf4_ = 0;
    } else if (!mIsGrappleGuardian) {
      if (0.1f + x900_ > x8ac_ && !IsFacingTarget(mgr, 2.0943952f)) {
        xcf4_ = 1;
      }
    } else {
      xcf4_ = 0;
      if (2.f + xcf0_ < x8ac_ && !IsFacingTarget(mgr, 1.5882496f)) {
        xcf4_ = 1;
      }
      if (xcf4_ == 0) {
        BodyController()->SetTurnSpeed(2.5f * xcec_);
      }
    }
  } else if (msg == kStateMsg_Deactivate) {
    BodyController()->SetTurnSpeed(xcec_);
    if (xcf4_ == 1) {
      xcf0_ = x8ac_;
    }
  }

  if (xcf4_ == 1) {
    DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Six));
  } else if (!mIsGrappleGuardian && x9fc_ == 0 && 0.2f + x900_ > x8ac_ &&
             !IsFacingTarget(mgr, 2.0943952f)) {
    xce8_ = -1000.f;
    xcf0_ = -1000.f;
    xcf4_ = 0;
    xce8_ = x8ac_;
    xcf4_ = 1;
    DeliverCommand(kStateMsg_Activate, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Six));
  } else {
    CVector3f toTarget = GetTargetPosition(mgr) - GetTranslation();
    toTarget.SetZ(0.f);
    if (toTarget.CanBeNormalized() == true) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), toTarget.AsNormalized(), 1.f));
    }
  }
}

bool CGrenchler::AbortCharge(CStateManager& mgr, const CTriggerData& data) const {
  if (!mIsGrappleGuardian) {
    return false;
  }
  return !IsFacingTarget(mgr, 1.5882496f) ? true : ChargeFinished(mgr, data);
}

void CGrenchler::GrappleSlideBonk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SendScriptMsgs(kSS_InternalState02, mgr);
    break;
  case kStateMsg_Update:
    if (10.f + xea4_ < x8ac_) {
      mXea8_0_ = true;
    }
    break;
  case kStateMsg_Deactivate:
    mEffectB.x1c_ = x8ac_;
    DestroyGrappleBeam(mgr);
    break;
  }
  SetAttackState(kGA_GrappleSlide, msg);
  BodyController()->SetLocomotionType(pas::kLT_Relaxed);
  BodyController()->CommandMgr().ClearLocomotionCmds();
}

void CGrenchler::GrappleSlide(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_GrappleSlide, msg);
  BodyController()->SetLocomotionType(pas::kLT_Internal5);
  BodyController()->CommandMgr().ClearLocomotionCmds();
  switch (msg) {
  case kStateMsg_Activate:
    xea4_ = x8ac_;
    mXea8_0_ = false;
    mXea8_1_ = false;
    SendScriptMsgs(kSS_Locked, mgr);
    xe6c_ = 0;
    break;
  case kStateMsg_Update: {
    CVector3f direction = mEffectC.x4c_ - mBeamAttack.x7c_.GetTranslation();
    direction.SetZ(0.f);
    if (direction.CanBeNormalized() == true) {
      direction.Normalize();
      const float elapsed = CMath::Clamp(0.f, x8ac_ - xea4_, 2.f);
      const float step = dt * (10.f * (elapsed * 0.5f) + 10.f);
      direction *= step;
      mEffectC.xc_ = rstl::max_val(mEffectC.xc_ - step, 0.f);
      MoveInOneFrameOR(GetTransform().TransposeRotate(direction), dt);
    }
    if (10.f + xea4_ < x8ac_) {
      mXea8_0_ = true;
    }
    break;
  }
  case kStateMsg_Deactivate:
    SendScriptMsgs(kSS_Unlocked, mgr);
    xeac_ = 0;
    mEffectB.x1c_ = x8ac_;
    break;
  }
}

void CGrenchler::ResolveCollision(CStateManager& mgr) {
  CAABox motionVolume = GetMotionVolume(0.f);
  CAreaCollisionCache cache(motionVolume);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildColliderList(nearList, *this, motionVolume);
  CGameCollision::CollisionFailsafe(mgr, cache, *this, *GetCollisionPrimitive(), nearList, 0.f, 0,
                                    0.f);
}

bool CGrenchler::SlideOver(CStateManager& mgr, const CTriggerData& data) const { return mXea8_0_; }

bool CGrenchler::SlideStop(CStateManager& mgr, const CTriggerData& data) const {
  const float slideStart = xea4_;
  const float time = x8ac_;
  if (0.5f + slideStart > time) {
    return false;
  }
  if (8.f + slideStart < time) {
    return true;
  }
  if (!mXea8_1_) {
    return false;
  }
  CVector3f toTarget = mEffectC.x4c_ - mBeamAttack.x7c_.GetTranslation();
  toTarget.SetZ(0.f);
  const float threshold = 2.f * mPredictedLeashTime + 2.5f;
  return toTarget.Magnitude() < threshold;
}

void CGrenchler::Backstep(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_Backstep, msg);
  xeac_ = 0;
  DeliverCommand(msg, pas::kAS_Step, CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
}

bool CGrenchler::ShouldBackstep(CStateManager& mgr, const CTriggerData& data) const {
  if (IsGuardianCharging() == true) {
    return false;
  }
  if (mPredictedLeashTime > 1.f) {
    return true;
  }
  if (x9fc_ == 1) {
    return false;
  }
  if (xf20_ != kInvalidUniqueId) {
    CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(xf20_)));
    if (actor != nullptr) {
      if (IsFacing(actor->GetTranslation(), 2.0943952f) == true) {
        return true;
      }
      if (CMath::AbsF(actor->GetTranslation().GetZ() - GetTranslation().GetZ()) > 4.f) {
        return true;
      }
    }
  }
  return xeac_ >= 2 + (xc6c_ == 1 ? 1 : 0);
}

bool CGrenchler::ShouldSlide(CStateManager& mgr, const CTriggerData& data) const {
  return !mEffectB.x28_ && !IsNearAvoidHint(mgr);
}

bool CGrenchler::ShouldTurn(CStateManager& mgr, const CTriggerData& data) const {
  return !IsFacingTarget(mgr, M_PIF / 4.f);
}

bool CGrenchler::FacingPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return IsFacingTarget(mgr, 0.61086524f);
}

bool CGrenchler::IsPlayerLookingAtMe(CStateManager& mgr) const {
  const CPlayer* player = mgr.GetPlayer(0);
  CVector2f playerForward = player->GetTransform().GetForward().ToVec2f();
  CVector2f toMe = (GetTranslation() - player->GetTranslation()).ToVec2f();
  return CVector2f::GetAngleDiff(playerForward, toMe) < 0.87266463f;
}

bool CGrenchler::ForceGrapple(CStateManager& mgr, const CTriggerData& data) const {
  return xe6c_ > 0;
}

bool CGrenchler::IsFacingTarget(CStateManager& mgr, float maxAngle) const {
  return IsFacing(GetTargetPosition(mgr), maxAngle);
}

bool CGrenchler::IsFacing(const CVector3f& position, float maxAngle) const {
  CVector2f toTarget = (position - GetTranslation()).ToVec2f();
  CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CVector2f::GetAngleDiff(forward, toTarget) < maxAngle;
}

CVector3f CGrenchler::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f aim = xc3c_.GetTranslation() + skAimOffset;
  if (!mIsGrappleGuardian) {
    return aim;
  }
  if (!mX90c_5_) {
    return xe2c_ + CVector3f(0.f, 0.f, 1.6f);
  }
  return CVector3f::Lerp(xe2c_, aim, xe28_);
}

CVector3f CGrenchler::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CGrenchler::GetScanObjectIndicatorPosition(const CStateManager& mgr) const {
  if (xa78_ == 14) {
    return mVectors.x18_;
  }
  return CActor::GetScanObjectIndicatorPosition(mgr);
}

void CGrenchler::GrappleBite(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_BiteAttack, msg);
  switch (msg) {
  case kStateMsg_Activate:
    mBiteAttack.x0_ = false;
    mX90c_0_ = false;
    IncrementAttached(mgr);
    xe24_ = 1.2f;
    break;
  case kStateMsg_Update:
    if (xe24_ > 0.f) {
      xe24_ -= dt;
      if (xe24_ <= 0.f) {
        DecrementAttached(mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mBiteAttack.x64_ = x8ac_;
    mX90c_0_ = true;
    if (xe24_ > 0.f) {
      DecrementAttached(mgr);
      xe24_ = 0.f;
    }
    mEffectB.x1c_ = x8ac_;
    break;
  }
  SetSubmerged(false);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_Two));
}

void CGrenchler::BiteAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_BiteAttack, msg);
  if (msg == kStateMsg_Activate) {
    mBiteAttack.x0_ = IsDeeplySubmerged(mgr);
  } else if (msg == kStateMsg_Deactivate) {
    mBiteAttack.x64_ = x8ac_;
  } else if (mIsGrappleGuardian == 1) {
    TurnToPlayer(mgr, dt, 1.0471976f);
  }
  SetSubmerged(mBiteAttack.x0_);
  DeliverCommand(msg, pas::kAS_MeleeAttack, CBCMeleeAttackCmd(pas::kS_One));
}

void CGrenchler::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_Lurk, msg);
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    x9fc_ = IsDeeplySubmerged(mgr);
    SetSubmerged(x9fc_);
    mHitByPlayerProjectile = false;
    mKnockBackController.EnableAllAnimReactions(false);
    if (x9fc_ == 1) {
      DisableGroundCollision(mgr);
      mX90c_0_ = false;
    }
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    mKnockBackController.EnableAllAnimReactions(true);
    if (x9fc_ == 1) {
      EnableGroundCollision(mgr, true);
    }
    break;
  }
}

void CGrenchler::SetAttackState(EAction state, EStateMsg msg) {
  if (msg == kStateMsg_Deactivate) {
    PushAttackHistory(mCollisionActors, xa78_);
    xa78_ = kGA_None;
  } else {
    xa78_ = state;
  }
}

void CGrenchler::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  bool submerged = IsDeeplySubmerged(mgr);
  if (msg == kStateMsg_Activate || x9fc_ != submerged) {
    SetSubmerged(submerged);
  }
  mX90c_4_ = false;
  CPatterned::Patrol(mgr, msg, dt);
}

void CGrenchler::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    UnmarkPathRegion(mgr);
    QuitTeam(mgr);
    xc80_ = 0;
    DestroyGrappleBeam(mgr);
    if (xec8_) {
      CSfxManager::RemoveEmitter(xec8_);
      xec8_ = CSfxHandle();
    }
    RemoveMaterial(kMT_Target, kMT_Orbit, kMT_SeekerTarget, mgr);
    SetSubmerged(IsDeeplySubmerged(mgr));
    if (x9fc_ == 1) {
      RemoveMaterial(kMT_GroundCollider, mgr);
      mVerticalMovement = true;
      mOnGround = false;
      mOnStaticGround = false;
      xcc8_ = mgr.Random()->Next();
      mStateControlledMassiveDeath = false;
      ResetBodyVulnerabilities(mgr, CDamageVulnerability::ReflectVulnerabilty());
    }
    if (mEffectA.x4_.mGen.get() != nullptr) {
      mEffectA.x4_.mGen->SetParticleEmission(false);
    }
    if (mEffectD.x0_.mGen.get() != nullptr) {
      mEffectD.x0_.mGen->SetParticleEmission(false);
    }
    AnimationData()->SetEffectState(rstl::string_l(skBodyBubblesEffect), false, mgr);
    for (int i = 0; i < 5; ++i) {
      AnimationData()->SetEffectState(rstl::string_l(skElectricEffects[i]), false, mgr);
    }
    DestroyGrappleBeam(mgr);
    EndMorphballCapture(mgr);
  } else if (msg == kStateMsg_Update) {
    if (mIsGrappleGuardian == 1 &&
        mgr.GetPlayer(0)->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
        mCollisionManager.get() != nullptr) {
      mCollisionManager.get()->SetActive(mgr, false);
    }
  }

  if (x9fc_ == 0) {
    CPatterned::Dead(mgr, msg, dt);
    return;
  }

  float rotateRate = 10.f * dt;
  if ((xcc8_ & 1) != 0) {
    rotateRate *= -1.f;
  }
  SetTransform((CQuaternion::ZRotation(CRelAngle::FromDegrees(rotateRate)) *
                CQuaternion::FromMatrix(GetTransform()))
                   .BuildTransform4f(GetTranslation()));

  if (xcbc_ == 1) {
    const float bob = CMath::FastSinR(2.5f * xcc0_);
    SetTranslation(
        CVector3f(GetTranslation().GetX(), GetTranslation().GetY(), 0.25f * bob + xcc4_));
    xcc0_ += dt;
    if (xcc0_ > GetFadeOnDeathTime()) {
      MassiveDeath(mgr);
    }
  } else {
    const float waterHeight = GetWaterSurfaceHeight(mgr);
    if (GetTranslation().GetZ() >= waterHeight - 3.3f) {
      SetVelocityWR(CVector3f::Zero());
      xcbc_ = 1;
      xcc4_ = GetTranslation().GetZ();
      mAlphaDelta = -1.f / GetFadeOnDeathTime();
    } else {
      xcb8_ += dt;
      SetVelocityWR(0.85f * CVector3f::Up());
      if (xcb8_ >= 10.f) {
        mAlphaDelta = -1.f / GetFadeOnDeathTime();
        xcc0_ += dt;
        if (xcc0_ > GetFadeOnDeathTime()) {
          MassiveDeath(mgr);
        }
      }
    }
  }
}

// Strings shared by several states; they sit at the start of .rodata in this order, with other
// strings in between (BossGrappleGuardian, head, Skeleton_Root...).
static const char* const skEyeLocator = "eye";                    // Guessed name
static const char* const skHornLocator = "horn_LCTR";             // Guessed name
static const char* const skSkeletonRootLocator = "Skeleton_Root"; // Guessed name
static const char* const skJawLocator = "jaw";                    // Guessed name
static EMaterialTypes sFloorMaterial = kMT_Floor;                 // Guessed name
static const char* const skAttachLocator = "attach_LCTR_SDK";     // Guessed name

void CGrenchler::SetupStateMachineHelper(CStateManager& mgr) {
  InitializeStateMachine(mgr);
  mStateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  mStateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
  mStateMachine->SetCodeFunctions(skCodeFuncs, ARRAY_SIZE(skCodeFuncs));
}

const CGenericFSM2* CGrenchler::GetStateMachine() const {
  if (!mFsm->IsLoaded()) {
    return nullptr;
  }
  TToken< CGenericFSM2 > machine(*mFsm);
  return *machine;
}

bool CGrenchler::IsDeeplySubmerged(CStateManager& mgr) const {
  return IsDeeplySubmerged(mgr, *this);
}

bool CGrenchler::IsDeeplySubmerged(CStateManager& mgr, const CActor& actor) const {
  if (actor.InFluidId() == kInvalidUniqueId) {
    return false;
  }
  CScriptWater* water =
      TCastToPtr< CScriptWater >(const_cast< CEntity* >(mgr.GetObjectById(actor.InFluidId())));
  if (water == nullptr) {
    return false;
  }
  return water->GetTriggerBoundsWR().GetMaxPoint().GetZ() > actor.GetTranslation().GetZ() + 2.5f;
}

float CGrenchler::GetWaterSurfaceHeight(CStateManager& mgr) {
  if (InFluidId() != kInvalidUniqueId) {
    xa04_ = InFluidId();
  }
  if (xa04_ == kInvalidUniqueId) {
    return -10000.f;
  }
  CScriptWater* water =
      TCastToPtr< CScriptWater >(const_cast< CEntity* >(mgr.GetObjectById(xa04_)));
  if (water == nullptr) {
    return GetTranslation().GetZ();
  }
  return water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
}

void CGrenchler::PreRenderBoneTracking(CStateManager& mgr) {
  mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->GetScale(),
                          *BodyController());
}

CVector3f CGrenchler::GetBlendedLocatorPosition(float height, const char* firstLocator,
                                                const char* secondLocator) const {
  const CVector3f first = GetLctrTransform(rstl::string_l(firstLocator)).GetTranslation();
  const CVector3f second = GetLctrTransform(rstl::string_l(secondLocator)).GetTranslation();
  if (height > first.GetZ() && height > second.GetZ()) {
    return CVector3f(first.GetX(), first.GetY(), height);
  }
  if (height < first.GetZ() && height < second.GetZ()) {
    return CVector3f(second.GetX(), second.GetY(), height);
  }
  return CVector3f(0.5f * (first.GetX() + second.GetX()), 0.5f * (first.GetY() + second.GetY()),
                   height);
}

void CGrenchler::PreRender(CStateManager& mgr) {
  CPatterned::PreRender(mgr);
  PreRenderBoneTracking(mgr);
  xc3c_ = GetLctrTransform(rstl::string_l(skSkeletonRootLocator));
  mBeamAttack.x7c_ = GetLctrTransform(rstl::string_l(skHornLocator));
  mBiteAttack.x30_ = GetLctrTransform(rstl::string_l(skJawLocator));
  if (mIsGrappleGuardian == true) {
    xe70_ = GetLctrTransform(rstl::string_l(skAttachLocator));
    xe2c_ = GetLctrTransform(rstl::string_l(skEyeLocator)).GetTranslation();
    if (mEffectA.x3c_.mGen.get() != nullptr) {
      mEffectA.x3c_.mGen->SetGlobalTranslation(xe2c_);
      mEffectA.x3c_.mGen->Update(0.0);
    }
    mVectors.x0_ = GetLctrTransform(rstl::string_l("eye")).GetTranslation();
    mVectors.xc_ = GetLctrTransform(rstl::string_l("horn_LCTR")).GetTranslation();
    mVectors.x18_ = GetLctrTransform(rstl::string_l("head")).GetTranslation();
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->Update(0.f, mgr, CCollisionActorManager::kUO_WorldSpace);
    }
  } else if (IsWadingInWater(mgr) == true) {
    const CVector3f first = GetLctrTransform(rstl::string_l("attach_LCTR_SDK")).GetTranslation();
    const CVector3f second = GetLctrTransform(rstl::string_l("attatch_LCTR")).GetTranslation();
    mEffectD.x14_ = first + (first - second) + CVector3f(0.f, 0.f, -0.35f);
  }
}

void CGrenchler::ThinkBoneTracking(float dt, CStateManager& mgr) {
  if (!ShouldTrackPlayer()) {
    mBoneTracking.SetActive(false);
  } else {
    mBoneTracking.SetTarget(mgr.GetPlayer(0)->GetUniqueId());
    mBoneTracking.SetActive(true);
  }
  mBoneTracking.Think(dt);
}

bool CGrenchler::ShouldTrackPlayer() const { return mX90c_0_; }

CVector3f CGrenchler::GetPlayerTargetPosition(CStateManager& mgr) const {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f position = player->GetAimPosition(mgr, 0.f);
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CAABox bounds = player->GetTouchBounds().data();
    position = bounds.GetCenterPoint();
  } else {
    position += CVector3f(0.f, 0.f, -0.5f);
  }
  return position;
}

CVector3f CGrenchler::GetHornTargetPosition() const {
  if (mEffectC.x0_ == CVector3f::Zero()) {
    return mBeamAttack.x7c_.GetTranslation();
  }
  CVector3f direction = mEffectC.x0_ - mBeamAttack.x7c_.GetTranslation();
  if (direction.CanBeNormalized() == true) {
    direction.Normalize();
    direction *= mEffectC.xc_;
    return mBeamAttack.x7c_.GetTranslation() + direction;
  }
  return mBeamAttack.x7c_.GetTranslation();
}

CVector3f CGrenchler::GetPlayerBeamTargetPosition(CStateManager& mgr) const {
  CPlayer* player = mgr.GetPlayer(0);
  CVector3f position = player->GetAimPosition(mgr, 0.f);
  if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    position += CVector3f(0.f, 0.f, -0.25f);
  } else {
    position += CVector3f(0.f, 0.f, -0.5f);
  }
  return position;
}

void CGrenchler::UpdateEffects(float dt, CStateManager& mgr) {
  const float waterHeight = GetWaterSurfaceHeight(mgr);
  if (mEffectD.x0_.mGen.get() != nullptr) {
    mEffectD.x0_.mGen->SetParticleEmission(false);
    if (IsWadingInWater(mgr) == true) {
      mEffectD.x0_.mGen->SetGeneratorRate(0.f);
      mEffectD.x20_ += dt;
      mEffectD.x0_.mGen->SetParticleEmission(true);
      mEffectD.x0_.mGen->SetGlobalScale(0.7f * CVector3f::One());
      float interval = 0.75f;
      CVector3f travelled = mEffectD.x8_ - GetTranslation();
      const float distance = travelled.Magnitude();
      if (dt > 0.f) {
        const float speed = distance / dt;
        if (speed > 4.5f) {
          interval = 0.05f;
        } else if (speed > 3.f) {
          interval = 0.11f;
        } else if (speed > 1.5f) {
          interval = 0.18f;
        }
      }
      interval *= (mEffectD.x14_.GetZ() < waterHeight ? 0.333f : 0.5f);
      mEffectD.x8_ = GetTranslation();
      const float surfaceZ = 0.05f + waterHeight;
      while (mEffectD.x20_ > interval) {
        mEffectD.x20_ -= interval;
        int variant;
        if (mEffectD.x14_.GetZ() < waterHeight) {
          const float roll = mgr.Random()->Range(0.f, 1.f);
          if (roll < 0.4f) {
            variant = 0;
          } else if (roll < 0.7f) {
            variant = 1;
          } else {
            variant = 2;
          }
        } else {
          const float roll = mgr.Random()->Range(0.f, 1.f);
          if (roll < 0.5f) {
            variant = 1;
          } else {
            variant = 2;
          }
        }
        CVector3f position = CVector3f::Zero();
        switch (variant) {
        case 0:
          position = mEffectD.x14_;
          break;
        case 1:
          position = GetBlendedLocatorPosition(waterHeight, "L_knee", "L_knee_2");
          break;
        case 2:
          position = GetBlendedLocatorPosition(waterHeight, "R_knee", "R_knee_2");
          break;
        }
        position.SetZ(surfaceZ);
        mEffectD.x0_.mGen->SetTranslation(position);
        mEffectD.x0_.mGen->ForceParticleCreation(1);
      }
    }
    mEffectD.x0_.mGen->Update(0.75f * dt);
  }

  if (mEffectA.x4_.mGen.get() != nullptr) {
    if (IsDeeplySubmerged(mgr) == true) {
      if (GetTranslation().GetZ() < waterHeight - 5.5f) {
        mEffectA.x4_.mGen->SetParticleEmission(false);
      } else {
        mEffectA.x4_.mGen->Update(dt);
        mEffectA.x4_.mGen->SetParticleEmission(true);
        mEffectA.x4_.mGen->SetTranslation(
            CVector3f(GetTranslation().GetX(), GetTranslation().GetY(), waterHeight));
      }
      if (GetTranslation().GetZ() > waterHeight - 4.f) {
        AnimationData()->SetEffectState(rstl::string_l(skBodyBubblesEffect), false, mgr);
      } else {
        AnimationData()->SetEffectState(rstl::string_l(skBodyBubblesEffect), true, mgr);
      }
    } else {
      mEffectA.x4_.mGen->SetParticleEmission(false);
      AnimationData()->SetEffectState(rstl::string_l(skBodyBubblesEffect), false, mgr);
    }
  }

  if (mEffectA.x18_ != kInvalidUniqueId) {
    CEntity* entity = mgr.ObjectById(mEffectA.x18_);
    if (entity != nullptr && entity->GetActive() == true) {
      CPlasmaProjectile* projectile = static_cast< CPlasmaProjectile* >(entity);
      CTransform4f xf(CTransform4f::LookAt(mBeamAttack.x7c_.GetTranslation(), mBeamAttack.x50_,
                                           CVector3f::Up()));
      projectile->UpdateFx(xf, dt, mgr);
    }
  }

  CVector3f targetPosition = CVector3f::Zero();
  if (mEffectC.x24_.mGen.get() != nullptr) {
    if (mEffectC.x90_) {
      CSfxManager::UpdateEmitter(mEffectC.x90_, mBeamAttack.x7c_.GetTranslation(),
                                 GetTransform().GetForward(), 127);
    }
    mEffectC.x24_.mGen->SetGlobalTranslation(mBeamAttack.x7c_.GetTranslation());
    if (xa78_ == 14) {
      targetPosition = GetPlayerTargetPosition(mgr);
      CVector3f toTarget = targetPosition - mBeamAttack.x7c_.GetTranslation();
      mEffectC.x24_.mGen->SetGlobalScale(CVector3f(3.f, 0.16f * toTarget.Magnitude(), 3.f));
      CTransform4f xf(
          CTransform4f::LookAt(mBeamAttack.x7c_.GetTranslation(), targetPosition, CVector3f::Up()));
      mEffectC.x24_.mGen->SetGlobalOrientation(xf);
    } else {
      targetPosition = GetHornTargetPosition();
      if (mEffectC.x48_ == 2 || mEffectC.x48_ == 3) {
        targetPosition = mEffectC.x4c_;
        targetPosition.SetZ(mBeamAttack.x7c_.GetTranslation().GetZ());
      }
      CVector3f toTarget = targetPosition - mBeamAttack.x7c_.GetTranslation();
      mEffectC.x24_.mGen->SetGlobalScale(CVector3f(3.f, 0.16f * toTarget.Magnitude(), 3.f));
      CTransform4f xf(
          CTransform4f::LookAt(mBeamAttack.x7c_.GetTranslation(), targetPosition, CVector3f::Up()));
      mEffectC.x24_.mGen->SetGlobalOrientation(xf);
    }
    mEffectC.x24_.mGen->Update(dt);
  }

  if (mEffectC.x30_.mGen.get() != nullptr) {
    CVector3f step = targetPosition - mBeamAttack.x7c_.GetTranslation();
    int count = static_cast< int >(step.Magnitude() / 0.4f);
    if (mEffectC.x30_.mGen->GetParticleCount() < count) {
      CElementGen* gen = mEffectC.x30_.mGen.get();
      gen->ForceParticleCreation(count - gen->GetParticleCount());
      count = mEffectC.x30_.mGen->GetParticleCount();
    }
    if (step.CanBeNormalized() == true) {
      step.Normalize();
      step *= 0.4f;
    }
    CVector3f position = mBeamAttack.x7c_.GetTranslation();
    for (int i = 0; i < count; ++i) {
      CElementGen::CParticle& particle = mEffectC.x30_.mGen->mParticles[i];
      particle.mPos = position + mgr.Random()->Float() * step;
      particle.mLineLengthOrSize = 0.5f;
      position += step;
    }
    for (int i = count; i < mEffectC.x30_.mGen->GetParticleCount(); ++i) {
      CElementGen::CParticle& particle = mEffectC.x30_.mGen->mParticles[i];
      particle.mPos = CVector3f(0.f, 0.f, -10000.f);
      particle.mLineLengthOrSize = 0.f;
    }
    mEffectC.x30_.mGen->Update(dt);
  }

  if (mIsGrappleGuardian == true) {
    bool electric = false;
    bool alive = false;
    if (xc6c_ == 1 && GetAlive() == true) {
      alive = true;
    }
    if (alive && xa78_ != 17) {
      electric = true;
    }
    for (int i = 0; i < 5; ++i) {
      AnimationData()->SetEffectState(rstl::string_l(skElectricEffects[i]), electric, mgr);
    }
    if (electric == true) {
      if (!xec8_) {
        xec8_ = PlayCustomSound(xc3c_.GetTranslation(), GetTransform().GetForward(),
                                mAudioPlaybackParms, true);
      }
      CSfxManager::UpdateEmitter(xec8_, xc3c_.GetTranslation(), GetTransform().GetForward(), 127);
    } else if (xec8_) {
      CSfxManager::RemoveEmitter(xec8_);
      xec8_.Clear();
    }
    UpdateExplosionHeight(mgr);
    if (mEffectA.x3c_.mGen.get() != nullptr) {
      if (CanCrystalTakeDamage() == true) {
        const float next = mEffectA.x44_ + dt;
        mEffectA.x44_ = 1.f < next ? 1.f : next;
        mEffectA.x3c_.mGen->SetParticleEmission(true);
      } else {
        const float next = mEffectA.x44_ - dt;
        mEffectA.x44_ = next < 0.f ? 0.f : next;
        if (0.f == mEffectA.x44_) {
          mEffectA.x3c_.mGen->SetParticleEmission(false);
        }
      }
      mEffectA.x3c_.mGen->SetGlobalTranslation(xe2c_);
      mEffectA.x3c_.mGen->SetGlobalScale(mEffectA.x44_ * CVector3f::One());
      mEffectA.x3c_.mGen->Update(dt);
    }
  }

  if (BodyController()->IsFrozen() == true) {
    StopElectricBeam(mgr);
  }
}

void CGrenchler::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {}

bool CGrenchler::IsWadingInWater(CStateManager& mgr) {
  if (mEffectD.x0_.mGen.get() != nullptr && !x9fc_) {
    float waterHeight = GetWaterSurfaceHeight(mgr);
    if (0.2f + GetTranslation().GetZ() < waterHeight) {
      return true;
    }
  }
  return false;
}

void CGrenchler::UpdateFacingDirection() {
  if (x99c_) {
    CVector3f direction = GetTransform().GetForward();
    direction.SetZ(0.f);
    if (direction.CanBeNormalized() == true) {
      direction.Normalize();
      static_cast< SConeVulnerability* >(x99c_.GetPtr())->SetDirection(direction);
    }
  }
}

void CGrenchler::UpdateGuardianBlend(float dt, CStateManager& mgr) {
  if (mIsGrappleGuardian) {
    if (mX90c_5_) {
      float step = dt / 0.5f;
      if (IsFacingTarget(mgr, 1.5707964f) == true) {
        xe28_ -= step;
      } else {
        xe28_ += step;
      }
      xe28_ = 0.f > xe28_ ? 0.f : (1.f < xe28_ ? 1.f : xe28_);
    }
  }
}

void CGrenchler::UpdateTeammateScan(float dt, CStateManager& mgr) {
  if (GetTeamAiMgr(mgr) != nullptr) {
    xf1c_ -= dt;
    if (xf1c_ < 0.f) {
      xf20_ = GetTeamAiMgr(mgr)->TouchingAnyTeammates(mgr, GetUniqueId(), 0.5f);
      if (xf20_ != kInvalidUniqueId) {
        xf1c_ = 2.f;
      } else {
        xf1c_ = mgr.Random()->Range(0.2f, 0.4f);
      }
    }
  }
}

void CGrenchler::UpdateMovement(float dt, CStateManager& mgr) {
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(CMaterialList(sSolidMaterial),
                                                                     CMaterialList(sFloorMaterial));
  if (mIsGrappleGuardian == true) {
    if (mHasHealthBar) {
      CVector3f toTarget = mVectors.xc_ - mVectors.x0_;
      CVector3f flat(toTarget.GetX(), toTarget.GetY(), 0.f);
      if (flat.CanBeNormalized() == true) {
        flat.Normalize();
      }
      const float flatY = flat.GetY();
      const float negFlatY = -flatY;
      const float flatX = flat.GetX();
      const CVector3f origin = mVectors.x0_ + flat;
      const float distance = 3.6f * toTarget.Magnitude();
      toTarget.Normalize();
      if (!CGameCollision::RayStaticLineOfSightTest(
              mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
              origin + CVector3f(flatY, -flatX, 0.f), toTarget, distance, filter) ||
          !CGameCollision::RayStaticLineOfSightTest(
              mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
              origin + CVector3f(negFlatY, flatX, 0.f), toTarget, distance, filter)) {
        MoveInOneFrameOR(2.f * (0.15f * (-1.f * CVector3f::Forward())), dt);
      }
    }
  } else {
    float distance = 1.7f;
    if (xa78_ == 1 || xa78_ == 2) {
      distance += 0.5f;
    }
    if (!CGameCollision::RayStaticLineOfSightTest(mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
                                                  mBeamAttack.x7c_.GetTranslation() -
                                                      CVector3f(0.f, 0.f, 1.f),
                                                  CVector3f::Up(), distance, filter)) {
      MoveInOneFrameOR(2.f * (0.15f * (-1.f * CVector3f::Forward())), dt);
    }
  }
}

void CGrenchler::Pursue(CStateManager& mgr, EStateMsg msg, float dt) {
  CVector3f target = GetTargetPosition(mgr);
  if (mIsGrappleGuardian == true) {
    target.SetZ(GetTranslation().GetZ());
  }

  bool inWater = IsDeeplySubmerged(mgr);
  if (msg == kStateMsg_Activate || x9fc_ != inWater) {
    SetSubmerged(inWater);
  }

  switch (msg) {
  case kStateMsg_Activate:
    mX90c_0_ = true;
    xa80_ = 100.f;
    xa90_ = 0.f;
    xa8c_ = 0.f;
    xa88_ = 0.f;
    xa7c_ = 0.f;
    xa84_ = 0.f;
    xa94_ = BodyController()->GetTurnSpeed();
    mReflectInfo.Reset();
    MoveToTarget(mgr, 0.f, target);
    xa8c_ = mgr.Random()->Range(5.f, 13.f);
    ResetBodyVulnerabilities(mgr, CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
    break;
  case kStateMsg_Update: {
    if (!mIsGrappleGuardian) {
      if (!IsNearPath(GetTranslation() + CVector3f(0.f, 0.f, 1.f), 0.f)) {
        if (FindJumpTarget(mgr, true) == true) {
          return;
        }
        BodyController()->SetTurnSpeed(2.f * xa94_);
      } else {
        BodyController()->SetTurnSpeed(xa94_);
        if (mReflectInfo.x50_ + 3.f < x8ac_) {
          xa80_ += dt;
          if (xa80_ > 0.5f) {
            xa80_ = 0.f;
            bool frustrated = false;
            if (Stuck(mgr, CTriggerData(0.f)) == true) {
              if (Frustrated(mgr, CTriggerData(0.f)) == true) {
                frustrated = true;
              }
            }
            if (FindJumpTarget(mgr, frustrated) == true) {
              return;
            }
          }
        }
      }
    } else if (mPredictedLeashTime > 5.f && x904_ + 0.5f < x8ac_) {
      const float height = mgr.Random()->Range(2.f, 4.f);
      SetTranslation(GetTranslation() + CVector3f(0.f, 0.f, height));
      ResolveCollision(mgr);
      ResetSteering();
      AddMaterial(kMT_GroundCollider, mgr);
      x904_ = x8ac_;
      xa90_ = -1.f;
    }

    xa90_ += dt;
    if (xa90_ > 0.25f) {
      xa90_ = 0.f;
      if (mPathFindSearch.RemainingPathDistance(GetTranslation()) > 25.f && !mIsGrappleGuardian) {
        ResetSteering();
      } else {
        UpdateChargeSteering();
      }
    }

    if (!mIsGrappleGuardian || x8bc_ + 1.5f < x8ac_) {
      if (!mX90c_6_ && (target - x8b0_).MagSquared() > 16.f) {
        MoveToTarget(mgr, dt, target);
      }
    }

    CVector3f moveVector = CVector3f::Zero();
    if (PathShagged(mgr, CTriggerData(0.f)) == true && !mIsGrappleGuardian) {
      BodyController()->CommandMgr().ClearLocomotionCmds();
      mX90c_6_ = false;
      xa88_ += dt;
    } else if (mPathFindSearch.IsOver()) {
      MoveToTarget(mgr, dt, target);
      xa88_ = 0.f;
    } else {
      mPathFindNavigation.PathFind(mgr, msg, dt, *this);
      moveVector = BodyController()->CommandMgr().GetMoveVector();
      xa88_ = 0.f;
    }

    if (PathShagged(mgr, CTriggerData(0.f)) == true &&
        (target - GetTranslation()).Magnitude() > 2.f) {
      xa7c_ += dt;
    } else {
      xa7c_ = 0.f;
    }

    BodyController()->CommandMgr().DeliverCmd(CBCLocomotionCmd(moveVector, CVector3f::Zero(), 1.f));
    xa84_ += dt;
    if (xa84_ > 1.f) {
      xeac_ = 0;
    }
    break;
  }
  case kStateMsg_Deactivate:
    UpdateChargeSteering();
    BodyController()->SetTurnSpeed(xa94_);
    break;
  }
}

void CGrenchler::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (!mStateMachine->HasState()) {
    SetupStateMachineHelper(mgr);
    return;
  }

  if (mCollisionManager.get() == nullptr) {
    CreateCollisionManager(mgr);
  }

  if (!mAlive) {
    CPatterned::Think(dt, mgr);
    return;
  }

  x8ac_ += dt;
  if (!BodyController()->GetIsActive()) {
    BodyController()->Activate(mgr, pas::kAS_Invalid);
  }
  CPatterned::Think(dt, mgr);
  ThinkBoneTracking(dt, mgr);

  if (mCollisionManager.get() != nullptr) {
    AnimationData()->PreRender();
    mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  }

  if (!mIsGrappleGuardian) {
    mSurfaceAlignment.Update(*this, mgr, dt);
  }

  if (InFluidId() != kInvalidUniqueId) {
    xa04_ = InFluidId();
  }

  UpdateEffects(dt, mgr);
  UpdateDamageFlash(dt);

  if (BodyController()->IsFrozen() == true && xc6c_ != 1 &&
      GetHealthInfo()->GetHP() < GetTailHealth()) {
    DestroyTail(mgr);
  }

  UpdateFacingDirection();
  UpdateGuardianBlend(dt, mgr);
  UpdateMovement(dt, mgr);
  UpdateTeammateScan(dt, mgr);
}

void CGrenchler::PreThink(float dt, CStateManager& mgr) {
  mBoneTracking.PreThink(*AnimationData());
  CPatterned::PreThink(dt, mgr);
}

bool CGrenchler::CanBeUnPossessed(CStateManager& mgr) const { return false; }

bool CGrenchler::PlayYellowHitReact(CStateManager& mgr, const CTriggerData& data) const {
  return xe6c_ == 4;
}

void CGrenchler::WalkTowardPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  Pursue(mgr, msg, dt);
}

bool CGrenchler::Submerged(CStateManager& mgr, const CTriggerData& data) const { return x9fc_; }

void CGrenchler::SetSubmerged(bool submerged) {
  if (submerged == true) {
    BodyController()->SetLocomotionType(pas::kLT_Internal14);
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_WorldUp);
  } else {
    if (x9fc_ == true && xa78_ != 12) {
      xa00_ = 1.f + x8ac_;
    }
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
  }
  x9fc_ = submerged;
}

CPFArea* CGrenchler::GetPathArea(CStateManager& mgr) const {
  const TAreaId areaId = GetCurrentAreaId();
  return mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea;
}

void CGrenchler::SetPathArea(CStateManager& mgr) {
  const TAreaId areaId = GetCurrentAreaId();
  mPathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed()->mPathArea);
}

void CGrenchler::UnmarkPathRegion(CStateManager& mgr) {
  if (x908_ == -1 || GetPathArea(mgr) == nullptr) {
    return;
  }
  CPFRegion* region = GetPathArea(mgr)->GetRegionPtr(x908_);
  if (region != nullptr) {
    region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() & ~1);
  }
  x908_ = -1;
}

CVector3f CGrenchler::GetTargetPosition(CStateManager& mgr) const {
  return mgr.GetPlayer(0)->GetTranslation();
}

bool CGrenchler::InRange(CStateManager& mgr, float minRange, float maxRange) const {
  const float distanceSquared = (GetTargetPosition(mgr) - GetTranslation()).MagSquared();
  return distanceSquared > minRange * minRange && distanceSquared < maxRange * maxRange;
}

bool CGrenchler::InBiteRange(CStateManager& mgr, const CTriggerData& data) const {
  const CVector3f target = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
  const float dx = target.GetX() - mBiteAttack.x30_.Get03();
  const float dy = target.GetY() - mBiteAttack.x30_.Get13();
  const float dz = target.GetZ() - mBiteAttack.x30_.Get23();
  const float heightLimit = mIsGrappleGuardian ? 7.f : 3.5f;
  if (dz > heightLimit || dz < -heightLimit) {
    return false;
  }
  if (mIsGrappleGuardian == true && xa78_ != 8) {
    const float gx = target.GetX() - mVectors.xc_.GetX();
    const float gy = target.GetY() - mVectors.xc_.GetY();
    const float gz = target.GetZ() - mVectors.xc_.GetZ();
    if (gx * gx + gy * gy + gz * gz < mBiteAttack.mMaxRange * mBiteAttack.mMaxRange) {
      return true;
    }
  }
  const float distanceSquared = 0.f + (dx * dx + dy * dy);
  return distanceSquared > mBiteAttack.mMinRange * mBiteAttack.mMinRange &&
         distanceSquared < mBiteAttack.mMaxRange * mBiteAttack.mMaxRange;
}

bool CGrenchler::InBurstRange(CStateManager& mgr, const CTriggerData& data) const {
  return InRange(mgr, mBurstAttack.mMinRange, mBurstAttack.mMaxRange);
}

bool CGrenchler::InBeamRange(CStateManager& mgr, const CTriggerData& data) const {
  float maxRange = mBeamAttack.mMaxRange;
  if (xa88_ > 2.f) {
    maxRange *= 2.f;
  }
  return InRange(mgr, mBeamAttack.mMinRange, maxRange);
}

bool CGrenchler::InChargeRange(CStateManager& mgr, const CTriggerData& data) const {
  return InRange(mgr, mChargeAttack.x18_, mChargeAttack.x1c_);
}

bool CGrenchler::Bored(CStateManager& mgr, const CTriggerData& data) const {
  if (mIsGrappleGuardian == true) {
    return false;
  }
  if (mX90c_6_ == true) {
    return false;
  }
  return FacingPlayer(mgr, CTriggerData(0.f)) ? xa88_ > xa8c_ : xa88_ > 1.f;
}

const CDamageVulnerability* CGrenchler::GetDamageVulnerability() const {
  if (xa78_ == 12 && !mReflectInfo.x1c_3_ && !mReflectInfo.x1c_5_) {
    return &mReflectInfo.mVulnerability;
  }
  return CPatterned::GetDamageVulnerability();
}

void CGrenchler::YellowHitReact(CStateManager& mgr, EStateMsg msg, float dt) {
  SetAttackState(kGA_YellowHitReact, msg);
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(pas::kTT_Nine));
  if (msg == kStateMsg_Activate) {
    if (xc6c_ == 1) {
      SetupBodyVulnerabilities(mgr, CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
    }
  } else if (msg == kStateMsg_Deactivate) {
    ResetBodyVulnerabilities(mgr, CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
  }
}

void CGrenchler::Null(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    ResetBodyVulnerabilities(mgr, CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
  }
}

void CGrenchler::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    if (IsDeeplySubmerged(mgr) == true) {
      xf18_ = pas::kTT_Four;
    } else if (mgr.Random()->Range(0.f, 1.f) < 0.5f) {
      xf18_ = pas::kTT_Zero;
    } else {
      xf18_ = pas::kTT_Five;
    }
  }
  SetAttackState(kGA_Taunt, msg);
  DeliverCommand(msg, pas::kAS_Taunt, CBCTauntCmd(xf18_));
}

void CGrenchler::MarkPathRegion(CStateManager& mgr) {
  const rstl::reserved_vector< CVector3f, 16 >& waypoints = mPathFindSearch.GetWaypoints();
  if (waypoints.size() != 0) {
    const CVector3f lastPoint = waypoints[waypoints.size() - 1];
    const CPFRegion* lastRegion = GetPathArea(mgr)->FindClosestRegion(
        lastPoint, GetSearchPath()->GetRegionFlags(), GetSearchPath()->GetCreatureMask(), 2.f);
    for (int i = waypoints.size() - 1; i > 0; --i) {
      const CVector3f midpoint = 0.5f * (waypoints[i] + waypoints[i - 1]);
      CPFRegion* region = GetPathArea(mgr)->FindClosestRegion(
          midpoint, GetSearchPath()->GetRegionFlags(), GetSearchPath()->GetCreatureMask(), 2.f);
      if (region && region != lastRegion && region->Data()->GetAvoidanceFlags() == 0) {
        region->Data()->SetAvoidanceFlags(region->Data()->GetAvoidanceFlags() | 1);
        x908_ = region->GetIndex();
        break;
      }
    }
  }
}

CScriptAiJumpPoint* CGrenchler::FindJumpPoint(CStateManager& mgr, const CVector3f& target,
                                              TUniqueId exclude) {
  CScriptAiJumpPoint* best = nullptr;
  float bestCost = FLT_MAX;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int index = list.GetFirstObjectIndex(); index != -1;
       index = list.GetNextObjectIndex(index)) {
    CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(list[index]);
    if (jumpPoint == nullptr || !jumpPoint->GetActive() ||
        jumpPoint->GetCurrentAreaId() != GetCurrentAreaId() ||
        jumpPoint->GetUniqueId() == exclude) {
      continue;
    }
    const CScriptWaypoint* waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(jumpPoint->GetJumpPoint()));
    if (waypoint == nullptr) {
      continue;
    }
    if (GetSearchPath()->PathExists(GetTranslation(), jumpPoint->GetTranslation()) !=
        CPathFindSearch::kR_Success) {
      continue;
    }
    const CVector3f toJump = GetTranslation() - jumpPoint->GetTranslation();
    const CVector3f jumpToWaypoint = jumpPoint->GetTranslation() - waypoint->GetTranslation();
    const CVector3f waypointToTarget = waypoint->GetTranslation() - target;
    const float height = fabs(toJump.GetZ());
    float cost = toJump.Magnitude() + height;
    cost = jumpToWaypoint.Magnitude() + cost;
    cost = waypointToTarget.Magnitude() + cost;
    if (GetSearchPath()->PathExists(waypoint->GetTranslation(), target) !=
        CPathFindSearch::kR_Success) {
      cost += 10000.f;
    }
    if (cost < bestCost) {
      bestCost = cost;
      best = jumpPoint;
    }
  }
  return best;
}

void CGrenchler::MoveToTarget(CStateManager& mgr, float dt, const CVector3f& target) {
  x8b0_ = target;
  if (mIsGrappleGuardian == true) {
    x8b0_.SetZ(GetTranslation().GetZ());
  }
  x8bc_ = x8ac_;
  mPathFindNavigation.SetDestination(x8b0_);
  if (xa78_ == 4) {
    mPathFindSearch.SetAvoidanceFilter(0);
  } else {
    mPathFindSearch.SetAvoidanceFilter(1);
  }
  UnmarkPathRegion(mgr);
  mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
  mX90c_6_ = false;
  if (!mIsGrappleGuardian && PathShagged(mgr, CTriggerData(0.f)) == true) {
    CScriptAiJumpPoint* jumpPoint = FindJumpPoint(mgr, target, kInvalidUniqueId);
    if (jumpPoint != nullptr) {
      x8b0_ = jumpPoint->GetTranslation();
      mPathFindNavigation.SetDestination(x8b0_);
      mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
      if (!PathShagged(mgr, CTriggerData(0.f))) {
        mX90c_6_ = true;
      } else {
        jumpPoint = FindJumpPoint(mgr, target, jumpPoint->GetUniqueId());
        if (jumpPoint != nullptr) {
          x8b0_ = jumpPoint->GetTranslation();
          mPathFindNavigation.SetDestination(x8b0_);
          mPathFindNavigation.PathFind(mgr, kStateMsg_Activate, dt, *this);
          if (!PathShagged(mgr, CTriggerData(0.f))) {
            mX90c_6_ = true;
          }
        }
      }
    }
  }
  MarkPathRegion(mgr);
}

bool CGrenchler::ClearPathToPlayer(CStateManager& mgr, const CTriggerData&) const {
  const CVector3f target = GetTargetPosition(mgr) + CVector3f(0.f, 0.f, 0.5f);
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid, kMT_AIBlock),
      CMaterialList(kMT_Player, kMT_CollisionActor, kMT_AIPassthrough));
  if (mgr.RayCollideWorld(GetTranslation() + CVector3f(0.f, 0.f, 0.5f), target, filter, this)) {
    return IsNearPath(target, 0.f);
  }
  return false;
}

void CGrenchler::SpawnTail(CStateManager& mgr) {
  const bool ingPossessed = IsIngPossessed();
  const CAnimationParameters& tailParams = ingPossessed ? mTailDark : mTail;
  const uint tailAnim = x9fc_ ? (ingPossessed ? mTailWhenUnderwaterDark : mTailWhenUnderwater)
                              : tailParams.GetInitialAnimation();
  const TUniqueId uid = mgr.AllocateUniqueId();
  CTransform4f xf = GetTransform();
  xf.SetColumn(kDZ, CVector3f(0.f, 0.f, 1.f));
  xf.SetTranslation(xc3c_.GetTranslation() + CVector3f(0.f, 0.f, 4.f));
  CGrenchlerTail* tail =
      rs_new CGrenchlerTail(uid, GetCurrentAreaId(),
                            CModelData(CAnimRes(tailParams.GetACSFile(), tailParams.GetCharacter(),
                                                GetModelData()->GetScale(), tailAnim, true)),
                            tailAnim, xf, x9fc_);
  tail->SetModelFlags(GetModelFlags());
  tail->SetCalculateLighting(true);
  mgr.AddObject(tail);
}

void CGrenchler::DestroyTail(CStateManager& mgr) {
  xc6c_ = 1;
  xc80_ = 0;
  xc78_ = x8ac_;
  PlayTailDestroyedSound();
  BodyController()->DouseFlames();
  if (mIsGrappleGuardian == true) {
    xe60_ = 0.f;
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_SeekerTarget, mgr);
    UpdateChargeSteering();
  } else {
    ResetBodyVulnerabilities(mgr, CPatterned::GetDamageVulnerability()->MakeIgnoreRadius());
  }
  const bool ingPossessed = IsIngPossessed() == true;
  rstl::optional_object< TLockedToken< CSkinnedModel > >& skinnedModel =
      ingPossessed ? mTaillessSkinnedModelDark : mTaillessSkinnedModel;
  const CAssetId modelRes = ingPossessed ? mTaillessModelDark : mTaillessModel;
  const CAssetId skinRes = ingPossessed ? mTaillessSkinRulesDark : mTaillessSkinRules;
  skinnedModel = TLockedToken< CSkinnedModel >(
      rs_new CSkinnedModel(gpSimplePool->GetObj(SObjectTag('CMDL', modelRes)),
                           gpSimplePool->GetObj(SObjectTag('CSKR', skinRes)),
                           AnimationData()->GetModelData()->GetLayoutInfo()));
  AnimationData()->SetSkinnedModel(*skinnedModel);
  SpawnTail(mgr);
  SendScriptMsgs(kSS_DGNR, mgr, kInvalidUniqueId, kSM_None);
  BodyController()->UnFreeze();
}

CGrenchler::CGrenchler(
    TUniqueId uid, const rstl::string& name, CEntityInfo& info, const CTransform4f& xf,
    const CModelData& modelData, const CPatternedInfo& patternedInfo, CAssetId fsmId,
    float tailDestroyedHealth, float minTimeBetweenCharges, float unknown_0x7bd1a35f,
    float chargeAttackMinRange, float chargeAttackMaxRange, float biteAttackMinRange,
    float biteAttackMaxRange, float biteAttackMinPause, bool isGrappleGuardian, bool hasHealthBar,
    const CDamageVulnerability& vulnerability, CAssetId tailAncs, int tailCharacter,
    int tailInitialAnim, uint tailWhenUnderwater, CAssetId taillessModel,
    CAssetId taillessSkinRules, CAssetId tailDarkAncs, int tailDarkCharacter,
    int tailDarkInitialAnim, uint tailWhenUnderwaterDark, CAssetId taillessModelDark,
    CAssetId taillessSkinRulesDark, ushort tailHitSound, ushort tailDestroyedSound,
    float biteAttackMaxPause, float biteAttackDamageRadius, const CDamageInfo& biteDamage,
    const CDamageInfo& beamDamage, float beamAttackMinRange, float beamAttackMaxRange,
    float beamAttackMinPause, float beamAttackMaxPause, float beamAttackMaxAngle,
    const SLdrAudioPlaybackParms& beamAttackSound, const CDamageInfo& burstDamage,
    CAssetId burstProjectile, float burstAttackMinRange, float burstAttackMaxRange,
    float burstAttackMinPause, float burstAttackMaxPause, float burstAttackDamageRadius,
    CAssetId surfaceRingsEffect, CAssetId shallowWaterRing, CAssetId electricEffect, CAssetId pART,
    CAssetId grappleSwoosh, CAssetId grappleBeamPart, CAssetId grappleHitFx,
    const CDamageInfo& grappleDamage, const SLdrAudioPlaybackParms& grappleBeamSound,
    CAssetId beamEffect, int unknown_0xd4753ff4, float unknown_0x05fc6001, float unknown_0x13e5b580,
    float unknown_0xfc6f199d, CAssetId grappleVisorEffect, const CDamageInfo& damageInfo,
    CAssetId pART_0x54b6bfa1, const SLdrAudioPlaybackParms& audioPlaybackParms,
    CAssetId grappleGuardianEyeGlow, CAssetId alternateScannableInfo,
    const CActorParameters& actorParams)
: CPatterned(kPAI_Grenchler, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mPathFindSearch(nullptr, 0x11 + (patternedInfo.GetIngPossessionData().isAnEncounter ? 0x200 : 0),
                  patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0, CPFRegion::kRP_Center)
, x8ac_(0.f)
, x8b0_(CVector3f::Zero())
, x8bc_(-1000.f)
, mBoneTracking(*AnimationData(), rstl::string_l("head"), 0.5235988f, 0.9424779f, 1)
, x8fc_(-1000.f)
, x900_(-1000.f)
, x904_(-1000.f)
, x908_(-1)
, mX90c_0_(false)
, mX90c_1_(false)
, mIsGrappleGuardian(isGrappleGuardian)
, mHasHealthBar(hasHealthBar)
, mX90c_4_(false)
, mX90c_5_(false)
, mX90c_6_(false)
, mSurfaceAlignment(CMaterialFilter::MakeExclude(CMaterialList(kMT_Ceiling, kMT_Wall)))
, mAlternateScanInfo(nullptr)
, mDamageVulnerability(vulnerability)
, x99c_()
, mFsm(gpSimplePool->GetObj(SObjectTag('FSM2', fsmId)))
, mTaillessModel(taillessModel)
, mTaillessSkinRules(taillessSkinRules)
, mTaillessModelDark(taillessModelDark)
, mTaillessSkinRulesDark(taillessSkinRulesDark)
, x9fc_(false)
, xa00_(-1000.f)
, xa04_(kInvalidUniqueId)
, xa06_(false)
, mCollisionManager(nullptr)
, xa74_(kGA_Invalid)
, xa78_(kGA_Invalid)
, xa7c_(0.f)
, xa80_(100.f)
, xa84_(0.f)
, xa88_(0.f)
, xa8c_(0.f)
, xa90_(0.f)
, xa94_(160.f)
, mBiteAttack(biteDamage, biteAttackMinRange, biteAttackMaxRange, biteAttackMinPause,
              biteAttackMaxPause, biteAttackDamageRadius)
, mBeamAttack(beamDamage, beamAttackSound, beamAttackMinRange, beamAttackMaxRange,
              beamAttackMinPause, beamAttackMaxPause, beamAttackMaxAngle)
, mBurstAttack(burstProjectile, burstDamage, burstAttackMinRange, burstAttackMaxRange,
               burstAttackMinPause, burstAttackMaxPause, burstAttackDamageRadius)
, xc3c_(CTransform4f::Identity())
, xc6c_(0)
, xc70_(-1000.f)
, mTailHealth(tailDestroyedHealth)
, xc78_(-1000.f)
, mTailHitSound(tailHitSound)
, mTailDestroyedSound(tailDestroyedSound)
, xc80_(0)
, mChargeAttack(minTimeBetweenCharges, unknown_0x7bd1a35f, chargeAttackMinRange,
                chargeAttackMaxRange)
, xcb8_(0.f)
, xcbc_(0)
, xcc0_(0.f)
, xcc4_(0.f)
, xcc8_(0)
, xccc_(kInvalidUniqueId)
, xcd0_(CVector3f::Zero())
, xcdc_(0.f)
, xce0_(0.f)
, xce4_(0)
, xce8_(-1000.f)
, xcec_(160.f)
, xcf0_(-1000.f)
, xcf4_(0)
, mGrappleEffect(pART)
, mEffectA(surfaceRingsEffect, electricEffect, beamEffect, grappleHitFx, grappleGuardianEyeGlow)
, mEffectB(unknown_0xd4753ff4, grappleVisorEffect, unknown_0x05fc6001)
, mEffectC(grappleSwoosh, grappleBeamPart, grappleDamage, grappleBeamSound)
, xe24_(0.f)
, xe28_(0.f)
, xe2c_(CVector3f::Zero())
, mTail(tailAncs, tailCharacter, tailInitialAnim)
, mTailWhenUnderwater(tailWhenUnderwater)
, mTailDark(tailDarkAncs, tailDarkCharacter, tailDarkInitialAnim)
, mTailWhenUnderwaterDark(tailWhenUnderwaterDark)
, xe58_(0.f)
, xe5c_(0.f)
, xe60_(0.f)
, xe64_(unknown_0x13e5b580)
, xe68_(unknown_0xfc6f199d)
, xe6c_(0)
, xe70_(CTransform4f::Identity())
, mXea0_0_(false)
, mXea0_1_(false)
, mXea0_2_(false)
, xea4_(-1000.f)
, mXea8_0_(false)
, mXea8_1_(false)
, xeac_(0)
, mAudioPlaybackParms(audioPlaybackParms)
, xec8_()
, mDamageEffect(damageInfo, pART_0x54b6bfa1)
, xf18_(pas::kTT_Five)
, xf1c_(1.f)
, xf20_(kInvalidUniqueId)
, mEffectD(shallowWaterRing) {
  if (!mIsGrappleGuardian) {
    mSurfaceAlignment.SetMode(CSurfaceAlignmentHelper::kM_NearbySurface);
    mSurfaceAlignment.SetAngularRate(16.f);
  } else {
    KnockBackController().EnableFreeze(false);
    KnockBackController().EnableSlow(false);
    KnockBackController().EnableKnockBackPhysics(false);
    KnockBackController().EnableLaggedBurnDeath(false);
    KnockBackController().EnableBurnDeath(false);
    KnockBackController().EnableExplodeDeath(false);
  }

  if (mTailHealth < 0.f || mTailHealth > GetHealthInfo()->GetHP()) {
    mTailHealth = CMath::Clamp(0.f, mTailHealth, GetHealthInfo()->GetHP());
  }

  if (alternateScannableInfo != kInvalidAssetId) {
    mAlternateScanInfo = rs_new TLockedToken< CScannableObjectInfo >(
        gpSimplePool->GetObj(SObjectTag('SCAN', alternateScannableInfo)));
  }
}

CGrenchler::~CGrenchler() {}

static const char* const skGuardianBossName = "BossGrappleGuardian";

// Guessed name; per-bone collision actor description table.
struct SCollisionActorDesc {
  const char* mBoneName;
  float mRadius;
  float mScale;
  int mType;
  int x10_;
  bool x14_;
};

static const SCollisionActorDesc skCollisionActors[] = {
    {"Skeleton_Root", 1.3f, 1.6f, 0, 75, true}, {"R_hip", 0.9f, 0.9f, 0, 75, true},
    {"L_hip", 0.9f, 0.9f, 0, 75, true},         {"tailbone_1", 0.6f, 0.9f, 0, 75, true},
    {"tailbone_2", 0.45f, 0.55f, 0, 75, false}, {"horn_LCTR", 0.8f, 0.7f, 1, 105, true},
    {"eye", 0.8f, 0.9f, 1, 105, true},          {"jaw", 1.2f, 1.1f, 2, 105, true},
    {"R_knee", 0.9f, 0.9f, 2, 105, true},       {"L_knee", 0.9f, 0.9f, 2, 105, true},
};

float CGrenchler::GetTailHealth() const { return mTailHealth; }

float CGrenchler::GetVulnerableAngle() const {
  return mIsGrappleGuardian == true ? 0.7853982f : 1.3089969f;
}

bool CGrenchler::CanCrystalTakeDamage() const {
  if (!mIsGrappleGuardian) {
    return false;
  }
  if (!mAlive) {
    return false;
  }
  switch (xa78_) {
  case 8:
    return true;
  case 5:
  case 6:
  case 7:
  case 9:
  case 10:
  case 17:
    return false;
  }
  if (xc80_ == 1) {
    return false;
  }
  return mX90c_5_;
}

float CGrenchler::GetFacingAngleDiff(const CTransform4f& xf) const {
  CVector3f otherForward = xf.GetForward();
  CVector3f forward = GetTransform().GetForward();
  forward.SetZ(0.f);
  otherForward.SetZ(0.f);
  if (!otherForward.CanBeNormalized() || !forward.CanBeNormalized()) {
    return 6.2831855f;
  }
  return CVector3f::GetAngleDiff(otherForward.AsNormalized(), forward.AsNormalized());
}

void CGrenchler::StartHitReaction(bool tailDestroyed) {
  KnockBackController().EnableAllAnimReactions(false);
  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_Flinch, xc6c_ != 1);
  KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack,
                                           tailDestroyed == true || xc6c_ != 1);
  pas::ESeverity severity;
  if (mIsGrappleGuardian == true && tailDestroyed == true) {
    severity = pas::kS_Ten;
  } else if (!x9fc_) {
    severity = tailDestroyed == true ? pas::kS_Three : pas::kS_Zero;
  } else {
    severity = tailDestroyed == true ? pas::kS_Five : pas::kS_Two;
  }
  KnockBackController().SetSeverity(severity);
  if (tailDestroyed == true) {
    BodyController()->CommandMgr().DeliverCmd(
        CBCKnockBackCmd(CVector3f::Forward(), severity, -1, true));
  }
}

bool CGrenchler::IsCrystalActor(TUniqueId id) {
  if (mCollisionManager.get() == nullptr) {
    return false;
  }
  for (uint i = 0; i < 10; ++i) {
    if (mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId() == id) {
      return skCollisionActors[i].mType == 1;
    }
  }
  return false;
}

bool CGrenchler::IsBodyActor(TUniqueId id) {
  if (mCollisionManager.get() == nullptr) {
    return false;
  }
  for (uint i = 0; i < 10; ++i) {
    if (mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId() == id) {
      return skCollisionActors[i].mType == 0;
    }
  }
  return false;
}

CScannableObjectInfo* CGrenchler::GetScannableObjectInfo() const {
  if (xc6c_ == 1 && GetAlive() == true && GetActive() == true) {
    if (mAlternateScanInfo.get() != nullptr) {
      return **mAlternateScanInfo;
    }
  }
  return CPatterned::GetScannableObjectInfo();
}

void CGrenchler::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetActive(mgr, true);
    }
    if (mIsGrappleGuardian == true) {
      if (mHasHealthBar == true) {
        mgr.SetBossParams(GetUniqueId(), GetHealthInfo()->GetInitialHP(),
                          gpStringTable->GetStringIndex(skGuardianBossName));
      }
      mChargeAttack.x24_ = x8ac_;
    }
    break;
  case kSM_Alert:
    mX90c_1_ = true;
    break;
  case kSM_Deactivate:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetActive(mgr, false);
    }
    break;
  case kSM_Delete:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->Destroy(mgr);
      mCollisionManager = nullptr;
    }
    if (mEffectA.x18_ != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mEffectA.x18_);
      mEffectA.x18_ = kInvalidUniqueId;
    }
    UnmarkPathRegion(mgr);
    DestroyGrappleBeam(mgr);
    if (xec8_) {
      CSfxManager::RemoveEmitter(xec8_);
      xec8_ = CSfxHandle();
    }
    break;
  case kSM_Launching:
    NotifyFalling(mgr, msg.GetSenderId());
    break;
  case kSM_Landed:
    if (xa78_ == 12) {
      ResetBodyVulnerabilities(mgr);
    }
    break;
  case kSM_Falling:
    if (mIsGrappleGuardian == true) {
      return;
    }
    break;
  case kSM_AreaLoaded:
    if (GetActive() && xccc_ == kInvalidUniqueId) {
      xccc_ = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
      JoinTeam(mgr);
    }
    AddMaterial(kMT_GroundCollider, mgr);
    SetPathArea(mgr);
    UpdateChargeSteering();
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionManager.get() != nullptr) {
      mCollisionManager->SetPhysicsActive(mgr, false);
    }
    break;
  case kSM_XXDG:
    mHitByPlayerProjectile = true;
    break;
  case kSM_ResistedDamage: {
    TUniqueId senderId = msg.GetSenderId();
    if (CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
      mHitByPlayerProjectile = true;
      if (IsCrystalActor(senderId) == true && CanCrystalTakeDamage() == true) {
        TUniqueId touchedId = collisionActor->GetLastTouchedObject();
        if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId))) {
          if (0.1f + xe58_ < CPatterned::skDamageHitTime) {
            if (xa78_ == 8) {
              xe5c_ += weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
              xe58_ = CPatterned::skDamageHitTime;
            } else if (xe6c_ == 0) {
              xe60_ += weapon->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
              xe58_ = CPatterned::skDamageHitTime;
              if (xe60_ >= xe68_) {
                xe6c_ = 4;
                xe60_ = 0.f;
              }
            }
          }
        }
      }
    }
    break;
  }
  case kSM_Damage:
    if (GetAlive()) {
      mHitByPlayerProjectile = true;
      TUniqueId senderId = msg.GetSenderId();
      if (CCollisionActor* collisionActor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
        CHealthInfo* collisionHealth = collisionActor->HealthInfo();
        bool inLanded = false;
        if (xa78_ == 12 && !mReflectInfo.x1c_3_ && !mReflectInfo.x1c_5_) {
          inLanded = true;
        }
        bool canDamage = xc80_ == 0;
        if (!mIsGrappleGuardian) {
          if (2.f + xc78_ > x8ac_) {
            canDamage = false;
          }
        } else if (xc6c_ == 1 && xa78_ != 17) {
          canDamage = false;
        }
        if (!inLanded && canDamage == true) {
          TUniqueId touchedId = collisionActor->GetLastTouchedObject();
          if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touchedId))) {
            if (mgr.GetPlayer(0)->GetUniqueId() != weapon->GetOwnerId()) {
              break;
            }
            CDamageInfo damage = weapon->GetCurrentDamageInfo();
            damage.SetRadius(0.f);
            bool tailDestroyed = false;
            if (xc6c_ != 1) {
              float scaledDamage = damage.GetDamage(*GetDamageVulnerability());
              float health = GetHealthInfo()->GetHP();
              if (health - scaledDamage < GetTailHealth()) {
                tailDestroyed = true;
                health = GetHealthInfo()->GetHP();
                const CDamageVulnerability* vulnerability = GetDamageVulnerability();
                damage.SetDamageFromVulnerability(*vulnerability, health - GetTailHealth());
                DestroyTail(mgr);
              } else {
                PlayTailHitSound();
              }
            }
            StartHitReaction(tailDestroyed);
            mgr.ApplyDamage(
                touchedId, GetUniqueId(), touchedId, damage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            x900_ = x8ac_;
            xcf0_ = -1000.f;
          } else {
            float damageAmount = 10000.f - collisionHealth->GetHP();
            bool tailDestroyed = false;
            if (xc6c_ != 1) {
              float health = GetHealthInfo()->GetHP();
              if (health - damageAmount < GetTailHealth()) {
                tailDestroyed = true;
                health = GetHealthInfo()->GetHP();
                damageAmount = health - GetTailHealth();
                DestroyTail(mgr);
              } else {
                PlayTailHitSound();
              }
            }
            StartHitReaction(tailDestroyed);
            TakeDamage(CVector3f::Zero(), damageAmount);
            x900_ = x8ac_;
            xcf0_ = -1000.f;
          }
        }
        collisionHealth->SetHP(10000.f);
        mHitByPlayerProjectile = true;
      }
    }
    break;
  case kSM_XHIT:
    if (xa78_ != 14 && xa78_ != 2 && xa78_ != 8) {
      TUniqueId senderId = msg.GetSenderId();
      if (CCollisionActor* collisionActor =
              TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId))) {
        if (collisionActor->GetLastTouchedObject() == mgr.GetPlayer(0)->GetUniqueId() &&
            mCurDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), mgr.GetPlayer(0)->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
          if (mIsGrappleGuardian == true && GetAlive() == true && xc6c_ == 1) {
            SendScriptMsgs(kSS_InternalState03, mgr, GetUniqueId(), kSM_None);
            CreateVisorEffect(mgr);
          }
        }
      }
    }
    break;
  case kSM_InternalMessage00:
    if (GetAlive() == true && GetActive() == true) {
      mX90c_4_ = true;
      mX90c_1_ = false;
    }
    break;
  case kSM_XCRT:
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
}

const CDamageVulnerability* CGrenchler::SConeVulnerability::GetDamageVulnerability(
    const CDamageVulnerability* vulnerability, const CVector3f& point, const CVector3f& direction,
    const CDamageInfo& info) {
  CVector3f flatDirection(direction.GetX(), direction.GetY(), 0.f);
  if (!flatDirection.CanBeNormalized()) {
    mResponse = mInsideResponse;
    return vulnerability;
  }

  if (CVector3f::GetAngleDiff(flatDirection.AsNormalized(), mDirection) > mMaxAngle) {
    mResponse = mOutsideResponse;
    return &CDamageVulnerability::ReflectVulnerabilty();
  }

  mResponse = mInsideResponse;
  return vulnerability;
}

bool CGrenchler::SConeVulnerability::GetCollisionResponseType(
    const CVector3f& point, const CVector3f& direction, const CWeaponMode& mode, int attributes,
    EWeaponCollisionResponseTypes& response) {
  response = mResponse;
  return true;
}

template < typename T >
void CGrenchler::DeliverCommand(EStateMsg msg, pas::EAnimationState state, const T& cmd) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mBodyController->CommandMgr().DeliverCmd(cmd);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*mBodyController, state)) {
      mBodyController->CommandMgr().DeliverCmd(cmd);
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

CEntity* REL_LoadGrenchler(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGrenchler sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGrenchler.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const ushort tailDestroyedSound = sldrThis.tailDestroyedSound == -1
                                        ? CSfxManager::kInternalInvalidSfxId
                                        : sldrThis.tailDestroyedSound;
  const ushort tailHitSound =
      sldrThis.tailHitSound == -1 ? CSfxManager::kInternalInvalidSfxId : sldrThis.tailHitSound;
  return rs_new CGrenchler(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData),
      sldrThis.patterned.stateMachine2, sldrThis.tailDestroyedHealth,
      sldrThis.minTimeBetweenCharges, sldrThis.unknown_0x7bd1a35f, sldrThis.chargeAttackMinRange,
      sldrThis.chargeAttackMaxRange, sldrThis.biteAttackMinRange, sldrThis.biteAttackMaxRange,
      sldrThis.biteAttackMinPause, sldrThis.isGrappleGuardian, sldrThis.hasHealthBar,
      LdrToDamageVulnerability(sldrThis.damageVulnerability), sldrThis.tail.ancs,
      sldrThis.tail.character_index, sldrThis.tail.initial_anim,
      sldrThis.tailWhenUnderwater.initial_anim, sldrThis.taillessModel, sldrThis.taillessSkinRules,
      sldrThis.tail_Dark.ancs, sldrThis.tail_Dark.character_index, sldrThis.tail_Dark.initial_anim,
      sldrThis.tailWhenUnderwater_Dark.initial_anim, sldrThis.taillessModel_Dark,
      sldrThis.taillessSkinRules_Dark, tailHitSound, tailDestroyedSound,
      sldrThis.biteAttackMaxPause, sldrThis.biteAttackDamageRadius,
      LdrToDamageInfo(sldrThis.biteDamage), LdrToDamageInfo(sldrThis.beamDamage),
      sldrThis.beamAttackMinRange, sldrThis.beamAttackMaxRange, sldrThis.beamAttackMinPause,
      sldrThis.beamAttackMaxPause, sldrThis.beamAttackMaxAngle, sldrThis.beamAttackSound_OneShot,
      LdrToDamageInfo(sldrThis.burstDamage), sldrThis.burstProjectile, sldrThis.burstAttackMinRange,
      sldrThis.burstAttackMaxRange, sldrThis.burstAttackMinPause, sldrThis.burstAttackMaxPause,
      sldrThis.burstAttackDamageRadius, sldrThis.surfaceRingsEffect, sldrThis.shallowWaterRing,
      sldrThis.electricEffect, sldrThis.pART, sldrThis.grappleSwoosh, sldrThis.grappleBeamPart,
      sldrThis.grappleHitFx, LdrToDamageInfo(sldrThis.grappleDamage),
      sldrThis.grappleBeamSound_Loop, sldrThis.beamEffect, sldrThis.unknown_0xd4753ff4,
      sldrThis.unknown_0x05fc6001, sldrThis.unknown_0x13e5b580, sldrThis.unknown_0xfc6f199d,
      sldrThis.grappleVisorEffect, LdrToDamageInfo(sldrThis.damageInfo), sldrThis.pART_0x54b6bfa1,
      sldrThis.audioPlaybackParms, sldrThis.grappleGuardianEyeGlow, sldrThis.alternateScannableInfo,
      LdrToActorParameters(sldrThis.actorInformation));
}

static void SetFuncPtrs() {
  static SGrenchler_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadGrenchler;
  SetSGrenchler_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSGrenchler_FuncPtrs(nullptr); }
