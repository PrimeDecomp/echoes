#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEffectWaypointPredicate.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CPirateEchoEmitter.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpacePirate.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "REL/REL_Setup.h"

#include "rstl/algorithm.hpp"

#include <float.h>

const SBurst CSpacePirate::skBurstsQuick[] = {
    {20, {3, 4, 5, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {2, 3, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {6, 5, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {1, 2, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {7, 6, 5, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsStandard[] = {
    {15, {5, 3, 2, 1, -1, 0, 0, 0}, 0.1f, 0.05f}, {20, {1, 2, 3, 4, -1, 0, 0, 0}, 0.1f, 0.05f},
    {20, {7, 6, 5, 4, -1, 0, 0, 0}, 0.1f, 0.05f}, {15, {3, 4, 5, 6, -1, 0, 0, 0}, 0.1f, 0.05f},
    {15, {6, 5, 4, 3, -1, 0, 0, 0}, 0.1f, 0.05f}, {15, {2, 3, 4, 5, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsFrenzied[] = {
    {40, {1, 2, 3, 4, 5, 6, -1, 0}, 0.1f, 0.05f}, {40, {7, 6, 5, 4, 3, 2, -1, 0}, 0.1f, 0.05f},
    {10, {2, 3, 4, 5, 4, 3, -1, 0}, 0.1f, 0.05f}, {10, {6, 5, 4, 3, 4, 5, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsJumping[] = {
    {20, {16, 4, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {5, 7, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {1, 10, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsInjured[] = {
    {15, {16, 1, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f}, {20, {3, 4, 6, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {25, {7, 5, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},  {25, {2, 6, 4, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {7, 5, 3, -1, 0, 0, 0, 0}, 0.1f, 0.05f},  {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsSeated[] = {
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsQuickOOV[] = {
    {10, {16, 15, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {13, 12, 10, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {9, 11, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {14, 10, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {10, {9, 11, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsStandardOOV[] = {
    {26, {16, 8, 11, 14, -1, 0, 0, 0}, 0.1f, 0.05f},
    {26, {16, 13, 11, 12, -1, 0, 0, 0}, 0.1f, 0.05f},
    {16, {9, 11, 13, 10, -1, 0, 0, 0}, 0.1f, 0.05f},
    {16, {14, 13, 12, 11, -1, 0, 0, 0}, 0.1f, 0.05f},
    {8, {10, 11, 12, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {8, {6, 8, 11, 13, -1, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsFrenziedOOV[] = {
    {40, {1, 16, 14, 12, 10, 11, -1, 0}, 0.1f, 0.05f},
    {40, {9, 11, 12, 13, 11, 7, -1, 0}, 0.1f, 0.05f},
    {10, {8, 10, 11, 12, 13, 12, -1, 0}, 0.1f, 0.05f},
    {10, {15, 13, 12, 10, 12, 9, -1, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsJumpingOOV[] = {
    {40, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {40, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {20, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsInjuredOOV[] = {
    {30, {9, 11, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {10, {13, 12, 10, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {9, 11, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {15, {14, 10, 12, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 15, 13, -1, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const SBurst CSpacePirate::skBurstsSeatedOOV[] = {
    {35, {7, 13, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {35, {9, 1, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {30, {16, 12, -1, 0, 0, 0, 0, 0}, 0.1f, 0.05f},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.f, 0.f},
};

const float CSpacePirate::skGravityConstant = 50.f;

static const float skRagDollParticleRadii[] = {0.45f, 0.52f, 0.35f, 0.1f,  0.15f, 0.35f, 0.1f,
                                               0.15f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f};

const SBurst* CSpacePirate::skBursts[] = {skBurstsQuick,
                                          skBurstsStandard,
                                          skBurstsFrenzied,
                                          skBurstsJumping,
                                          skBurstsInjured,
                                          skBurstsSeated,
                                          skBurstsQuickOOV,
                                          skBurstsStandardOOV,
                                          skBurstsFrenziedOOV,
                                          skBurstsJumpingOOV,
                                          skBurstsInjuredOOV,
                                          skBurstsSeatedOOV,
                                          nullptr};

rstl::list< TUniqueId > CSpacePirate::mChargePlayerList;

static rstl::string skOneEye = rstl::string_l("OneEye");
static rstl::string skTwoEyes = rstl::string_l("TwoEyes");

static CPatterned::StateMachine::STriggerFunction skTriggers[] = {
    {"Stuck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::Stuck)},
    {"PatternShagged",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::PatternShagged)},
    {"HearShot", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::HearShot)},
    {"HearPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::HearPlayer)},
    {"AggressionCheck",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::AggressionCheck)},
    {"CoverCheck", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::CoverCheck)},
    {"CoverFind", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::CoverFind)},
    {"CoverBlown", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::CoverBlown)},
    {"CoverNearlyBlown",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::CoverNearlyBlown)},
    {"CoveringFire",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::CoveringFire)},
    {"ShouldAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldAttack)},
    {"LineOfSight",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::LineOfSight)},
    {"PatternOver",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::PatternOver)},
    {"SpotPlayer", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::SpotPlayer)},
    {"ShouldDodge",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldDodge)},
    {"ShouldRetreat",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldRetreat)},
    {"InRange", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::InRange)},
    {"ShouldCrouch",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldCrouch)},
    {"ShouldMove", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldMove)},
    {"ShotAt", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShotAt)},
    {"Attacked", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::Attacked)},
    {"HasTargetingPoint",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::HasTargetingPoint)},
    {"ShouldWallHang",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldWallHang)},
    {"AnimOver", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::AnimOver)},
    {"ShouldStrafe",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldStrafe)},
    {"ShouldSpecialAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldSpecialAttack)},
    {"StartAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::StartAttack)},
    {"BreakAttack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::BreakAttack)},
    {"LostInterest",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::LostInterest)},
    {"BounceFind", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::BounceFind)},
    {"OffLine", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::OffLine)},
    {"Landed", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::Landed)},
    {"ShouldJumpBack",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldJumpBack)},
    {"Leash", static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::Leash)},
    {"HasAttackPattern",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::HasAttackPattern)},
    {"IsAmbushing",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::IsAmbushing)},
    {"ShouldWarpIn",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldWarpIn)},
    {"ShouldLaunchGrenade",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::ShouldLaunchGrenade)},
    {"InProjectileRange",
     static_cast< CPatterned::StateMachine::TriggerFunc >(&CSpacePirate::InProjectileRange)},
};

static CPatterned::StateMachine::SStateFunction skStates[] = {
    {"Ambushing", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Ambushing)},
    {"WarpIn", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::WarpIn)},
    {"WarpOut", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::WarpOut)},
    {"PostWarpOut", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::PostWarpOut)},
    {"Attack", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Attack)},
    {"Crouch", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Crouch)},
    {"CoverAttack", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::CoverAttack)},
    {"Halt", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Halt)},
    {"Run", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Run)},
    {"PathFind", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::PathFind)},
    {"Patrol", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Patrol)},
    {"TargetPatrol",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::TargetPatrol)},
    {"Shuffle", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Shuffle)},
    {"TurnAround", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::TurnAround)},
    {"Dodge", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Dodge)},
    {"Lurk", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Lurk)},
    {"Taunt", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Taunt)},
    {"Cover", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Cover)},
    {"Dead", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Dead)},
    {"TargetCover", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::TargetCover)},
    {"TargetPlayer",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::TargetPlayer)},
    {"Approach", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Approach)},
    {"WallHang", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::WallHang)},
    {"WallDetach", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::WallDetach)},
    {"GetUp", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::GetUp)},
    {"Generate", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Generate)},
    {"Skid", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Skid)},
    {"DoubleSnap", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::DoubleSnap)},
    {"JumpBack", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::JumpBack)},
    {"Bounce", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Bounce)},
    {"PathFindEx", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::PathFindEx)},
    {"Enraged", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Enraged)},
    {"Jump", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Jump)},
    {"Deactivate", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Deactivate)},
    {"LaunchGrenade",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::LaunchGrenade)},
    {"Captured", static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::Captured)},
    {"RemoveFromWorld",
     static_cast< CPatterned::StateMachine::StateFunc >(&CSpacePirate::RemoveFromWorld)},
};

CSpacePirate::CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& modelData,
                           const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                           const CSpacePirateData& data)
: CPatterned(kPAI_SpacePirate, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Ground,
             kCT_One, kBT_BiPedal, actorParams)
, mPirateData(data)
, mPendingAmbush((mPirateData.mFlags & 0x3) != 0)
, mCeilingAmbush(mPirateData.mFlags & 0x2)
, mNonAggressive(mPirateData.mFlags & 0x4)
, mMelee(mPirateData.mFlags & 0x8)
, mNoShuffleCloseCheck(mPirateData.mFlags & 0x10)
, mOnlyAttackInRange(mPirateData.mFlags & 0x20)
, x8f4_30_(mPirateData.mFlags & 0x40)
, mNoKnockbackImpulseReset(mPirateData.mFlags & 0x80)
, mNoMeleeAttack(mPirateData.mFlags & 0x200)
, mBreakAttack(mPirateData.mFlags & 0x400)
, mSeated(mPirateData.mFlags & 0x1000)
, mShadowPirate(mPirateData.mFlags & 0x2000)
, mAlertBeforeCloak(mPirateData.mFlags & 0x4000)
, mNoBreakDodge(mPirateData.mFlags & 0x8000)
, mFloatingCorpse(mPirateData.mFlags & 0x10000)
, mRagdollNoAiCollision(mPirateData.mFlags & 0x20000)
, mTrooper(mPirateData.mFlags & 0x40000)
, mHearNoise(false)
, mEnableMeleeAttack(false)
, x8f6_27_(false)
, x8f6_28_(false)
, mEnableRetreat(false)
, mShuffleClose(false)
, mEnablePatrol(false)
, mEnableAim(false)
, mHearPlayerFire(false)
, mInProjectilePath(false)
, mNoPlayerLos(false)
, mInWallHang(false)
, mJumpVelSet(false)
, mPrevInCineCam(false)
, mPendingFrenzyChance(false)
, mAppliedBladeDamage(false)
, mAlwaysAggressive(false)
, mCoverCheck(false)
, mEnableDodge(false)
, mNoPlayerDodge(false)
, mAllEnergyDrained(false)
, mMayStartAttack(false)
, x8f9_24_(false)
, mUseJumpBackJump(false)
, mStarted(false)
, mInRange(false)
, mSatUp(false)
, mCloseMelee(false)
, mSentAttackMsg(false)
, mNormalDodge(false)
, mGettingUp(false)
, mWarpInRequested(false)
, mDeleteAfterWarpOut(false)
, mWarpTimeCaptured(false)
, mInJump(false)
, mCannotShoot(false)
, mWallDetaching(false)
, mFrenzyFrames(0)
, mCoverPoint(kInvalidUniqueId)
, mPreviousCoverPoint(kInvalidUniqueId)
, mSteeringSpeed(1.f)
, mTargetDelta(CVector3f::Forward())
, mCoverPointRearDir(CVector3f::Zero())
, mPathFindSearch(nullptr, 1, patternedInfo.GetPathfindingIndex(), 1.f, 1.f, 0,
                  CPFRegion::kRP_Center)
, mSteeringDelayTimer(0.f)
, xa14_(0)
, mInitialHP(patternedInfo.GetHealthInfo().GetHP())
, mCoverRange(0.f)
, mHeadSeg(CSegId::Invalid())
, xa24_(0)
, mTaunt(pas::kTT_Invalid)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 70.f * (M_PIF / 180.f),
                CMath::Deg2Rad(180.f), kBTF_None)
, mCoverDir(pas::kCD_Invalid)
, mIntoJumpDist(1.f)
, mEyeHeight(2.f)
, mTimeLosClear(0.f)
, mTimeNoPlayerLos(0.f)
, mLosCheckTimer(0.f)
, mAttachedActor(kInvalidUniqueId)
, mGunSeg(CSegId::Invalid())
, mElbowSeg(CSegId::Invalid())
, mWristSeg(CSegId::Invalid())
, mSwooshSeg(CSegId::Invalid())
, mLeftHipSeg(CSegId::Invalid())
, mRightHipSeg(CSegId::Invalid())
, mCollarSeg(CSegId::Invalid())
, mAttackRemTime(1.f)
, mTargetId(kInvalidUniqueId)
, mBurstFire(skBursts, mPirateData.mFirstBurstCount)
, mJumpHeight(3.f)
, mPatrolDestPos(CVector3f::Zero())
, mSkidDir(pas::kSD_Invalid)
, mStrafeDelayTimer(0.f)
, mMeleeSeverity(pas::kS_Invalid)
, mJumpPoint(kInvalidUniqueId)
, mDodgeDir(pas::kSD_Invalid)
, mDodgeDist(3.f)
, mBreakDodgeDist(3.f)
, mTimeSinceHitByPlayer(FLT_MAX)
, mLowHealthFrenzyTimer(FLT_MAX)
, mRagdollDelayTimer(0.f)
, mRagDoll(nullptr)
, mIkChain()
, mCloakDelayTimer(0.f)
, mElectricParticleTimer(0.f)
, mCloakStepTime(0.f)
, mShadowPirateAlpha(0.5f)
, mMinCloakAlpha(mPirateData.mCloakOpacity)
, mMaxCloakAlpha(mPirateData.mMaxCloakOpacity)
, mDodgeDelayTimer(mPirateData.mDodgeDelayTimeMin)
, mAimDelayTimer(mPirateData.mGunTrackDelay)
, mAimReleaseTimer(0.f)
, mTeamAiMgrId(kInvalidUniqueId)
, mHeldPosition(CVector2f::Zero())
, mHoldPositionTime(0.f)
, mLeashTimer(0.f)
, mPlayerFirePos(CVector3f::Zero())
, mHearPlayerIndex(-1)
, mAttackTargetPos(CVector3f::Zero())
, mWarpTime(0.f)
, mProjectileInfo()
, mKnockBackSfx()
, mWarpPhase(-1)
, mGrenadeLauncherModel()
, mGrenadesToLaunch(0)
, mPortalPlane(0.f, CUnitVector3f(static_cast< const CVector3f& >(CVector3f::Forward()))) {
  SetupWeaponModel(data.mWeaponData);
  if (data.mProjectile != kInvalidAssetId) {
    mProjectileInfo = CProjectileInfo(data.mProjectile, data.mProjectileDamage);
    mProjectileInfo->Token().Lock();
  }
  mBurstFire.SetBurstType(1);
  mBoneTracking.SetDisableTrackingDistance(25.f * GetModelData()->GetScale().GetZ());
  const CAnimData* animData = GetAnimationData();
  mHeadSeg = animData->GetLocatorSegId(rstl::string_l("Head_1"));
  mElbowSeg = animData->GetLocatorSegId(rstl::string_l("R_elbow"));
  mWristSeg = animData->GetLocatorSegId(rstl::string_l("R_wrist"));
  mSwooshSeg = animData->GetLocatorSegId(rstl::string_l("Swoosh_LCTR"));
  mGunSeg = animData->GetLocatorSegId(rstl::string_l("gun_LCTR"));
  mLeftHipSeg = animData->GetLocatorSegId(rstl::string_l("L_hip"));
  mRightHipSeg = animData->GetLocatorSegId(rstl::string_l("R_hip"));
  mCollarSeg = animData->GetLocatorSegId(rstl::string_l("Collar"));

  if (!mOnlyAttackInRange) {
    const CPASAnimParmData jump(pas::kAS_Jump, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(0),
                                CPASAnimParm::FromEnum(0));
    mIntoJumpDist = GetModelData()->GetScale().GetY() * GetAnimationDistance(jump);
    const CPASAnimParmData dodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                 CPASAnimParm::FromEnum(1));
    mDodgeDist = GetModelData()->GetScale().GetX() * GetAnimationDistance(dodge);
    const CPASAnimParmData breakDodge(pas::kAS_Step, CPASAnimParm::FromEnum(3),
                                      CPASAnimParm::FromEnum(2));
    mBreakDodgeDist = GetModelData()->GetScale().GetX() * GetAnimationDistance(breakDodge);
  } else {
    BodyController()->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
  }

  const CAABox& baseBounds = GetBaseBoundingBox();
  mEyeHeight = (baseBounds.GetMaxPoint().GetZ() - baseBounds.GetMinPoint().GetZ()) * 0.6f;

  if (ActorLights()) {
    ActorLights()->SetAmbienceGenerated(false);
  }

  KnockBackController().SetLocomotionDuringElectrocution(true);

  if (!BodyController()->HasBodyState(pas::kAS_AdditiveAim)) {
    mMelee = true;
  }

  if (patternedInfo.GetEchoParameters().mIsEchoEmitter) {
    SetEchoEmitter(
        true, rs_new CPirateEchoEmitter(this, GetTranslation(), patternedInfo.GetEchoParameters(),
                                        animData->GetLocatorSegId(rstl::string_l("Head_1")),
                                        animData->GetLocatorSegId(rstl::string_l("R_wing_LCTR")),
                                        animData->GetLocatorSegId(rstl::string_l("L_wing_LCTR")),
                                        animData->GetLocatorSegId(rstl::string_l("gun_lctr")),
                                        animData->GetLocatorSegId(rstl::string_l("Swoosh_LCTR")),
                                        animData->GetLocatorSegId(rstl::string_l("R_ankle")),
                                        animData->GetLocatorSegId(rstl::string_l("L_ankle"))));
  }

  if (mFloatingCorpse) {
    SetHighlightedInDarkVisor(false);
  }
}

CSpacePirate::~CSpacePirate() {}

void CSpacePirate::SetupStateMachine(CStateManager& mgr) {
  StateMachine* stateMachine = mStateMachine.get();
  CPatterned::SetupStateMachine(mgr);
  stateMachine->SetTriggerFunctions(skTriggers, ARRAY_SIZE(skTriggers));
  stateMachine->SetStateFunctions(skStates, ARRAY_SIZE(skStates));
}

TUniqueId CSpacePirate::ChooseTarget(CStateManager& mgr) const {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsTeamMemberInRange(mgr, *this, 20.f)) {
        return team->FindBestIndividualAttackTarget(mgr, *this);
      }
    }
  }
  TUniqueId target = CScriptTeamAiMgr::ChoosePlayer(mgr, *this);
  return target;
}

TUniqueId CSpacePirate::ChooseTargetPlayer(CStateManager& mgr) const {
  float closestDist = 10.f;
  float currentDist = 0.f;
  TUniqueId closestId = kInvalidUniqueId;
  TUniqueId result = mTargetId;
  bool teamChoice = false;
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer* player = mgr.GetPlayer(i);
    if (player->GetUniqueId() == mTargetId) {
      if (!mgr.GetPlayerState(i)->IsPlayerAlive()) {
        teamChoice = true;
        result = ChooseTarget(mgr);
        break;
      }
      currentDist = (player->GetTranslation() - GetTranslation()).Magnitude();
    } else {
      const float dist = (player->GetTranslation() - GetTranslation()).Magnitude();
      if (dist < closestDist) {
        closestDist = dist;
        closestId = mgr.GetPlayer(i)->GetUniqueId();
      }
    }
  }
  if (currentDist > 30.f && closestDist < 10.f) {
    result = closestId;
  }
  if (!teamChoice && mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsTeamMemberInRange(mgr, *this, 50.f)) {
        return mTargetId;
      }
    }
  }
  return result;
}

void CSpacePirate::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId senderId = msg.GetSenderId();
  const EScriptObjectMessage message = msg.GetMessage();
  if (mInWallHang || mCeilingAmbush) {
    switch (message) {
    case kSM_OffGround:
      if ((mInWallHang && BodyController()->GetCurrentStateId() == pas::kAS_WallHang &&
           !BodyController()->GetBodyStateInfo().GetCurrentState()->ApplyGravity()) ||
          (mCeilingAmbush && (BodyController()->GetCurrentStateId() == pas::kAS_Locomotion ||
                              (BodyController()->GetCurrentStateId() == pas::kAS_Jump &&
                               !BodyController()->GetBodyStateInfo().IsInAir())))) {
        Stop();
        SetMomentumWR(CVector3f::Zero());
        return;
      }
      break;
    case kSM_Landed:
      mTimeSinceHitByPlayer = FLT_MAX;
      break;
    }
  }
  switch (message) {
  case kSM_Alert:
  case kSM_Activate:
    if (GetActive()) {
      if (mOnlyAttackInRange) {
        mMayStartAttack = true;
      } else {
        mHitByPlayerProjectile = true;
      }
    } else if (mCeilingAmbush) {
      RemoveMaterial(kMT_GroundCollider, mgr);
      mOnGround = false;
    }
    if (message == kSM_Activate && mTrooper) {
      if (!GetActive() && mWarpPhase == -1) {
        mColor.SetAlpha(0.f);
      }
      mWarpInRequested = true;
    }
    break;
  }
  CPatterned::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_AreaLoaded: {
    for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
      if (it->state == kSS_Retreat && it->msg == kSM_Next) {
        const TUniqueId id = mgr.GetIdForScript(it->objId);
        if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id))) {
          cover->Reserve(GetUniqueId());
        }
      } else if (it->state == kSS_Patrol && it->msg == kSM_Follow) {
        mEnablePatrol = true;
      }
    }
    mPathFindSearch.SetArea(
        mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetPostConstructed()->mPathArea);
    if (mFloatingCorpse) {
      mRagdollDelayTimer = 0.01f;
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      mAlive = false;
      HealthInfo()->SetHP(-1.f);
    } else {
      SetEyeParticleActive(mgr, true);
    }
    break;
  }
  case kSM_Decrement:
    if (mRagDoll.get() != nullptr) {
      mRagDoll->SetNoOverTimer(false);
      mRagDoll->SetContinueSmallMovements(false);
    }
    break;
  case kSM_Create: {
    if (mCeilingAmbush && mShadowPirate) {
      mColor.SetAlpha(mPirateData.mCloakOpacity);
      mAlphaDelta = -1.f;
    }
    xa24_ = mgr.Random()->Next() % 6;
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList passthrough(kMT_AIPassthrough);
    include.Remove(passthrough);
    exclude.Add(passthrough);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    break;
  }
  case kSM_SetToZero:
    if (GetActive()) {
      mEnableRetreat = true;
      RequestWarpOut(mgr, false);
    }
    break;
  case kSM_OffGround:
    if (BodyController()->GetPercentageFrozen() != 1.f) {
      float momentum = GetGravityConstant() * GetMass();
      if (mCeilingAmbush) {
        momentum *= 3.f;
      }
      SetMomentumWR(CVector3f(0.f, 0.f, -momentum));
    }
    if (BodyController()->GetCurrentStateId() == pas::kAS_Step) {
      SetVelocityWR(CVector3f(0.f, 0.f, GetVelocityWR().GetZ()));
    }
    mBurstFire.SetBurstType(3);
    break;
  case kSM_Launching:
    if (BodyController()->GetCurrentStateId() != pas::kAS_Hurled) {
      CPatterned::AcceptScriptMsg(mgr, CScriptMsg(senderId, GetUniqueId(), kSM_OffGround));
      SetMomentumWR(CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass()));
      SetVelocityForJump();
    }
    break;
  case kSM_Landed:
    if (!mOnlyAttackInRange) {
      mBurstFire.SetBurstType(1);
    } else {
      mBurstFire.SetBurstType(4);
    }
    mJumpVelSet = false;
    mInJump = false;
    if (mShadowPirate && GetVelocityWR().GetZ() < -1.f) {
      mAlphaDelta = 1.f;
      mCloakDelayTimer += -0.05f * GetVelocityWR().GetZ();
      mCloakDelayTimer = CMath::Clamp(0.f, mCloakDelayTimer, 1.f);
      mMaxCloakAlpha = 0.5f;
      if (GetAlive()) {
        mgr.ActorModelParticles()->StartElectric(*this);
        mElectricParticleTimer = 1.f + mCloakDelayTimer;
      }
    }
    break;
  case kSM_Action:
    if (CScriptTargetingPoint* point =
            TCastToPtr< CScriptTargetingPoint >(mgr.ObjectById(senderId))) {
      if (point->GetActive()) {
        mBoneTracking.SetTarget(senderId);
        mTargetId = senderId;
        SetTeamMemberTarget(mgr);
        mHitByPlayerProjectile = true;
      } else {
        mTargetId = ChooseTarget(mgr);
        SetTeamMemberTarget(mgr);
        mBoneTracking.SetTarget(mTargetId);
      }
      mAttackRemTime = 0.f;
    }
    break;
  case kSM_Deactivate:
  case kSM_Delete:
    SquadRemove(mgr);
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kSM_Start:
    mStarted = false;
    break;
  case kSM_Stop:
    mStarted = true;
    break;
  case kSM_Escape:
    if (GetActive()) {
      RequestWarpOut(mgr, true);
    }
    break;
  }
}

void CSpacePirate::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed()) {
    if (const CScriptTrigger* trigger = TCastToConstPtr< CScriptTrigger >(actor)) {
      if (trigger->GetActive() && (trigger->GetTriggerFlags() & kTFL_DetectAI) &&
          trigger->GetForceMagnitude() > 0.f) {
        mRagDoll->TorsoImpulse() += trigger->GetForceField();
      }
    }
  }
}

bool CSpacePirate::Listen(CStateManager& mgr, const CVector3f& position, EListenNoiseType type) {
  bool heard = false;
  if (GetAlive()) {
    const CVector3f delta = position - GetTranslation();
    const float hearingRadius = mPirateData.mHearingRadius * mPirateData.mHearingRadius;
    if (delta.MagSquared() < hearingRadius &&
        (mDetectionHeightRange == 0.f ||
         delta.GetZ() * delta.GetZ() < mDetectionHeightRange * mDetectionHeightRange)) {
      mHearNoise = true;
      heard = true;
    }
    if (type == kLNT_PlayerFire) {
      mHearPlayerFire = true;
      mPlayerFirePos = position;
    }
  }
  const bool result = heard;
  return result;
}

void CSpacePirate::SetEyeParticleActive(CStateManager& mgr, bool active) {}

bool CSpacePirate::CheckTargetable(CStateManager& mgr) { return GetModelAlphau8(mgr) > 127; }

void CSpacePirate::SetVelocityForJump() {
  if (!mJumpVelSet && !mWallDetaching) {
    CVector3f velocity = CVector3f::Zero();
    const CVector3f delta = mPatrolDestPos - GetTranslation();
    const float gravity = GetGravityConstant();
    const float jumpZ = mJumpHeight + CMath::Max(mPatrolDestPos.GetZ(), GetTranslation().GetZ());
    velocity.SetZ(CMath::SqrtF(2.f * gravity * (jumpZ - GetTranslation().GetZ())));
    float time = velocity.GetZ() / gravity;
    time += CMath::SqrtF(2.f * (jumpZ - mPatrolDestPos.GetZ()) / gravity);
    const float invTime = 1.f / time;
    velocity.SetX(invTime * delta.GetX());
    velocity.SetY(invTime * delta.GetY());
    SetVelocityWR(velocity);
    mJumpVelSet = true;
    mInJump = true;
  }
}

void CSpacePirate::SetAttackTarget(CStateManager& mgr, TUniqueId target) {
  mTargetId = target;
  SetTeamMemberTarget(mgr);
  mBurstFire.SetBurstType(1);
  mAttackRemTime = 0.f;
}

bool CSpacePirate::AttachActorToPirate(TUniqueId id) {
  if (mAttachedActor == kInvalidUniqueId) {
    if (!mRagDoll.null()) {
      mRagDoll->SetActorAttached(true);
    }
    mAttachedActor = id;
    return true;
  }
  return false;
}

void CSpacePirate::DetachActorFromPirate() {
  mAttachedActor = kInvalidUniqueId;
  if (!mRagDoll.null()) {
    mRagDoll->SetActorAttached(false);
  }
}

CVector3f CSpacePirate::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                  const CVector3f& aimPos) const {
  return GetTranslation();
}

void CSpacePirate::JoinTeam(CStateManager& mgr) {
  if (mTeamAiMgrId == kInvalidUniqueId) {
    mTeamAiMgrId = CScriptTeamAiMgr::GetAssociatedTeamId(*this, mgr);
  }
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->JoinTeam(*this, mMelee ? CTeamAiRole::kTAR_Melee : CTeamAiRole::kTAR_Projectile,
                     CTeamAiRole::kTAR_Unknown, CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CSpacePirate::SquadRemove(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->QuitTeam(GetUniqueId());
        mTeamAiMgrId = kInvalidUniqueId;
      }
    }
  }
}

void CSpacePirate::SquadReset(CStateManager& mgr) {
  CScriptTeamAiMgr::EndAttack(mMelee ? CScriptTeamAiMgr::kAT_Melee
                                     : CScriptTeamAiMgr::kAT_Projectile,
                              mgr, mTeamAiMgrId, GetUniqueId(), true);
}

void CSpacePirate::SetTeamMemberTarget(CStateManager& mgr) {
  if (mTeamAiMgrId != kInvalidUniqueId) {
    if (CScriptTeamAiMgr* team = TCastToPtr< CScriptTeamAiMgr >(mgr.ObjectById(mTeamAiMgrId))) {
      team->SetMemberTargetId(GetUniqueId(), mTargetId);
    }
  }
}

void CSpacePirate::CheckForProjectiles(CStateManager& mgr) {
  if (mHearPlayerFire) {
    CVector3f extent(5.f, 5.f, 5.f);
    CAABox bounds(mPlayerFirePos - extent, mPlayerFirePos + extent);
    mInProjectilePath = false;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      const CGameProjectile* projectile =
          TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(nearList[i]));
      if (projectile) {
        CVector3f delta = GetBoundingBox().GetCenterPoint() - projectile->GetTranslation();
        if (delta.IsMagnitudeSafe()) {
          if (CVector3f::Dot(GetTransform().GetForward(), delta) < 0.f) {
            delta.Normalize();
            CVector3f projDelta = projectile->GetTranslation() - projectile->GetPreviousPos();
            if (projDelta.IsMagnitudeSafe()) {
              projDelta.Normalize();
              if (CVector3f::Dot(projDelta, delta) > 0.939f) {
                mInProjectilePath = true;
              }
            }
          }
        } else {
          mInProjectilePath = true;
        }
        if (mInProjectilePath) {
          break;
        }
      }
    }
    mHearPlayerFire = false;
  }
}

bool CSpacePirate::LineOfSightTest(CStateManager& mgr, const CVector3f& eyePos,
                                   const CVector3f& targetPos, const CMaterialList& excludeList) {
  CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), excludeList);
  return mgr.RayCollideWorld(eyePos, targetPos, filter, this);
}

void CSpacePirate::UpdateCantSeePlayer(CStateManager& mgr, float dt) {
  mLosCheckTimer += dt;
  mTimeLosClear += dt;
  if (mLosCheckTimer > 0.1f) {
    mLosCheckTimer = 0.f;
    CVector3f eyePos = GetTranslation() + CVector3f(0.f, 0.f, mEyeHeight);
    CPlayer* player = TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)));
    if (!player) {
      if (!mgr.IsMultiplayer()) {
        player = mgr.GetPlayer(0);
      }
    }
    if (player) {
      CVector3f aimPos = player->GetAimPosition(mgr, 0.f);
      if (GetCoverPoint(mgr, mCoverPoint)) {
        switch (mCoverDir) {
        case pas::kCD_Left:
          eyePos -= 2.f * GetTransform().GetRight();
          break;
        case pas::kCD_Right:
          eyePos += 2.f * GetTransform().GetRight();
          break;
        default:
          break;
        }
      } else {
        CVector3f toPlayer = (aimPos - eyePos).AsNormalized();
        eyePos += 1.1f * CVector3f::Cross(toPlayer, CVector3f::Up());
      }
      if (!LineOfSightTest(mgr, eyePos, aimPos,
                           CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
        mTimeLosClear = 0.f;
      }
    }
  }
  mNoPlayerLos = mTimeLosClear < mPirateData.mMinLosClearTime;
}

void CSpacePirate::UpdateHeldPosition(CStateManager& mgr, float dt) {
  if (CPlayer* player =
          TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)))) {
    CVector2f pos = player->GetTranslation().ToVec2f();
    if ((pos - mHeldPosition).MagSquared() < 3.f) {
      mHoldPositionTime += dt;
    } else {
      mHeldPosition = pos;
      mHoldPositionTime = 0.f;
    }
  } else {
    mHoldPositionTime = 0.f;
  }
}

static CVector3f Random2f(CStateManager& mgr, float min, float max) {
  CVector3f result(mgr.Random()->Float() - 0.5f, mgr.Random()->Float() - 0.5f, 0.f);
  if (CMath::AbsF(result.GetX()) < 0.001f) {
    result.SetX(0.001f);
  }
  result.Normalize();
  result *= (max - min) * mgr.Random()->Float() + min;
  return result;
}

void CSpacePirate::AvoidActors(CStateManager& mgr) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* ai = TCastToConstPtr< CPatterned >(list[i])) {
      if (ai != this && ai->GetCurrentAreaId() == GetCurrentAreaId()) {
        CVector3f separation =
            mSteeringBehaviors.Separation(*this, ai->GetTranslation(), mPirateData.mAvoidDistance);
        if (separation.IsMagnitudeSafe()) {
          BodyController()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(separation.AsNormalized(), CVector3f::Zero(), 0.5f));
          if (!mSteeringDelayTimer) {
            if (CSpacePirate* pirate = TCastToPtr< CSpacePirate >(const_cast< CPatterned* >(ai))) {
              if (!pirate->mSteeringDelayTimer) {
                CVector3f delta = pirate->GetTranslation() - GetTranslation();
                if (CVector3f::Dot(GetTransform().GetForward(), delta) > 0.f &&
                    CVector3f::Dot(pirate->GetVelocityWR(), pirate->GetTransform().GetForward()) >
                        0.f) {
                  mSteeringDelayTimer = 1.f;
                }
              }
            }
          }
        }
      }
    }
  }
}

bool CSpacePirate::IsPathClear(CStateManager& mgr, const CVector3f& dir, float dist) {
  CVector3f center = GetBoundingBox().GetCenterPoint();
  bool clear = false;
  if (mPathFindSearch.OnPath(center + dist * dir) == CPathFindSearch::kR_Success) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    mgr.BuildNearList(nearList, center, dir, dist, filter, this);
    if (CGameCollision::RayDynamicLineOfSightTest(mgr, center, dir, dist, filter, nearList,
                                                  nullptr)) {
      clear = true;
    }
  }
  const bool result = clear;
  return result;
}

pas::EStepDirection CSpacePirate::GetStrafeDir(CStateManager& mgr, float dist) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  float distSq = dist * dist;
  bool left = true;
  bool right = true;
  pas::EStepDirection result = pas::kSD_Invalid;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CSpacePirate* pirate = TCastToPtr< CSpacePirate >(const_cast< CEntity* >(list[i]))) {
      if (pirate != this && pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
        CVector3f delta = pirate->GetTranslation() - GetTranslation();
        float deltaSq = delta.MagSquared();
        if (deltaSq < distSq) {
          float dot = CVector3f::Dot(delta, GetTransform().GetRight());
          if (dot > 0.866f * deltaSq || (dot > 0.f && deltaSq < 3.f)) {
            right = false;
          } else if (dot < -deltaSq * 0.866f || (dot < 0.f && deltaSq < 3.f)) {
            left = false;
          }
        }
      }
    }
  }
  CVector3f center = GetBoundingBox().GetCenterPoint();
  CVector3f rightVec = GetTransform().GetRight();
  if (right) {
    right = mPathFindSearch.OnPath(center + dist * rightVec) == CPathFindSearch::kR_Success;
  }
  if (left) {
    left = mPathFindSearch.OnPath(center - dist * rightVec) == CPathFindSearch::kR_Success;
  }
  if (left || right) {
    CVector3f start = left ? center : center - dist * rightVec;
    CVector3f end = right ? center : center + dist * rightVec;
    float length = (left && right) ? 2.f * dist : dist;
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    mgr.BuildNearList(nearList, start, rightVec, length, filter, this);
    if (left) {
      left = CGameCollision::RayDynamicLineOfSightTest(mgr, start, rightVec, dist, filter, nearList,
                                                       nullptr);
    }
    if (right) {
      right = CGameCollision::RayDynamicLineOfSightTest(mgr, center, rightVec, dist, filter,
                                                        nearList, nullptr);
    }
    if (left && right) {
      if ((mgr.Random()->Next() & 0x4000) != 0) {
        left = false;
      } else {
        right = false;
      }
    }
    if (left) {
      result = pas::kSD_Left;
    } else if (right) {
      result = pas::kSD_Right;
    }
  }
  return result;
}

void CSpacePirate::CheckBlade(CStateManager& mgr) {
  if (!mAppliedBladeDamage && mSwooshSeg != CSegId::Invalid()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mgr.ObjectById(mTargetId))) {
      CTransform4f swoosh = GetLctrTransform(mSwooshSeg);
      const CVector3f& scale = GetModelData()->GetScale();
      CVector3f extent = 0.5f * scale;
      CAABox bounds(swoosh.GetTranslation() - extent, swoosh.GetTranslation() + extent);
      if (bounds.DoBoundsOverlap(actor->GetBoundingBox())) {
        mgr.ApplyDamage(
            GetUniqueId(), actor->GetUniqueId(), GetUniqueId(), mPirateData.mBladeDamage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mAppliedBladeDamage = true;
      }
    }
  }
}

CVector3f CSpacePirate::GetTargetPos(CStateManager& mgr) {
  const CEntity* entity = mgr.GetObjectById(mTargetId);
  const CPlayer* player = TCastToConstPtr< CPlayer >(entity);
  if (!player) {
    const CActor* actor = static_cast< const CActor* >(entity);
    if (actor && actor->GetActive()) {
      return actor->GetTranslation();
    }
    mTargetId = ChooseTarget(mgr);
    SetTeamMemberTarget(mgr);
    mBoneTracking.SetTarget(mTargetId);
    TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
  } else {
    return player->GetTranslation();
  }
  return GetTranslation() + 10.f * GetTransform().GetForward();
}

void CSpacePirate::SetCinematicCollision(CStateManager& mgr) {
  RemoveMaterial(kMT_AIBlock, mgr);
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  include.Remove(kMT_AIBlock);
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(include, GetMaterialFilter().GetExcludeList()));
}

void CSpacePirate::SetNonCinematicCollision(CStateManager& mgr) {
  AddMaterial(kMT_AIBlock, mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_AIBlock)),
      GetMaterialFilter().GetExcludeList()));
}

void CSpacePirate::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if ((!mCeilingAmbush || !GetAlive()) && mRagDoll.get() == nullptr &&
      mAttachedActor == kInvalidUniqueId) {
    KnockBackController().EnableKnockBackPhysics(!mNoKnockbackImpulseReset);
    KnockBackController().EnableAnimReaction(CKnockBackMgr::kAR_KnockBack, IsOnGround());
    bool enableFreeze = true;
    if (IsIngPossessed() || (mShadowPirate && !info.GetDamageInfo().GetWeaponMode().IsCharged() &&
                             !info.GetDamageInfo().GetWeaponMode().IsComboed())) {
      enableFreeze = false;
    }
    KnockBackController().EnableFreeze(enableFreeze);
    CPatterned::KnockBack(mgr, info);
    if (mShadowPirate) {
      if (GetAlive()) {
        if (info.GetDamageInfo().GetKnockBackPower(*GetDamageVulnerability(), 0.f) >= 4.f &&
            BodyController()->GetPercentageFrozen() != 1.f) {
          mAlphaDelta = 1.f;
          mCloakDelayTimer +=
              0.1f * info.GetDamageInfo().GetKnockBackPower(*GetDamageVulnerability(), 0.f);
          mCloakDelayTimer = CMath::Clamp(0.f, mCloakDelayTimer, 1.f);
          mMaxCloakAlpha = 0.5f;
          mgr.ActorModelParticles()->StartElectric(*this);
          mElectricParticleTimer = mCloakDelayTimer + 1.f;
        }
      } else {
        mAlphaDelta = 1.f;
        mMaxCloakAlpha = 1.f;
        mMinCloakAlpha = 0.f;
        mgr.ActorModelParticles()->StartElectric(*this);
        mElectricParticleTimer = 2.f;
      }
    }
    if (GetAlive()) {
      switch (GetKnockBackController().GetActiveReaction()) {
      case CKnockBackMgr::kAR_Hurled:
        mStateMachine->SetState(mgr, *this, rstl::string_l("GetUpNow"));
        mKnockBackSfx = CSfxManager::AddEmitter(mPirateData.mSound_Hurled, GetTranslation(), 127,
                                                GetCurrentAreaId().Value(), false, false,
                                                CSfxManager::kMedPriority);
        break;
      }
    } else if (!mFloatingCorpse) {
      switch (GetKnockBackController().GetActiveReaction()) {
      case CKnockBackMgr::kAR_Hurled:
        if (GetKnockBackController().GetFollowUp() != CKnockBackMgr::kFU_LaggedBurnDeath &&
            GetKnockBackController().GetFollowUp() != CKnockBackMgr::kFU_BurnDeath &&
            GetKnockBackController().GetFollowUp() != CKnockBackMgr::kFU_ExplodeDeath &&
            GetKnockBackController().GetFollowUp() != CKnockBackMgr::kFU_IceDeath) {
          mKnockBackSfx = CSfxManager::AddEmitter(mPirateData.mSound_Death, GetTranslation(), 127,
                                                  GetCurrentAreaId().Value(), false, false,
                                                  CSfxManager::kMedPriority);
        }
        break;
      }
    }
  }
}

void CSpacePirate::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (GetAlive()) {
    CPatterned::Death(mgr, direction, state);
    if (mAttachedActor != kInvalidUniqueId) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCKnockDownCmd(GetTransform().GetForward(), pas::kS_Two));
    }
  }
}

bool CSpacePirate::Stuck(CStateManager& mgr, const CTriggerData& data) const {
  if (mStateMachine->GetTime() > 0.5f) {
    return CPatterned::Stuck(mgr, data) || CPatterned::PathShagged(mgr, data);
  }
  return false;
}

bool CSpacePirate::PatternShagged(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::Stuck(mgr, data);
}

void CSpacePirate::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mEnableAim = true;
    BodyController()->AbortScriptedAnimations();
    JoinTeam(mgr);
    if (!mSentAttackMsg) {
      mSentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    if (mCeilingAmbush) {
      mPatrolDestPos = GetTranslation() + CVector3f::Down();
      mJumpHeight = 0.f;
    } else {
      const TUniqueId id = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
      if (const CActor* actor =
              TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(id)))) {
        mPatrolDestPos = actor->GetTranslation();
        mJumpHeight = 3.f;
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
      BodyController()->SetLocomotionType(pas::kLT_Combat);
    }
    break;
  case kStateMsg_Update: {
    pas::EJumpType jumpType = pas::kJT_Normal;
    if (mCeilingAmbush) {
      jumpType = pas::kJT_Ambush;
    }
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mPatrolDestPos, jumpType, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    if (mAnimationState.GetState() == CAnimationState::kAS_Repeat) {
      BodyController()->SetLocomotionType(pas::kLT_Combat);
    }
    CVector3f target = GetTargetPos(mgr) - GetTranslation();
    target.SetZ(0.f);
    BodyController()->CommandMgr().SetTargetVector(target);
    break;
  }
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mJumpHeight = 3.f;
    mCeilingAmbush = false;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mTargetId);
    break;
  }
}

void CSpacePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Relaxed);
    mSteeringSpeed = BodyController()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) /
                     BodyController()->GetBodyStateInfo().GetLocomotionSpeed(pas::kLA_Run);
    break;
  case kStateMsg_Update:
    UpdateCantSeePlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    if (!mSentAttackMsg) {
      mSentAttackMsg = true;
      SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    }
    break;
  }
  if (mEnablePatrol) {
    CPatterned::Patrol(mgr, msg, dt);
    switch (msg) {
    case kStateMsg_Activate:
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyController()->SetTurnSpeed(BodyController()->GetTurnSpeed() / 1.25f);
      break;
    case kStateMsg_Update:
      AvoidActors(mgr);
      mPatrolDestPos = mWaypointNavigation.GetDestinationPosition();
      break;
    case kStateMsg_Deactivate:
      BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
      BodyController()->SetTurnSpeed(BodyController()->GetTurnSpeed() * 1.25f);
      break;
    }
  }
}

void CSpacePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Patrol(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate: {
    mSteeringSpeed = 1.f;
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    const TUniqueId id = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    if (const CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
      mWaypointNavigation.SetDestination(id);
      const CVector3f forward = GetTransform().GetForward();
      const CVector3f toWaypoint = waypoint->GetTranslation() - GetTranslation();
      if (CVector3f::Dot(forward, toWaypoint) <= 0.f) {
        mWaypointNavigation.SetInPosition(true);
      }
    }
    break;
  }
  case kStateMsg_Update: {
    const CScriptAIWaypoint* aiWaypoint =
        TCastToPtr< CScriptAIWaypoint >(mgr.ObjectById(mWaypointNavigation.GetDestination()));
    if (aiWaypoint) {
      const uint jump = (aiWaypoint->GetFlags() >> 1) & 1;
      const uint drop = (aiWaypoint->GetFlags() >> 2) & 1;
      if (jump || drop) {
        const float maxSpeed = BodyController()->GetBodyStateInfo().GetMaxSpeed();
        const float distance =
            maxSpeed * ((1.5f * dt + 0.1f) * GetModelData()->GetScale().GetY()) + mIntoJumpDist;
        if ((GetTranslation() - aiWaypoint->GetTranslation()).MagSquared() < distance * distance) {
          mWaypointNavigation.SetInPosition(true);
          mJumpHeight = jump ? 3.f : 0.f;
        }
      }
    }
    if (BodyController()->GetCurrentStateId() == pas::kAS_Jump) {
      bool targetPlayer = true;
      if (aiWaypoint) {
        if (aiWaypoint->CheckConnectedObject(mgr, kSS_Arrived, kSM_Next) != kInvalidUniqueId) {
          targetPlayer = false;
        }
      }
      if (targetPlayer) {
        BodyController()->CommandMgr().SetTargetVector(GetTargetPos(mgr) - GetTranslation());
      }
    }
    mPatrolDestPos = mWaypointNavigation.GetDestinationPosition();
    break;
  }
  case kStateMsg_Deactivate:
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

bool CSpacePirate::PatternOver(CStateManager& mgr, const CTriggerData& data) const {
  return mWaypointNavigation.GetDestination() == kInvalidUniqueId;
}

bool CSpacePirate::HearShot(CStateManager& mgr, const CTriggerData& data) const {
  const bool heard = mHearNoise;
  mHearNoise = false;
  return heard;
}

bool CSpacePirate::HearPlayer(CStateManager& mgr, const CTriggerData& data) const {
  bool heard = false;
  mHearPlayerIndex = (mHearPlayerIndex + 1) % mgr.GetNumPlayers();
  if (mHearPlayerIndex != -1) {
    const CPlayer* player = mgr.GetPlayer(mHearPlayerIndex);
    if (player->GetVelocityWR().MagSquared() > 0.1f) {
      const CVector3f delta = player->GetTranslation() - GetTranslation();
      if (delta.MagSquared() < mPirateData.mHearingRadius * mPirateData.mHearingRadius) {
        heard = true;
      }
    }
  }
  return heard;
}

void CSpacePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt) { mSteeringSpeed = 0.f; }

void CSpacePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) { mSteeringSpeed = 1.f; }

void CSpacePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mEnableAim = true;
    BodyController()->AbortScriptedAnimations();
    JoinTeam(mgr);
    if (mTargetId == kInvalidUniqueId) {
      mTargetId = ChooseTarget(mgr);
      SetTeamMemberTarget(mgr);
    }
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mTargetId);
    if (BodyController()->HasBodyState(pas::kAS_Taunt)) {
      if (!mShadowPirate) {
        bool findOtherPirate = true;
        if (mMelee) {
          const CPASAnimParmData parms(pas::kAS_Taunt, CPASAnimParm::FromEnum(2));
          const CPASDatabase& database = BodyController()->GetPASDatabase();
          const rstl::pair< float, int > anim =
              database.FindBestAnimation(parms, *mgr.Random(), -1);
          if (anim.first > 0.f) {
            findOtherPirate = false;
            mTaunt = pas::kTT_Two;
          }
        }
        if (findOtherPirate) {
          bool withOtherPirate = false;
          const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
          for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
            if (const CSpacePirate* pirate = TCastToConstPtr< CSpacePirate >(list[i])) {
              if (pirate != this && !pirate->mEnableAim && pirate->GetAlive() &&
                  pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                if ((pirate->GetTranslation() - GetTranslation()).MagSquared() <
                    mPirateData.mHearingRadius * mPirateData.mHearingRadius) {
                  withOtherPirate = true;
                }
              }
            }
          }
          mTaunt = withOtherPirate ? pas::kTT_Zero : pas::kTT_One;
        }
      } else {
        mTaunt = mAlertBeforeCloak ? pas::kTT_One : pas::kTT_Zero;
      }
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    } else {
      CSfxManager::AddEmitter(mPirateData.mSound_Alert, GetTranslation(),
                              GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Taunt)) {
      BodyController()->CommandMgr().DeliverCmd(CBCTauntCmd(mTaunt));
    }
    break;
  case kStateMsg_Deactivate:
    if (mTaunt == pas::kTT_Zero) {
      mgr.InformListeners(GetTranslation(), kLNT_PlayerFire);
    }
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSpacePirate::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    SquadReset(mgr);
    mLeashTimer = 0.f;
    mGettingUp = true;
    break;
  case kStateMsg_Update:
    if (BodyController()->GetCurrentStateId() == pas::kAS_LieOnGround &&
        mPathFindSearch.Search(GetTranslation(), GetTranslation()) ==
            CPathFindSearch::kR_NoSourcePoint) {
      mPendingDeath = true;
    } else if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Getup)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
    }
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mGettingUp = false;
    break;
  }
}

void CSpacePirate::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    ReleaseCoverPoint(mgr, mCoverPoint, true);
    mSteeringSpeed = 0.f;
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mNoPlayerLos = true;
    mTimeNoPlayerLos = 0.f;
    const float aggression = mPirateData.mAggressionCheck;
    mAlwaysAggressive = mgr.Random()->Range(0.f, 100.f) < aggression;
    const float cover = mPirateData.mCoverCheck;
    mCoverCheck = mgr.Random()->Range(0.f, 100.f) < cover;
    const float dodge = mPirateData.mDodgeCheck;
    mEnableDodge = mgr.Random()->Range(0.f, 100.f) < dodge;
    mEnableAim = true;
    BodyController()->AbortScriptedAnimations();
    if (mTargetId == kInvalidUniqueId) {
      mTargetId = ChooseTarget(mgr);
      SetTeamMemberTarget(mgr);
      mBoneTracking.SetActive(true);
      mBoneTracking.SetTarget(mTargetId);
    }
    if (mOnlyAttackInRange) {
      mBurstFire.SetBurstType(4);
      BodyController()->SetLocomotionType(pas::kLT_Combat);
    }
    mNormalDodge = false;
    break;
  }
  case kStateMsg_Update:
    if (BodyController()->HasBodyState(pas::kAS_Turn)) {
      if (mAnimationState.GetState() != CAnimationState::kAS_NotReady &&
          mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Turn)) {
        CVector3f delta = mAttackTargetPos - GetTranslation();
        if (delta.IsMagnitudeSafe()) {
          BodyController()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
        }
      }
      if (mAnimationState.GetState() != CAnimationState::kAS_Repeat) {
        mAttackTargetPos = GetTargetPos(mgr);
        CVector3f delta = mAttackTargetPos - GetTranslation();
        delta.SetZ(0.f);
        if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.9f) {
          mAnimationState.SetState(CAnimationState::kAS_Ready);
        }
      }
    }
    if (mSeated && mSatUp) {
      if (mAttackRemTime > GetAverageAttackTime() &&
          BodyController()->GetLocomotionType() == pas::kLT_Combat) {
        BodyController()->SetLocomotionType(pas::kLT_Internal5);
      } else if (mAttackRemTime < 0.5f * GetAverageAttackTime() &&
                 BodyController()->GetLocomotionType() == pas::kLT_Internal5) {
        BodyController()->SetLocomotionType(pas::kLT_Combat);
      }
    }
    UpdateCantSeePlayer(mgr, dt);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mAlwaysAggressive = false;
    mNoPlayerDodge = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CSpacePirate::AggressionCheck(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (!mNonAggressive) {
    if (mAlwaysAggressive) {
      result = true;
    } else if (mChargePlayerList.empty() && mTimeNoPlayerLos > 10.f) {
      result = true;
    }
    if (result) {
      if (rstl::find< rstl::list< TUniqueId >::const_iterator, TUniqueId >(
              mChargePlayerList.begin(), mChargePlayerList.end(), GetUniqueId()) ==
          mChargePlayerList.end()) {
        mChargePlayerList.push_back(GetUniqueId());
      }
    }
  }
  return result;
}

bool CSpacePirate::CoverCheck(CStateManager& mgr, const CTriggerData& data) const {
  return mCoverCheck;
}

bool CSpacePirate::CoverFind(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  float minDistSq = mPirateData.mSearchRadius * mPirateData.mSearchRadius;
  const CScriptCoverPoint* closest = nullptr;
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CScriptCoverPoint* coverPoint = TCastToConstPtr< CScriptCoverPoint >(list[i])) {
      if (coverPoint->GetActive() && !coverPoint->ShouldLandHere() &&
          !coverPoint->GetInUse(GetUniqueId()) &&
          coverPoint->GetCurrentAreaId() == GetCurrentAreaId() &&
          coverPoint->GetUniqueId() != mPreviousCoverPoint) {
        const float distSq = (GetTranslation() - coverPoint->GetTranslation()).MagSquared();
        if (distSq < minDistSq &&
            !coverPoint->Blown(const_cast< CSpacePirate* >(this)->GetTargetPos(mgr))) {
          minDistSq = distSq;
          closest = coverPoint;
        }
      }
    }
  }
  if (closest) {
    const_cast< CSpacePirate* >(this)->ReleaseCoverPoint(mgr, mCoverPoint, true);
    if (CScriptCoverPoint* coverPoint =
            TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(closest->GetUniqueId()))) {
      const_cast< CSpacePirate* >(this)->SetCoverPoint(coverPoint, mCoverPoint);
      result = true;
      mPreviousCoverPoint = mCoverPoint;
      mCoverPointRearDir = -closest->GetTransform().GetForward();
    }
  }
  return result;
}

bool CSpacePirate::CoverBlown(CStateManager& mgr, const CTriggerData& data) const {
  bool result = true;
  CVector3f target = const_cast< CSpacePirate* >(this)->GetTargetPos(mgr);
  CVector3f toTarget = target - GetTranslation();
  if (toTarget.MagSquared() > mMinAttackRange * mMinAttackRange) {
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      result = coverPoint->Blown(target);
      if (!result && mSteeringSpeed == 0.f &&
          GetBodyController()->GetCurrentStateId() != pas::kAS_Step) {
        CVector3f toCover = coverPoint->GetTranslation() - GetTranslation();
        if (toCover.MagSquared() > 3.f * GetModelData()->GetScale().GetY()) {
          result = true;
        }
      }
    }
  }
  return result;
}

bool CSpacePirate::CoverNearlyBlown(CStateManager& mgr, const CTriggerData& data) const {
  bool result = true;
  if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
    result = false;
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId))) {
      CVector3f pos = player->GetTranslation() + 1.f * player->GetVelocityWR();
      result = coverPoint->Blown(pos);
    }
  }
  return result;
}

bool CSpacePirate::CoveringFire(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CSpacePirate* pirate = TCastToConstPtr< CSpacePirate >(list[i])) {
      if (pirate != this && pirate->mInAttackState &&
          pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
        result = true;
      }
    }
  }
  return result;
}

bool CSpacePirate::ShouldAttack(CStateManager& mgr, const CTriggerData& data) const {
  bool result = true;
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId))) {
    CVector3f target = const_cast< CSpacePirate* >(this)->GetTargetPos(mgr);
    int numCloserPirates = 0;
    float distSq = (GetTranslation() - target).MagSquared();
    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      if (const CSpacePirate* pirate = TCastToConstPtr< CSpacePirate >(list[i])) {
        if (pirate != this && pirate->mInAttackState && pirate->mAlive &&
            pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
          if ((pirate->GetTranslation() - target).MagSquared() < distSq) {
            ++numCloserPirates;
            if (numCloserPirates > 3) {
              result = false;
            }
          }
        }
      }
    }
  }
  return result;
}

void CSpacePirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (GetBodyController()->GetCurrentStateId() != pas::kAS_Cover) {
      if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
        const uint attackDir = static_cast< uint >(coverPoint->GetAttackDirection());
        mCoverDir = static_cast< pas::ECoverDirection >((attackDir >> 1) & 1);
        mAnimationState.SetState(CAnimationState::kAS_Ready);
        mDestPos = coverPoint->GetTranslation();
        if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Cover)) {
          CBCCoverCmd cmd(mCoverDir, coverPoint->GetTranslation(),
                          -coverPoint->GetTransform().GetForward());
          BodyController()->CommandMgr().DeliverCmd(cmd);
        }
      }
    }
    break;
  case kStateMsg_Update:
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Cover)) {
        CBCCoverCmd cmd(mCoverDir, coverPoint->GetTranslation(),
                        -coverPoint->GetTransform().GetForward());
        BodyController()->CommandMgr().DeliverCmd(cmd);
      }
      BodyController()->CommandMgr().SetTargetVector(-coverPoint->GetTransform().GetForward());
    }
    UpdateCantSeePlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSpacePirate::CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_LeanFromCover));
    mInAttackState = true;
    break;
  case kStateMsg_Update:
    UpdateCantSeePlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mInAttackState = false;
    break;
  }
}

void CSpacePirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void CSpacePirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->SetLocomotionType(pas::kLT_Combat);
    mAttackTargetPos = GetTargetPos(mgr);
    mTargetDelta = mAttackTargetPos - GetBoundingBox().GetCenterPoint();
    mSteeringSpeed = 0.f;
    mEnableMeleeAttack = false;
    if (!mNoMeleeAttack && TooClose(mgr, CTriggerData(0.f))) {
      mEnableMeleeAttack = true;
      mAppliedBladeDamage = false;
    }
    if (CVector3f::Dot(GetTransform().GetForward(), mTargetDelta.AsNormalized()) < 0.8f) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), mTargetDelta, 1.f));
    }
    mInAttackState = true;
    mMaxCloakAlpha = 0.75f;
    break;
  case kStateMsg_Update:
    if (mEnableMeleeAttack) {
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
        BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
      }
      BodyController()->CommandMgr().SetTargetVector(mTargetDelta);
      CheckBlade(mgr);
      if (mShadowPirate) {
        if (mAnimationState.IsOver()) {
          mAlphaDelta = -0.4f;
        } else {
          mAlphaDelta = 1.f;
          mMaxCloakAlpha = 0.75f;
        }
      }
    }
    UpdateCantSeePlayer(mgr, dt);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mEnableMeleeAttack = false;
    mInAttackState = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

void CSpacePirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mNoMeleeAttack) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    mAttackTargetPos = GetTargetPos(mgr);
    mTargetDelta = mAttackTargetPos - GetTranslation();
    mSteeringSpeed = 0.f;
    mEnableMeleeAttack = true;
    mMeleeSeverity = pas::kS_One;
    mAppliedBladeDamage = false;
    mInAttackState = true;
    mCloseMelee = false;
    mChargePlayerList.remove(GetUniqueId());
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_MeleeAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(mMeleeSeverity));
    }
    if (mMeleeSeverity == pas::kS_One && mAnimationState.IsOver()) {
      CVector3f delta = GetTargetPos(mgr) - GetTranslation();
      if (delta.MagSquared() < mMinAttackRange * mMinAttackRange &&
          CVector3f::Dot(delta.AsNormalized(), GetTransform().GetForward()) > -0.123f) {
        mAnimationState.SetState(CAnimationState::kAS_Ready);
        mMeleeSeverity = pas::kS_Two;
        mAppliedBladeDamage = false;
        mTargetDelta = delta;
        mCloseMelee = true;
      }
    }
    if (mCloseMelee) {
      mTargetDelta = GetTargetPos(mgr) - GetTranslation();
    }
    BodyController()->CommandMgr().SetTargetVector(mTargetDelta);
    if (mShadowPirate) {
      if (mAnimationState.IsOver()) {
        mAlphaDelta = -0.4f;
      } else {
        mAlphaDelta = 1.f;
        mMaxCloakAlpha = 0.75f;
      }
    }
    UpdateCantSeePlayer(mgr, dt);
    UpdateHeldPosition(mgr, dt);
    CheckBlade(mgr);
    break;
  case kStateMsg_Deactivate:
    mEnableMeleeAttack = false;
    mInAttackState = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CSpacePirate::ShouldCrouch(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint);
  if (coverPoint) {
    result = coverPoint->ShouldCrouch();
  }
  return result;
}

void CSpacePirate::Crouch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->SetLocomotionType(pas::kLT_Crouch);
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      mTargetDelta = coverPoint->GetTransform().GetForward();
    }
    mSteeringSpeed = 0.f;
    mCoverDir = pas::kCD_Invalid;
    break;
  case kStateMsg_Update:
    BodyController()->CommandMgr().SetTargetVector(mTargetDelta);
    UpdateCantSeePlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSpacePirate::ShouldStrafe(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  bool noPlayerStrafe = false;
  mSkidDir = pas::kSD_Invalid;
  if (!mNonAggressive) {
    CVector3f toTarget = const_cast< CSpacePirate* >(this)->GetTargetPos(mgr) - GetTranslation();
    if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f) {
      if ((mLowHealthFrenzyTimer < 0.66f || mTimeSinceHitByPlayer < 0.66f) &&
          mStrafeDelayTimer == 0.f) {
        CVector3f center = GetBoundingBox().GetCenterPoint();
        CVector3f delta =
            (const_cast< CSpacePirate* >(this)->GetTargetPos(mgr) - center).AsNormalized();
        if (CVector3f::Dot(delta, GetTransform().GetForward()) > 0.707f) {
          mSkidDir = const_cast< CSpacePirate* >(this)->GetStrafeDir(mgr, 10.f);
          if (mSkidDir != pas::kSD_Invalid) {
            result = true;
          } else {
            noPlayerStrafe = true;
          }
        }
      }
      if (!noPlayerStrafe && !result && mTimeNoPlayerLos > 1.f) {
        if (const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId))) {
          if ((player->GetTranslation() - GetTranslation()).Magnitude() < 15.f &&
              mSkidDir == pas::kSD_Invalid) {
            mSkidDir = const_cast< CSpacePirate* >(this)->GetStrafeDir(mgr, 5.f);
            if (mSkidDir != pas::kSD_Invalid) {
              result = true;
            }
          }
        }
      }
    }
  }
  return result;
}

void CSpacePirate::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStrafeDelayTimer = 4.f;
    mInAttackState = true;
    break;
  case kStateMsg_Update:
    if (GetBodyController()->GetCurrentStateId() != pas::kAS_Step) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mSkidDir, pas::kStep_Normal));
    }
    break;
  case kStateMsg_Deactivate:
    mInAttackState = false;
    break;
  }
}

void CSpacePirate::Approach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    mSteeringSpeed = 1.f;
    break;
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

bool CSpacePirate::SpotPlayer(CStateManager& mgr, const CTriggerData& data) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    CVector3f toPlayer = mgr.GetPlayer(i)->GetTranslation() - GetTranslation();
    float distance = toPlayer.Magnitude();
    float angle = mDetectionAngle;
    if (CVector3f::Dot(toPlayer, GetTransform().GetForward()) > distance * angle) {
      return true;
    }
  }
  return false;
}

bool CSpacePirate::LineOfSight(CStateManager& mgr, const CTriggerData& data) const {
  return !mNoPlayerLos;
}

void CSpacePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mJumpPoint = kInvalidUniqueId;
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      mReflectedDestPos = GetTranslation();
      mInPosition = false;
      mDestObj = coverPoint->GetUniqueId();
      mDestPos = coverPoint->GetTranslation();
    }
    if (GetSearchPath()->Search(GetTranslation(), mDestPos) == CPathFindSearch::kR_Success) {
      mReflectedDestPos = GetTranslation();
      mDestPos = GetSearchPath()->GetPoint();
      mInPosition = false;
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(mDestPos - GetTranslation(), CVector3f::Zero(), 1.f));
    } else {
      CScriptAiJumpPoint* best = nullptr;
      float minDistSq = FLT_MAX;
      CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
      for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
        if (CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(list[i])) {
          if (jumpPoint->GetActive() && jumpPoint->GetType() == 0 &&
              !jumpPoint->GetInUse(GetUniqueId()) &&
              jumpPoint->GetJumpTarget() == kInvalidUniqueId &&
              jumpPoint->GetCurrentAreaId() == GetCurrentAreaId()) {
            CVector3f toJump = jumpPoint->GetTranslation() - GetTranslation();
            float distSq = toJump.MagSquared();
            if (distSq > 25.f &&
                CVector3f::Dot(jumpPoint->GetTransform().GetForward(), toJump) > 0.f) {
              if (const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(
                      mgr.GetObjectById(jumpPoint->GetJumpPoint()))) {
                if ((mDestPos.GetZ() - GetTranslation().GetZ()) *
                        (waypoint->GetTranslation().GetZ() - jumpPoint->GetTranslation().GetZ()) >
                    0.f) {
                  CVector3f toDest = mDestPos - waypoint->GetTranslation();
                  distSq += 4.f * toJump.GetZ() * toJump.GetZ();
                  distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
                  if (distSq < minDistSq &&
                      GetSearchPath()->PathExists(GetTranslation(), jumpPoint->GetTranslation()) ==
                          CPathFindSearch::kR_Success) {
                    bool good = false;
                    bool noPath =
                        GetSearchPath()->PathExists(waypoint->GetTranslation(), mDestPos) !=
                        CPathFindSearch::kR_Success;
                    if (noPath) {
                      distSq += 1000.f;
                    }
                    if (!noPath) {
                      good = true;
                    }
                    if (distSq < minDistSq) {
                      minDistSq = distSq;
                      best = jumpPoint;
                      if (good) {
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (best) {
        mDestPos = best->GetTranslation();
        if (GetSearchPath()->Search(GetTranslation(), mDestPos) == CPathFindSearch::kR_Success) {
          mReflectedDestPos = GetTranslation();
          mDestPos = GetSearchPath()->GetPoint();
          mInPosition = false;
          mJumpPoint = best->GetUniqueId();
          mJumpHeight = best->GetJumpApex();
          if (const CScriptWaypoint* waypoint =
                  TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(best->GetJumpPoint()))) {
            mPatrolDestPos = waypoint->GetTranslation();
            BodyController()->CommandMgr().DeliverCmd(
                CBCLocomotionCmd(mDestPos, CVector3f::Zero(), 1.f));
          }
        }
      }
    }
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
    if (mEnableAim) {
      mSteeringSpeed = 1.f;
    }
    mInRange = false;
    mNormalDodge = true;
    break;
  case kStateMsg_Update:
    CPatterned::PathFind(mgr, msg, dt);
    BodyController()->CommandMgr().SetTargetVector(mgr.GetPlayer(0)->GetTranslation() -
                                                   GetTranslation());
    if (mJumpPoint != kInvalidUniqueId) {
      if (CScriptAiJumpPoint* jumpPoint =
              TCastToPtr< CScriptAiJumpPoint >(mgr.ObjectById(mJumpPoint))) {
        float maxSpeed = BodyController()->BodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = GetModelData()->GetScale();
        float jumpDistance = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY()) + mIntoJumpDist;
        if ((GetTranslation() - jumpPoint->GetTranslation()).MagSquared() <
            jumpDistance * jumpDistance) {
          mAnimationState.SetState(CAnimationState::kAS_Ready);
          if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
            BodyController()->CommandMgr().DeliverCmd(CBCJumpCmd(
                mDestPos, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
          }
          mInJump = true;
        }
      }
    }
    AvoidActors(mgr);
    if (!mInRange) {
      if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
        float maxSpeed = BodyController()->BodyStateInfo().GetMaxSpeed();
        const CVector3f& scale = GetModelData()->GetScale();
        mCoverRange = maxSpeed * ((1.5f * dt + 0.1f) * scale.GetY());
        if (coverPoint->ShouldWallHang()) {
          mCoverRange += mIntoJumpDist;
        }
        mInRange = (GetTranslation() - coverPoint->GetTranslation()).MagSquared() <
                   mCoverRange * mCoverRange;
      }
    }
    UpdateCantSeePlayer(mgr, dt);
    UpdateHeldPosition(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    CPatterned::PathFind(mgr, msg, dt);
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mJumpPoint = kInvalidUniqueId;
    mInRange = false;
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

bool CSpacePirate::InRange(CStateManager& mgr, const CTriggerData& data) const { return mInRange; }

void CSpacePirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyController()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    CVector3f targetPos = GetTargetPos(mgr);
    if (!mNoShuffleCloseCheck && TooClose(mgr, CTriggerData(0.f))) {
      SetDestPos(GetTranslation() +
                 mMinAttackRange * (GetTranslation() - targetPos).AsNormalized() +
                 Random2f(mgr, 0.f, 5.f));
      mDestObj = kInvalidUniqueId;
      mShuffleClose = true;
    } else {
      CVector3f fromTarget = GetTranslation() - targetPos;
      CVector3f side = CVector3f::Cross(CVector3f::Up(), fromTarget);
      float distance = mMaxAttackRange * mgr.Random()->Float() + mMaxAttackRange;
      float sideDistance = 2.f * mMaxAttackRange * (mgr.Random()->Float() - 0.5f);
      SetDestPos(targetPos + distance * fromTarget.AsNormalized() +
                 sideDistance * side.AsNormalized());
      mDestObj = kInvalidUniqueId;
      mShuffleClose = false;
    }
    mSteeringSpeed = 1.f;
    break;
  }
  }
  CPatterned::PathFind(mgr, msg, dt);
  BodyController()->CommandMgr().SetTargetVector(mgr.GetPlayer(0)->GetTranslation() -
                                                 GetTranslation());
  switch (msg) {
  case kStateMsg_Update:
    AvoidActors(mgr);
    break;
  case kStateMsg_Deactivate:
    mShuffleClose = false;
    break;
  }
}

void CSpacePirate::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAttackTargetPos = GetTargetPos(mgr);
    CVector3f delta = mAttackTargetPos - GetTranslation();
    delta.SetZ(0.f);
    if (CVector3f::Dot(GetTransform().GetForward(), delta.AsNormalized()) < 0.8f) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    break;
  }
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Turn)) {
      CVector3f delta = mAttackTargetPos - GetTranslation();
      if (delta.IsMagnitudeSafe()) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(delta.AsNormalized(), CVector3f::Zero(), 1.f));
      } else {
        mAnimationState.SetState(CAnimationState::kAS_Over);
      }
    }
    UpdateCantSeePlayer(mgr, dt);
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CSpacePirate::ShouldDodge(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (mEnableDodge) {
    if (!mNonAggressive && !mNoPlayerDodge) {
      CVector3f toTarget = const_cast< CSpacePirate* >(this)->GetTargetPos(mgr) - GetTranslation();
      if (CVector3f::Dot(toTarget, GetTransform().GetForward()) > 0.f &&
          (mTimeSinceHitByPlayer < 0.33f || mLowHealthFrenzyTimer < 0.33f) &&
          mTimeNoPlayerLos < 0.5f) {
        result = true;
      }
    }
    if (!result) {
      if (const CMetroid* metroid = TCastToConstPtr< CMetroid >(mgr.GetObjectById(mTargetId))) {
        if (metroid->IsAttacking()) {
          CVector3f delta = GetTranslation() - metroid->GetTranslation();
          if (CVector3f::Dot(delta, metroid->GetTransform().GetForward()) > 0.f) {
            result = true;
          }
        }
      }
    }
  }
  return result;
}

void CSpacePirate::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mEnableBreakDodge = false;
    if (!mNormalDodge && !mNoBreakDodge && mDodgeDelayTimer <= 0.f) {
      const float chance =
          0.15f * (1.f + 4.f * (mInitialHP - GetHealthInfo()->GetHP()) / mInitialHP);
      if (mgr.Random()->Float() < chance) {
        mEnableBreakDodge = true;
      }
      mDodgeDelayTimer =
          mgr.Random()->Range(mPirateData.mDodgeDelayTimeMin, mPirateData.mDodgeDelayTimeMax);
    }
    mDodgeDir = GetStrafeDir(mgr, mEnableBreakDodge ? mBreakDodgeDist : mDodgeDist);
    if (mDodgeDir != pas::kSD_Invalid) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
    }
    break;
  case kStateMsg_Update:
    if (!mEnableBreakDodge) {
      if (mNormalDodge || mgr.Random()->Float() < 0.5f) {
        if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
          BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDir, pas::kStep_Dodge));
        }
      } else if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
        BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDir, pas::kStep_RollDodge));
      }
    } else {
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Step)) {
        BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDir, pas::kStep_BreakDodge));
      }
      if (GetMaterialList().HasMaterial(kMT_Orbit) && mStateMachine->GetTime() > 0.5f) {
        RemoveMaterial(kMT_Orbit, mgr);
        mgr.GetPlayer(0)->SetOrbitRequestForTarget(GetUniqueId(), CPlayer::kOR_ActivateOrbitSource,
                                                   mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mNoPlayerDodge = true;
    if (!GetMaterialList().HasMaterial(kMT_Orbit)) {
      AddMaterial(kMT_Orbit, mgr);
    }
    break;
  }
}

bool CSpacePirate::ShouldRetreat(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  if (mEnableRetreat && !mTrooper) {
    TUniqueId wpId = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId));
    if (!wp) {
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->state == kSS_Retreat && it->msg == kSM_Follow) {
          TUniqueId id = mgr.GetIdForScript(it->objId);
          wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
          if (wp) {
            break;
          }
        }
      }
    }
    if (wp) {
      mDestObj = wpId;
      const_cast< CSpacePirate* >(this)->SetDestPos(wp->GetTranslation());
    } else {
      mDestObj = kInvalidUniqueId;
      const_cast< CSpacePirate* >(this)->SetDestPos(GetTranslation());
    }
    const_cast< CSpacePirate* >(this)->mEnableRetreat = false;
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    result = true;
    const_cast< CSpacePirate* >(this)->ReleaseCoverPoint(mgr, mCoverPoint, true);
    mHearNoise = false;
    mEnableAim = false;
    mHitByPlayerProjectile = false;
  }
  return result;
}

bool CSpacePirate::ShouldMove(CStateManager& mgr, const CTriggerData& data) const {
  CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint);
  return coverPoint && !coverPoint->ShouldStay();
}

bool CSpacePirate::ShotAt(CStateManager& mgr, const CTriggerData& data) const {
  return mLowHealthFrenzyTimer < (data.GetFloat() ? data.GetFloat() : 0.5f);
}

bool CSpacePirate::Attacked(CStateManager& mgr, const CTriggerData& data) const {
  return mTimeSinceHitByPlayer < (data.GetFloat() ? data.GetFloat() : 0.5f);
}

bool CSpacePirate::HasTargetingPoint(CStateManager& mgr, const CTriggerData& data) const {
  bool result = true;
  CActor* actor = TCastToPtr< CActor >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)));
  CPlayer* player = TCastToPtr< CPlayer >(actor);
  if (player || !actor || !actor->GetActive()) {
    result = false;
    if (!player) {
      mTargetId = ChooseTarget(mgr);
      const_cast< CSpacePirate* >(this)->SetTeamMemberTarget(mgr);
      mBoneTracking.SetTarget(mTargetId);
    }
    float scale = 1.f;
    float margin = mPirateData.mSearchRadius * scale;
    CVector3f extent(margin, margin, margin);
    CAABox bounds(GetTranslation() - extent, GetTranslation() + extent);
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    mgr.BuildNearList(nearList, bounds, CMaterialFilter::MakeExclude(CMaterialList(kMT_Solid)),
                      nullptr);
    for (int i = 0; i < nearList.size(); ++i) {
      const CScriptTargetingPoint* point =
          TCastToConstPtr< CScriptTargetingPoint >(mgr.GetObjectById(nearList[i]));
      if (point && point->GetActive() && point->GetCurrentAreaId() == GetCurrentAreaId() &&
          !point->GetLocked()) {
        result = true;
        mTargetId = point->GetUniqueId();
        const_cast< CSpacePirate* >(this)->SetTeamMemberTarget(mgr);
        mBoneTracking.SetTarget(mTargetId);
        break;
      }
    }
  }
  return result;
}

void CSpacePirate::TargetCover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      mDestObj = mCoverPoint;
      mDestPos = coverPoint->GetTranslation();
    }
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    break;
  }
}

void CSpacePirate::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mDestObj = mgr.GetPlayer(0)->GetUniqueId();
    SetDestPos(mgr.GetPlayer(0)->GetTranslation());
    mReflectedDestPos = GetTranslation();
    mInPosition = false;
    break;
  }
}

bool CSpacePirate::HasAttackPattern(CStateManager& mgr, const CTriggerData& data) const {
  return GetConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

bool CSpacePirate::IsAmbushing(CStateManager& mgr, const CTriggerData& data) const {
  return mPendingAmbush;
}

bool CSpacePirate::ShouldWarpIn(CStateManager& mgr, const CTriggerData& data) const {
  return mWarpPhase == -1 && mTrooper && mWarpInRequested;
}

bool CSpacePirate::ShouldLaunchGrenade(CStateManager& mgr, const CTriggerData& data) const {
  if (mPirateData.mWeaponData.mEquippedWeapon == kEW_GrenadeLauncher) {
    return mAttackRemTime <= 0.f;
  }
  return false;
}

bool CSpacePirate::InProjectileRange(CStateManager& mgr, const CTriggerData& data) const {
  switch (mPirateData.mWeaponData.mEquippedWeapon) {
  case kEW_GrenadeLauncher: {
    const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
    if (target) {
      float minSq = mPirateData.mWeaponData.mGrenadeMinAttackDist *
                    mPirateData.mWeaponData.mGrenadeMinAttackDist;
      float maxSq = mPirateData.mWeaponData.mGrenadeMaxAttackDist *
                    mPirateData.mWeaponData.mGrenadeMaxAttackDist;
      float dx = target->GetTranslation().GetX() - GetTranslation().GetX();
      float dy = target->GetTranslation().GetY() - GetTranslation().GetY();
      float dz = target->GetTranslation().GetZ() - GetTranslation().GetZ();
      float distSq = dx * dx + dy * dy + dz * dz;
      if (distSq >= minSq && distSq <= maxSq) {
        return true;
      }
    }
    break;
  }
  case kEW_None:
    return true;
  }
  return false;
}

bool CSpacePirate::ShouldWallHang(CStateManager& mgr, const CTriggerData& data) const {
  CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint);
  return coverPoint && coverPoint->ShouldWallHang();
}

void CSpacePirate::Ambushing(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == kStateMsg_Activate) {
    if (mCeilingAmbush) {
      BodyController()->SetLocomotionType(pas::kLT_Internal6);
    } else {
      BodyController()->SetLocomotionType(pas::kLT_Crouch);
    }
  }
}

void CSpacePirate::WarpIn(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mColor.SetAlpha(0.f);
    mAlphaDelta = 0.f;
    mWarpPhase = 0;
    mWarpTimeCaptured = false;
    RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    break;
  case kStateMsg_Update:
    switch (mWarpPhase) {
    case 0: {
      rstl::vector< const CScriptWaypoint* > waypoints;
      waypoints.reserve(GetConnectionList().size());
      rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
      for (; it != GetConnectionList().end(); ++it) {
        if (it->state == kSS_GRNT && it->msg == kSM_Follow) {
          TUniqueId id = mgr.GetIdForScript(it->objId);
          const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
          if (wp) {
            waypoints.push_back_unsafe(wp);
          }
        }
      }
      if (!waypoints.empty()) {
        int index = mgr.Random()->Range(0, waypoints.size() - 1);
        SetTranslation(waypoints[index]->GetTranslation());
        SetTransform(CQuaternion::FromMatrix(waypoints[index]->GetTransform())
                         .BuildTransform4f(GetTranslation()));
      }
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      CMaterialFilter filter =
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid, kMT_Player, kMT_Character));
      CVector3f extent(10.f, 10.f, 10.f);
      CAABox bounds(GetTranslation() - extent, GetTranslation() + extent);
      mgr.BuildNearList(nearList, bounds, filter, this);
      if (!CGameCollision::DetectDynamicCollisionBoolean(*GetCollisionPrimitive(), GetTransform(),
                                                         nearList, mgr)) {
        mWarpPhase = 1;
        AddMaterial(kMT_Character, kMT_Solid, kMT_Target, mgr);
      }
      break;
    }
    case 1:
      if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
        BodyController()->CommandMgr().DeliverCmd(
            CBCGenerateCmd(pas::kGType_One, CVector3f::Zero()));
      } else if (BodyController()->GetCurrentStateId() == pas::kAS_Generate) {
        if (!mWarpTimeCaptured) {
          mWarpTimeCaptured = true;
          mWarpTime = BodyController()->GetAnimTimeRemaining();
        } else if (mWarpTime > FLT_EPSILON) {
          float ratio = BodyController()->GetAnimTimeRemaining() / mWarpTime;
          mColor.SetAlpha(CMath::Max(0.f, 1.f - ratio));
        }
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    AddMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
    mColor.SetAlpha(1.f);
    mAlphaDelta = 0.f;
    mWarpPhase = -1;
    mWarpInRequested = false;
    break;
  }
}

void CSpacePirate::WarpOut(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mWarpPhase = 1;
    mWarpTimeCaptured = false;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, CVector3f::Zero()));
    } else if (BodyController()->GetCurrentStateId() == pas::kAS_Generate) {
      if (!mWarpTimeCaptured) {
        mWarpTimeCaptured = true;
        mWarpTime = BodyController()->GetAnimTimeRemaining();
      } else if (mWarpTime > FLT_EPSILON) {
        float ratio = BodyController()->GetAnimTimeRemaining() / mWarpTime;
        mColor.SetAlpha(CMath::Min(1.f, ratio));
      }
    }
    break;
  case kStateMsg_Deactivate:
    mColor.SetAlpha(0.f);
    mAlphaDelta = 0.f;
    break;
  }
}

void CSpacePirate::PostWarpOut(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (!mWarpInRequested) {
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), GetUniqueId(), kSM_Deactivate));
    }
    mWarpPhase = -1;
    if (mDeleteAfterWarpOut) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    break;
  }
}

void CSpacePirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mInWallHang = true;
    if (CScriptCoverPoint* coverPoint = GetCoverPoint(mgr, mCoverPoint)) {
      if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(
              const_cast< CEntity* >(mgr.GetObjectById(coverPoint->CheckConnectedObject_if(
                  mgr, kSS_Arrived, kSM_Next, CEffectWaypointPredicate()))))) {
        mDestObj = waypoint->GetUniqueId();
        mDestPos = waypoint->GetTranslation();
        mReflectedDestPos = GetTranslation();
        mInPosition = false;
      }
      mTargetDelta = coverPoint->GetTransform().GetForward();
    }
    mInAttackState = true;
    mBoneTracking.SetActive(false);
    mCannotShoot = true;
    break;
  case kStateMsg_Update: {
    bool tryWallHang = true;
    if (mAnimationState.GetState() == CAnimationState::kAS_Ready &&
        CVector3f::GetAngleDiff(GetTransform().GetForward(), mTargetDelta) > M_PIF / 12.f) {
      tryWallHang = false;
      BodyController()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), mTargetDelta, 1.f));
    }
    if (tryWallHang && mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_WallHang)) {
      BodyController()->CommandMgr().DeliverCmd(CBCWallHangCmd(mDestObj));
    }
    if (BodyController()->GetCurrentStateId() == pas::kAS_WallHang) {
      mCannotShoot = !BodyController()->GetBodyStateInfo().GetCurrentState()->CanShoot();
    }
    mBurstFire.SetBurstType(1);
    break;
  }
  case kStateMsg_Deactivate:
    mInWallHang = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mInAttackState = false;
    mBoneTracking.SetActive(true);
    mCannotShoot = false;
    break;
  }
}

void CSpacePirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mInWallHang = true;
    mCannotShoot = true;
    mWallDetaching = true;
    break;
  case kStateMsg_Update:
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  case kStateMsg_Deactivate:
    mInWallHang = false;
    mCannotShoot = false;
    mWallDetaching = false;
    break;
  }
}

bool CSpacePirate::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  if (mInWallHang) {
    return GetBodyController()->GetCurrentStateId() != pas::kAS_WallHang;
  }
  return CPatterned::AnimOver(mgr, data);
}

bool CSpacePirate::ShouldJumpBack(CStateManager& mgr, const CTriggerData& data) const {
  return !mNoShuffleCloseCheck || mHoldPositionTime > 6.f;
}

void CSpacePirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (!ShouldJumpBack(mgr, CTriggerData(0.f))) {
    return;
  }
  switch (msg) {
  case kStateMsg_Activate:
    if (!mOnlyAttackInRange && !IsPathClear(mgr, -GetTransform().GetForward(), 5.f)) {
      const float height = GetSearchPath()->GetCharacterHeight();
      mPathFindSearch.SetCharacterHeight(5.f + height);
      const CVector3f dest = GetTranslation() + 10.f * GetTransform().GetForward();
      if (GetSearchPath()->Search(GetTranslation(), dest) == CPathFindSearch::kR_Success &&
          (GetSearchPath()->GetWaypoints().back() - dest).MagSquared() < 3.f) {
        if (CMath::AbsF(GetSearchPath()->RemainingPathDistance(GetTranslation()) - 10.f) < 4.f) {
          mPatrolDestPos = GetSearchPath()->GetWaypoints().back();
          mJumpHeight = 5.f;
          mUseJumpBackJump = true;
          mAnimationState.SetState(CAnimationState::kAS_Ready);
        }
      }
      GetSearchPath()->SetCharacterHeight(height);
    }
    break;
  case kStateMsg_Update:
    if (!mUseJumpBackJump) {
      BodyController()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      BodyController()->CommandMgr().SetTargetVector(GetTargetPos(mgr) - GetTranslation());
    } else if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mDestPos, pas::kJT_Normal, pas::kJS_IntoJump, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    break;
  case kStateMsg_Deactivate:
    if (mUseJumpBackJump) {
      mAnimationState.SetState(CAnimationState::kAS_NotReady);
      mUseJumpBackJump = false;
    }
    mHoldPositionTime = 0.f;
    break;
  }
}

bool CSpacePirate::ShouldSpecialAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mOnlyAttackInRange && !mBurstFire.IsBurstSet() && mAttackRemTime > 2.f) {
    return true;
  }
  return false;
}

bool CSpacePirate::LostInterest(CStateManager& mgr, const CTriggerData& data) const {
  if (mOnlyAttackInRange && mAttackRemTime < 1.5f) {
    return true;
  }
  return false;
}

bool CSpacePirate::StartAttack(CStateManager& mgr, const CTriggerData& data) const {
  if (mMayStartAttack) {
    mMayStartAttack = false;
    return true;
  }
  return false;
}

bool CSpacePirate::BreakAttack(CStateManager& mgr, const CTriggerData& data) const {
  return mBreakAttack;
}

bool CSpacePirate::BounceFind(CStateManager& mgr, const CTriggerData& data) const {
  bool result = false;
  float minDistSq = FLT_MAX;
  CScriptAiJumpPoint* best = nullptr;
  CObjectList& list = mgr.ObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(list[i])) {
      if (jumpPoint->GetActive() && !jumpPoint->GetInUse(GetUniqueId()) &&
          jumpPoint->GetType() == 0 && jumpPoint->GetJumpTarget() != kInvalidUniqueId &&
          jumpPoint->GetCurrentAreaId() == GetCurrentAreaId()) {
        CVector3f toJump = jumpPoint->GetTranslation() - GetTranslation();
        float distSq = toJump.MagSquared();
        if (distSq < minDistSq &&
            CVector3f::Dot(jumpPoint->GetTransform().GetForward(), toJump) > 0.f) {
          if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(
                  mgr.GetObjectById(jumpPoint->GetJumpTarget()))) {
            CVector3f toDest = mDestPos - wp->GetTranslation();
            distSq += toDest.MagSquared() + 9.f * toDest.GetZ() * toDest.GetZ();
            if (distSq < minDistSq &&
                CVector3f::Dot(wp->GetTransform().GetForward(), toDest) > 0.f &&
                const_cast< CSpacePirate* >(this)->GetSearchPath()->PathExists(
                    GetTranslation(), jumpPoint->GetTranslation()) == CPathFindSearch::kR_Success) {
              bool good = false;
              bool noPath = const_cast< CSpacePirate* >(this)->GetSearchPath()->PathExists(
                                wp->GetTranslation(), mDestPos) != CPathFindSearch::kR_Success;
              if (noPath) {
                distSq += 1000.f;
              }
              if (!noPath) {
                good = true;
              }
              if (distSq < minDistSq) {
                minDistSq = distSq;
                best = jumpPoint;
                if (good) {
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  if (best) {
    if (const CScriptWaypoint* wp =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(best->GetJumpPoint()))) {
      const_cast< CSpacePirate* >(this)->SetDestPos(best->GetTranslation());
      result = true;
      mJumpPoint = best->GetUniqueId();
      mJumpHeight = best->GetJumpApex();
      mPatrolDestPos = wp->GetTranslation();
    }
  }
  return result;
}

void CSpacePirate::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mInRange = false;
    break;
  case kStateMsg_Update:
    AvoidActors(mgr);
    if (!mInRange) {
      if (const CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(
              const_cast< CEntity* >(mgr.GetObjectById(mJumpPoint)))) {
        const float maxSpeed = BodyController()->GetBodyStateInfo().GetMaxSpeed();
        mCoverRange =
            maxSpeed * ((1.5f * dt + 0.1f) * GetModelData()->GetScale().GetY()) + mIntoJumpDist;
        mInRange = (GetTranslation() - jumpPoint->GetTranslation()).MagSquared() <
                   mCoverRange * mCoverRange;
      }
    }
    break;
  case kStateMsg_Deactivate:
    mInRange = false;
    break;
  }
}

void CSpacePirate::Bounce(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (const CScriptAiJumpPoint* jumpPoint = TCastToPtr< CScriptAiJumpPoint >(
            const_cast< CEntity* >(mgr.GetObjectById(mJumpPoint)))) {
      const TUniqueId target = jumpPoint->GetJumpTarget();
      if (const CScriptWaypoint* waypoint =
              TCastToPtr< CScriptWaypoint >(const_cast< CEntity* >(mgr.GetObjectById(target)))) {
        CBodyStateCmdMgr& cmdMgr = BodyController()->CommandMgr();
        cmdMgr.DeliverCmd(CBCJumpCmd(mPatrolDestPos, waypoint->GetTranslation()));
      }
    }
    break;
  case kStateMsg_Update:
    if (mStateMachine->GetTime() > 0.1f && BodyController()->GetCurrentStateId() != pas::kAS_Jump) {
      StateMachineState().SetCodeTrigger();
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSpacePirate::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::Dead(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking.SetActive(false);
    SetEyeParticleActive(mgr, false);
    SquadReset(mgr);
    break;
  case kStateMsg_Update:
    if (BodyController()->GetCurrentStateId() == pas::kAS_Death) {
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      RemoveMaterial(kMT_GroundCollider, kMT_Solid, kMT_AIBlock, mgr);
      AddMaterial(kMT_ProjectilePassthrough, mgr);
      SetMomentumWR(CVector3f::Zero());
      CPhysicsActor::Stop();
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CSpacePirate::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mPendingDeath = true;
    break;
  }
}

void CSpacePirate::RemoveFromWorld(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SetActive(false);
    mgr.DeleteObjectRequest(GetUniqueId());
    break;
  }
}

void CSpacePirate::LaunchGrenade(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mTeamAiMgrId == kInvalidUniqueId ||
        CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                      GetUniqueId())) {
      mAnimationState.SetState(CAnimationState::kAS_Ready);
      mGrenadesToLaunch = mgr.Random()->Range(1, mPirateData.mWeaponData.mGrenadeCount);
    } else {
      mAnimationState.SetState(CAnimationState::kAS_Over);
    }
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_LoopAttack)) {
      BodyController()->CommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::kLAT_Zero, true));
    } else if (mGrenadesToLaunch <= 0) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    mAttackRemTime = GetAverageAttackTime();
    if (mgr.IsRandomAvailable() == true) {
      const float variation = mAttackTimeVariation;
      mAttackRemTime += mgr.Random()->Float() * variation;
    }
    BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    CScriptTeamAiMgr::EndAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId, GetUniqueId(),
                                false);
    break;
  }
}

void CSpacePirate::Captured(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mDisabledAnimationDeltas = 3;
    CPhysicsActor::Stop();
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, -1));
    RemoveMaterial(kMT_Orbit, kMT_Target, kMT_Solid, mgr);
    SetDrawShadow(false);
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Generate)) {
      BodyController()->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Seven, -1));
    }
    break;
  case kStateMsg_Deactivate:
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CSpacePirate::OffLine(CStateManager& mgr, const CTriggerData& data) const {
  return GetBodyController()->GetCurrentStateId() != pas::kAS_Jump && !IsOnGround();
}

bool CSpacePirate::Landed(CStateManager& mgr, const CTriggerData& data) const {
  return IsOnGround();
}

void CSpacePirate::Jump(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimationState.SetState(CAnimationState::kAS_Ready);
    mLeashTimer = 0.f;
    break;
  case kStateMsg_Update:
    if (mAnimationState.CanIssueCommand(*BodyController(), pas::kAS_Jump)) {
      mJumpVelSet = true;
      BodyController()->CommandMgr().DeliverCmd(
          CBCJumpCmd(mDestPos, pas::kJT_Normal, pas::kJS_Loop, 0, CBCJumpCmd::kFF_AmbushJump));
    }
    UpdateLeashTimer(dt);
    break;
  case kStateMsg_Deactivate:
    mJumpVelSet = false;
    mAnimationState.SetState(CAnimationState::kAS_NotReady);
    break;
  }
}

bool CSpacePirate::Leash(CStateManager& mgr, const CTriggerData& data) const {
  return mLeashTimer > data.GetFloat();
}

void CSpacePirate::UpdateLeashTimer(float dt) {
  if (BodyController()->GetPercentageFrozen() != 1.f && !BodyController()->IsElectrocuting()) {
    mLeashTimer += dt;
  }
}

void CSpacePirate::RequestWarpOut(CStateManager& mgr, bool deleteAfter) {
  if (GetAlive() && !mGettingUp && mWarpPhase == -1 && mTrooper) {
    mStateMachine->SetState(mgr, *this, rstl::string_l("WarpOut"));
    mDeleteAfterWarpOut = deleteAfter;
  }
}

void CSpacePirate::SetupWeaponModel(const CSpacePirateWeaponData& weaponData) {
  switch (weaponData.mEquippedWeapon) {
  case kEW_GrenadeLauncher:
    if (weaponData.mGrenadeLauncher != kInvalidAssetId) {
      mGrenadeLauncherModel =
          CModelData(CStaticRes(weaponData.mGrenadeLauncher, GetModelData()->GetScale()));
    }
    break;
  }
}

CProjectileInfo* CSpacePirate::ProjectileInfo() {
  if (mProjectileInfo) {
    return &mProjectileInfo.data();
  }
  return nullptr;
}

bool CSpacePirate::FireProjectile(float dt, CStateManager& mgr) {
  bool result = false;
  const CTransform4f gunXf = GetLctrTransform(mGunSeg);
  if (!mAlive) {
    LaunchProjectile(gunXf, mgr, 6, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                     CVector3f(1.f, 1.f, 1.f));
    result = true;
  } else if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetId))) {
    CVector3f pos = actor->GetTranslation();
    bool inTurret = false;
    if (const CPlayer* player = TCastToConstPtr< CPlayer >(actor)) {
      if (player->Get_x12f8() == CPlayer::kTS_Active) {
        pos = player->GetTranslation();
        inTurret = true;
      } else {
        pos = ProjectileInfo()->PredictInterceptPos(
            gunXf.GetTranslation(), player->GetAimPosition(mgr, 0.f), *player, true, dt);
      }
    }
    CVector3f gunToPos = pos - gunXf.GetTranslation();
    const float distance = gunToPos.Magnitude();
    gunToPos *= 1.f / distance;
    const float dot = CVector3f::Dot((GetLctrTransform(mWristSeg).GetTranslation() -
                                      GetLctrTransform(mElbowSeg).GetTranslation())
                                         .AsNormalized(),
                                     gunToPos);
    if (dot > 0.707f || (distance < 6.f && dot > 0.5f)) {
      if (inTurret || LineOfSightTest(mgr, gunXf.GetTranslation(), pos,
                                      CMaterialList(kMT_Player, kMT_ProjectilePassthrough))) {
        pos += GetTransform().Rotate(mBurstFire.GetDistanceCompensatedError(distance, 6.f));
        const CTransform4f shotXf =
            CTransform4f::LookAt(gunXf.GetTranslation(), pos, CVector3f::Up());
        LaunchProjectile(shotXf, mgr, 6, CWeapon::kPA_None, false, CImpactVisorEffect::None(),
                         CVector3f(1.f, 1.f, 1.f));
        result = true;
      }
    }
  }
  if (result) {
    CSfxManager::AddEmitter(mPirateData.mSound_Projectile, GetTranslation(),
                            GetCurrentAreaId().Value(), true, false, CSfxManager::kMedPriority);
  }
  const bool fired = result;
  return fired;
}

void CSpacePirate::LaunchBouncyGrenade(CStateManager& mgr) {
  --mGrenadesToLaunch;
  const CTransform4f launchXf = GetLctrTransform(mGunSeg);
  const CVector3f origin = launchXf.GetTranslation();
  float angle = 0.f;
  float speed = mPirateData.mWeaponData.mGrenadeMinLaunchSpeed;
  const CActor* target = static_cast< const CActor* >(mgr.GetObjectById(mTargetId));
  if (target) {
    const CVector3f aim = GetGrenadeTargetPosition(mgr, target);
    SolveGrenadeLaunch(aim, origin, angle, speed);

    CVector3f flat = aim - origin;
    flat.SetZ(0.f);
    const CVector3f forward = launchXf.GetColumn(kDY);
    CVector3f direction = flat.CanBeNormalized() ? flat.AsNormalized() : forward;
    if (CVector3f::GetAngleDiff(forward, direction) > M_PIF / 4.f) {
      direction = CVector3f::Slerp(forward, direction, CRelAngle::FromRadians(M_PIF / 4.f));
    }
    const CVector3f look =
        CVector3f::Slerp(direction, CVector3f::Up(), CRelAngle::FromRadians(angle));
    const CTransform4f grenadeXf = CTransform4f::LookAt(origin, origin + look, CVector3f::Up());

    CEntity* grenade = rs_new CBouncyGrenade(
        mgr.AllocateUniqueId(), rstl::string_l("Bouncy Grenade"),
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true), grenadeXf,
        CModelData::CModelDataNull(), CActorParameters::None(), GetUniqueId(), speed,
        mPirateData.mWeaponData.mGrenade, 5.f, CAABox::MakeMaxInvertedBox(), kInvalidUniqueId, 0.f,
        0, nullptr, nullptr);
    if (grenade) {
      mgr.AddObject(grenade);
    }
  }
}

CVector3f CSpacePirate::GetGrenadeTargetPosition(const CStateManager& mgr,
                                                 const CActor* target) const {
  CVector3f aim = target->GetAimPosition(mgr, 0.5f);
  const CPlayer* player = TCastToConstPtr< CPlayer >(target);
  if (player && player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    aim -= CVector3f(0.f, 0.f, 0.5f * player->GetEyeHeight());
  }
  if (mPirateData.mWeaponData.mGrenade.GetNumBounces() != 0) {
    const CVector3f pos = GetTranslation();
    if (aim.GetZ() <= pos.GetZ() + 2.f) {
      aim = pos + 0.7f * (aim - pos);
    }
  }
  return aim;
}

void CSpacePirate::SolveGrenadeLaunch(const CVector3f& target, const CVector3f& origin,
                                      float& outAngle, float& outSpeed) const {
  float angle = 0.f;
  float speed = mPirateData.mWeaponData.mGrenadeMinLaunchSpeed;
  float bestError = FLT_MAX;
  const float heightDelta = target.GetZ() - origin.GetZ();
  const CVector2f offset = CVector2f(target.GetX() - origin.GetX(), target.GetY() - origin.GetY());
  const float distance = offset.Magnitude();
  const float halfGravityDistSq = 0.5f * kDefaultGravityAccel * distance * distance;
  const float minSpeedSq = mPirateData.mWeaponData.mGrenadeMinLaunchSpeed *
                           mPirateData.mWeaponData.mGrenadeMinLaunchSpeed;
  const float maxSpeedSq = mPirateData.mWeaponData.mGrenadeMaxLaunchSpeed *
                           mPirateData.mWeaponData.mGrenadeMaxLaunchSpeed;
  float startAngle = 0.f;
  float stepAngle = M_PIF / 40.f;
  if (target.GetZ() > origin.GetZ()) {
    startAngle = M_PIF / 4.f;
    stepAngle = -stepAngle;
  }
  for (float i = 0.f; i < 10.f; i += 1.f) {
    const float candidate = stepAngle * i + startAngle;
    const float cosine = CMath::FastCosR(candidate);
    const float sine = CMath::FastSinR(candidate);
    const float denominator = distance * (cosine * sine) - heightDelta * (cosine * cosine);
    if (denominator > FLT_EPSILON) {
      const float speedSq = halfGravityDistSq / denominator;
      if (speedSq >= minSpeedSq && speedSq <= maxSpeedSq) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        break;
      }
      const float error = speedSq > maxSpeedSq ? speedSq - maxSpeedSq : minSpeedSq - speedSq;
      if (error < bestError) {
        angle = candidate;
        speed = CMath::SqrtF(speedSq);
        bestError = error;
      }
    }
  }
  outAngle = angle;
  outSpeed = speed;
}

void CSpacePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BeginAction:
    RemoveMaterial(kMT_Solid, mgr);
    mAllEnergyDrained = true;
    handled = true;
    break;
  case kUE_EndAction:
    mCloseMelee = false;
    handled = true;
    break;
  case kUE_DeGenerate:
  case kUE_BecomeRagDoll:
    if (mOnlyAttackInRange || GetHealthInfo()->GetHP() <= 0.f) {
      mRagdollDelayTimer = mgr.Random()->Float() * 0.05f + 0.001f;
    }
    handled = true;
    break;
  case kUE_IkLock:
    if (!mIkChain.GetActive()) {
      const CSegId& bone = AnimationData()->GetLocatorSegId(node.GetLocatorName());
      if (bone.val() != 0) {
        const CTransform4f xf = GetLctrTransform(bone);
        mIkChain.Activate(*AnimationData(), bone, xf);
        mSatUp = true;
      }
    }
    handled = true;
    break;
  case kUE_IkRelease:
    mIkChain.Deactivate();
    handled = true;
    break;
  case kUE_ScreenShake:
    SendScriptMsgs(kSS_Play, mgr, kSM_None);
    handled = true;
    break;
  case kUE_FadeOut:
    if (mShadowPirate) {
      mAlphaDelta = -0.8f;
      mgr.ActorModelParticles()->StartElectric(*this);
      mElectricParticleTimer = 1.f;
    }
    handled = true;
    break;
  case kUE_Projectile:
    LaunchBouncyGrenade(mgr);
    handled = true;
    break;
  case kUE_BreakLockOn:
    if (GetAlive()) {
      handled = true;
    }
    break;
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

uchar CSpacePirate::GetModelAlphau8(const CStateManager& mgr) const {
  return mShadowPirate ? static_cast< int >(mShadowPirateAlpha * 255.f) : mColor.GetAlphau8();
}

const CDamageVulnerability* CSpacePirate::GetDamageVulnerability() const {
  if (mWarpPhase != -1) {
    return &CDamageVulnerability::PassThroughVulnerabilty();
  }
  return CPatterned::GetDamageVulnerability();
}

const CDamageVulnerability* CSpacePirate::GetDamageVulnerability(const CVector3f& position,
                                                                 const CVector3f& direction,
                                                                 const CDamageInfo& damage) const {
  return GetDamageVulnerability();
}

void CSpacePirate::PreRenderAllViewports(CStateManager& mgr) {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed()) {
    mRagDoll->PreRenderAllViewports(*this, 0.2f);
    UpdatePortalSystemState(mgr);
  } else {
    CPatterned::PreRenderAllViewports(mgr);
  }
}

void CSpacePirate::PreRender(CStateManager& mgr) {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed()) {
    mRagDoll->PreRender(GetTranslation(), *ModelData());
  }
  CPatterned::PreRender(mgr);
  if (mRagDoll.get() == nullptr || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                            ModelData()->GetScale(), *BodyController());
    mIkChain.PreRender(*ModelData()->AnimationData(), GetTransform(), ModelData()->GetScale());
  }
  if (CMath::AbsF(mPortalPlane.GetConstant()) > 0.f) {
    const CModelFlags flags = GetModelFlags();
    SetModelFlags(CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_Unknown80));
  }
}

void CSpacePirate::Render(const CStateManager& mgr) const {
  const float time = GetAlive() ? CGraphics::GetSecondsMod900() : 0.f;
  if (mWarpPhase == 1) {
    const float strength = CMath::FastSinR(M_PIF * mColor.GetAlpha());
    mgr.DrawSpaceWarp(GetBoundingBox().GetCenterPoint(), strength);
  }
  const CTimeProvider provider(time);
  if (CMath::AbsF(mPortalPlane.GetConstant()) > 0.f) {
    GetModelData()->SetupWorldSpacePortalPlane(GetTransform(), mPortalPlane);
  }
  CPatterned::Render(mgr);
  RenderGrenadeLauncher(mgr, GetTransform(), GetModelFlags());
}

void CSpacePirate::RenderGrenadeLauncher(const CStateManager& mgr, const CTransform4f& xf,
                                         const CModelFlags& flags) const {
  const int alphaBuffer = GetRenderAlphaBufferAlpha(mgr);
  if (alphaBuffer != -1) {
    gpRender->SetDestinationAlpha(alphaBuffer);
  }
  if (GetModelAlphau8(mgr) != 0 && mGrenadeLauncherModel) {
    const CTransform4f launcherXf = xf * GetScaledLocatorTransform(mWristSeg);
    mGrenadeLauncherModel->Render(mgr, launcherXf, GetActorLights(), flags);
  }
  if (alphaBuffer != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

CAABox CSpacePirate::GetSortingBounds(const CStateManager& mgr) const {
  const CAABox bounds = GetModelData()->GetBounds(GetTransform());
  const CVector3f center = bounds.GetCenterPoint();
  const CVector3f radius = (bounds.GetMaxPoint() - bounds.GetMinPoint()) * 0.25f;
  return CAABox(center - radius, center + radius);
}

CAABox CSpacePirate::GetScanVisorRenderBounds(const CStateManager& mgr) const {
  CAABox box = CAABox::MakeMaxInvertedBox();
  box = GetAnimationData()->CalcBoundingBoxFromModelVerts();
  box = box.GetTransformedAABox(CTransform4f::Translate(-GetTranslation()) * GetTransform() *
                                CTransform4f::Scale(GetModelData()->GetScale()));
  return box;
}

void CSpacePirate::ScanVisorRender(const CStateManager& mgr, const CTransform4f& xf,
                                   const CModelFlags& flags) const {
  if (!GetModelData()->IsNull()) {
    GetModelData()->Render(CModelData::kWM_Normal, xf, nullptr, flags);
  }
  RenderGrenadeLauncher(mgr, xf, flags);
}

bool CSpacePirate::ShouldFrenzy(CStateManager& mgr) {
  bool reset = false;
  if (mPendingFrenzyChance) {
    mPendingFrenzyChance = false;
    if (mgr.Random()->Next() % 100 < 25) {
      reset = true;
    }
  }
  if (!mChargePlayerList.empty()) {
    reset = true;
  }
  const CPlayer* player = TCastToConstPtr< CPlayer >(mgr.GetObjectById(mTargetId));
  if (player && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    reset = true;
  }
  if (GetHealthInfo()->GetHP() < 0.3f * mInitialHP && mgr.Random()->Next() % 100 < 60 &&
      mLowHealthFrenzyTimer < 0.5f) {
    reset = true;
  }
  if (reset) {
    mFrenzyFrames = mgr.Random()->Range(2, 4);
  }
  return --mFrenzyFrames >= 0;
}

void CSpacePirate::UpdateCloak(float dt, CStateManager& mgr) {
  if (mShadowPirate) {
    if (mAlive) {
      if (mCloakDelayTimer > 0.f) {
        mCloakDelayTimer -= dt;
        if (mCloakDelayTimer <= 0.f) {
          mAlphaDelta = -0.4f;
        }
      }
    } else {
      mMinCloakAlpha = 0.f;
      mMaxCloakAlpha = 1.f;
    }
    if (mElectricParticleTimer > 0.f) {
      mElectricParticleTimer -= dt;
      if (mElectricParticleTimer <= 0.f && !BodyController()->IsElectrocuting()) {
        mgr.ActorModelParticles()->StopElectric(*this);
      }
    }
    if (BodyController()->GetPercentageFrozen() != 1.f) {
      mAlphaDelta = 2.f;
    }
    if (mAlphaDelta < 0.f && mColor.GetAlpha() < mMinCloakAlpha) {
      mColor.SetAlpha(mMinCloakAlpha);
      mAlphaDelta = 0.f;
      RemoveMaterial(kMT_Target, mgr);
    }
    if (mAlphaDelta > 0.f && mColor.GetAlpha() > mMaxCloakAlpha) {
      mColor.SetAlpha(mMaxCloakAlpha);
      AddMaterial(kMT_Target, mgr);
    }
    mCloakStepTime -= dt;
    if (mCloakStepTime < 0.f) {
      const float random = mgr.Random()->Float();
      mCloakStepTime = 0.08f * (1.f - random);
      if (mAlphaDelta < 0.f) {
        mShadowPirateAlpha = mColor.GetAlpha();
        if (mAlive) {
          mShadowPirateAlpha -= random * (mColor.GetAlpha() - mMinCloakAlpha);
        }
      } else if (mAlphaDelta > 0.f) {
        mShadowPirateAlpha = mColor.GetAlpha() + random * (mMaxCloakAlpha - mColor.GetAlpha());
      } else {
        mShadowPirateAlpha = mColor.GetAlpha();
      }
    }
  }
}

void CSpacePirate::UpdateKnockBackSfx() {
  if (mKnockBackSfx) {
    if (CSfxManager::IsPlaying(mKnockBackSfx) || CSfxManager::IsQueued(mKnockBackSfx)) {
      CSfxManager::UpdateEmitter(mKnockBackSfx, GetTranslation(), GetTransform().GetForward(), 127);
    } else {
      mKnockBackSfx.Clear();
    }
  }
}

void CSpacePirate::UpdateAttacks(float dt, CStateManager& mgr) {
  if (mPirateData.mWeaponData.mEquippedWeapon == kEW_None) {
    CPlayer* player = nullptr;
    bool reset = true;
    if ((!mAlive || (GetBodyController()->GetBodyStateInfo().GetCurrentState()->CanShoot() &&
                     mEnableAim && !BodyController()->IsFrozen() && !mMelee && !mCeilingAmbush &&
                     !mStarted && !BodyController()->IsElectrocuting())) &&
        mBurstFire.GetBurstType() != -1) {
      player = TCastToPtr< CPlayer >(const_cast< CEntity* >(mgr.GetObjectById(mTargetId)));
      if (mAlive) {
        if (!mOnlyAttackInRange ||
            (player && (player->GetTranslation() - GetTranslation()).MagSquared() <
                           mLeashRadius * mLeashRadius)) {
          reset = false;
          mAttackRemTime -= dt;
          if (mAttackRemTime < 0.f) {
            const CTeamAiRole* role =
                CScriptTeamAiMgr::GetTeamAiRole(mgr, mTeamAiMgrId, GetUniqueId());
            if (!role || role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
              if (mTeamAiMgrId == kInvalidUniqueId ||
                  CScriptTeamAiMgr::StartAttack(CScriptTeamAiMgr::kAT_Projectile, mgr, mTeamAiMgrId,
                                                GetUniqueId())) {
                if (ShouldFrenzy(mgr)) {
                  mBurstFire.SetBurstType(2);
                }
                if (mSeated) {
                  mBurstFire.SetBurstType(5);
                }
                if (player &&
                    CVector3f::Dot(player->GetTransform().GetForward(),
                                   GetTranslation() - player->GetTranslation()) < 0.f &&
                    mBurstFire.GetBurstType() < 6) {
                  mBurstFire.SetBurstType(mBurstFire.GetBurstType() + 6);
                }
                mBurstFire.Start(mgr);
                const float variation = mAttackTimeVariation;
                const float average = GetAverageAttackTime();
                const float random = mgr.Random()->Float();
                mAttackRemTime = random * variation + average;
                if (player) {
                  const CVector3f fromPlayer =
                      (GetGunEyePos() - player->GetAimPosition(mgr, 0.f)).AsNormalized();
                  if (CVector3f::Dot(fromPlayer, player->GetTransform().GetForward()) < 0.9f) {
                    const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
                    for (int i = list.GetFirstObjectIndex(); i != -1;
                         i = list.GetNextObjectIndex(i)) {
                      if (CSpacePirate* pirate =
                              TCastToPtr< CSpacePirate >(const_cast< CEntity* >(list[i]))) {
                        if (pirate != this && pirate->mEnableAim &&
                            pirate->GetCurrentAreaId() == GetCurrentAreaId()) {
                          mAttackRemTime += 0.2f;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      mBurstFire.Update(mgr, dt);
      if (mBurstFire.ShouldFire()) {
        if (player && player->GetSidewaysDashing() && mgr.Random()->Float() < 0.5f) {
          mBurstFire.SetAvoidAccuracy(true);
        }
        FireProjectile(dt, mgr);
        mBurstFire.SetAvoidAccuracy(false);
        if (IsIngPossessed()) {
          const float variation = mPirateData.mIngNextShotTimeVariation;
          const float average = mPirateData.mIngAverageNextShotTime;
          mBurstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + average);
        } else {
          const float variation = mPirateData.mNextShotTimeVariation;
          const float average = mPirateData.mAverageNextShotTime;
          mBurstFire.SetTimeToNextShot(variation * (mgr.Random()->Float() - 0.5f) + average);
        }
      } else if (!mBurstFire.IsBurstSet()) {
        reset = true;
      }
    }
    if (reset) {
      SquadReset(mgr);
    }
    for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
      SetValidTarget(i, CheckTargetable(mgr));
    }
  } else if (mPirateData.mWeaponData.mEquippedWeapon == kEW_GrenadeLauncher) {
    mAttackRemTime -= dt;
  }
}

void CSpacePirate::UpdateAimBodyState(float dt, CStateManager& mgr) {
  if (mAlive && mEnableAim && !BodyController()->IsFrozen() &&
      !BodyController()->IsElectrocuting() && !mMelee && mRagDoll.get() == nullptr &&
      (!mSeated || mSatUp) && !mCannotShoot) {
    mAimDelayTimer = CMath::Max(0.f, mAimDelayTimer - dt);
    if (mAimDelayTimer <= 0.f) {
      BodyController()->CommandMgr().DeliverCmd(CBCAdditiveAimCmd(mInWallHang != 0));
      const CTransform4f gunXf = GetLctrTransform(mGunSeg);
      const CVector3f offset =
          mInWallHang ? CVector3f::Zero() : GetTranslation() - gunXf.GetTranslation();
      const CVector3f targetPos = GetTargetPos(mgr);
      CVector3f direction = GetTransform().TransposeMultiply(
          targetPos + CVector3f(offset.GetX(), offset.GetY(), 0.f));
      if (mInWallHang) {
        direction = CVector3f(-direction.GetX(), -direction.GetY(), direction.GetZ());
      }
      BodyController()->CommandMgr().DeliverAdditiveTargetVector(direction);
      mAimReleaseTimer = 0.5f;
    }
  } else if (mAimReleaseTimer > 0.f) {
    mAimReleaseTimer -= dt;
    BodyController()->CommandMgr().DeliverAdditiveTargetVector(GetTransform().GetForward());
    if (mAimReleaseTimer <= 0.f) {
      BodyController()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_AdditiveIdle));
    }
  }
}

void CSpacePirate::PreThink(float dt, CStateManager& mgr) {
  if (mRagDoll.get() == nullptr || !mRagDoll->IsPrimed()) {
    mBoneTracking.PreThink(*AnimationData());
  }
  CPatterned::PreThink(dt, mgr);
}

void CSpacePirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  EchoEmitter()->SetBounds(CAABox(GetTranslation(), GetTranslation()));
  if (!BodyController()->GetIsActive()) {
    BodyController()->Activate(mgr, pas::kAS_Invalid);
  }
  bool inCineCam = false;
  if (!mgr.IsMultiplayer() && mgr.GetCameraManager(0)->IsInCinematicCamera()) {
    inCineCam = true;
  }
  if (inCineCam && !mPrevInCineCam) {
    SetCinematicCollision(mgr);
  } else if (!inCineCam && mPrevInCineCam && !mRagdollNoAiCollision) {
    SetNonCinematicCollision(mgr);
  }
  mPrevInCineCam = inCineCam;
  const float steeringSpeed = mSteeringDelayTimer ? 0.f : mSteeringSpeed;
  BodyController()->CommandMgr().SetSteeringSpeedRange(steeringSpeed, steeringSpeed);
  mUnkTimer = CMath::Max(0.f, mUnkTimer - dt);
  if (mAlive) {
    mTimeSinceHitByPlayer += dt;
    mLowHealthFrenzyTimer += dt;
    if (mInProjectilePath) {
      mLowHealthFrenzyTimer = 0.f;
      mInProjectilePath = false;
    }
    if (mHitByPlayerProjectile) {
      mTimeSinceHitByPlayer = 0.f;
      mHitByPlayerProjectile = false;
    }
  }
  UpdateCloak(dt, mgr);
  UpdateKnockBackSfx();
  if (BodyController()->GetPercentageFrozen() < 1.f) {
    if (mAlive) {
      mSteeringDelayTimer = CMath::Max(0.f, mSteeringDelayTimer - dt);
      if (mNoPlayerLos) {
        mTimeNoPlayerLos += dt;
      } else {
        mTimeNoPlayerLos = 0.f;
      }
      mStrafeDelayTimer = CMath::Max(0.f, mStrafeDelayTimer - dt);
      mDodgeDelayTimer = CMath::Max(0.f, mDodgeDelayTimer - dt);
      CheckForProjectiles(mgr);
      if (mEnableAim) {
        mTargetId = ChooseTargetPlayer(mgr);
        SetTeamMemberTarget(mgr);
      }
    }
    UpdateAttacks(dt, mgr);
    UpdateAimBodyState(dt, mgr);
    mIkChain.Update(dt);
  }
  const bool noRagDoll = mRagDoll.null();
  if (noRagDoll || (!mFloatingCorpse && !mRagDoll->IsPrimed())) {
    CPatterned::Think(dt, mgr);
    if (BodyController()->GetPercentageFrozen() != 1.f) {
      mBoneTracking.Think(dt);
    }
  } else {
    CActor::Think(dt, mgr);
    UpdateAlphaDelta(mgr, dt);
    UpdateHitDamageTime(dt);
    UpdateIngPossession(dt);
    if (BodyController()->IsFrozen()) {
      BodyController()->UnFreeze();
    }
  }
  if (!noRagDoll) {
    if (!mRagDoll->IsPrimed()) {
      mRagDoll->Prime(mgr, GetTransform(), *ModelData());
      const CVector3f position = GetTranslation();
      SetTransform(CTransform4f::Identity());
      SetTranslation(position);
      BodyController()->SetPlaybackRate(0.f);
    } else {
      float waterTop = -FLT_MAX / 2.f;
      if (InFluidId() != kInvalidUniqueId) {
        const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
        if (water && water->GetActive()) {
          waterTop = water->GetTriggerBoundsWR().GetMaxPoint().GetZ();
        }
      }
      mRagDoll->Update(mgr, dt * GetDeathTimeScale(), waterTop);
      ModelData()->AdvanceParticles(GetTransform(), dt, mgr);
    }
    if (mRagDoll->IsOver() && !mRagDoll->WillContinueSmallMovements()) {
      SetMomentumWR(CVector3f::Zero());
      Stop();
      if (!GetFadeToDeath()) {
        SetFadeToDeath(true);
        mAlphaDelta = -1.f / 3.f;
        mAllEnergyDrained = true;
      }
    }
  }
  if (mRagdollDelayTimer > 0.f) {
    mRagdollDelayTimer -= dt;
    if (mRagdollDelayTimer <= 0.f) {
      if (mRagDoll.null()) {
        const float* first = skRagDollParticleRadii;
        const float* last = first + ARRAY_SIZE(skRagDollParticleRadii);
        const rstl::reserved_vector< float, 14 > radii(first, last);
        mRagDoll =
            rs_new CPirateRagDoll(mgr, this, mPirateData.mSound_Impact,
                                  (mFloatingCorpse ? 3 : 0) | (mRagdollNoAiCollision ? 4 : 0),
                                  skGravityConstant, -3.f, radii);
        RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      }
      mRagdollDelayTimer = 0.f;
    }
  }
}

bool CSpacePirate::TryToBeCaptured(CStateManager& mgr) {
  mStateMachine->SetState(mgr, *this, rstl::string_l("Captured"));
  return true;
}

rstl::optional_object< CAABox > CSpacePirate::GetTouchBounds() const {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed() && mRagDoll->IsRenderBoundsValid()) {
    return mRagDoll->GetCachedRenderBounds();
  }
  return CPatterned::GetTouchBounds();
}

CRagDoll* CSpacePirate::GetRagDoll() const {
  if (mRagDoll.get() != nullptr && mRagDoll->IsPrimed() && mRagDoll->IsRenderBoundsValid()) {
    return mRagDoll.get();
  }
  return nullptr;
}

CEntity* LoadSpacePirate(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpacePirate sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpacePirate.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  const CBouncyGrenadeData grenade(
      sldrThis.weaponData.grenadeMass, sldrThis.weaponData.unknown_0xed086ce0,
      LdrToDamageInfo(sldrThis.weaponData.grenadeDamage), sldrThis.weaponData.grenadeNumBounces,
      sldrThis.weaponData.grenadeExplosion, sldrThis.weaponData.grenadeExplosion,
      sldrThis.weaponData.grenadeTrail, sldrThis.weaponData.grenadeEffect,
      sldrThis.weaponData.sound_GrenadeBounce, sldrThis.weaponData.sound_GrenadeExplode, 0.1f,
      150.f, 0.1f, 150.f, true);
  const CSpacePirate::CSpacePirateWeaponData weaponData(
      sldrThis.weaponData.equippedWeapon, sldrThis.weaponData.grenadeLauncher, grenade,
      sldrThis.weaponData.unknown_0xa95a025b, sldrThis.weaponData.grenadeMinLaunchSpeed,
      sldrThis.weaponData.grenadeMaxLaunchSpeed, sldrThis.weaponData.grenadeMinAttackDist,
      sldrThis.weaponData.grenadeMaxAttackDist);
  const CSpacePirate::CSpacePirateData data(
      sldrThis.aggressiveness, sldrThis.coverCheck, sldrThis.searchRadius, sldrThis.fallBackCheck,
      sldrThis.fallBackRadius, sldrThis.hearingRadius, sldrThis.flags, sldrThis.unknown_0xce670970,
      sldrThis.projectile, LdrToDamageInfo(sldrThis.projectileDamage), sldrThis.sound_Projectile,
      LdrToDamageInfo(sldrThis.bladeDamage), sldrThis.kneelAttackChance, sldrThis.kneelAttackShot,
      LdrToDamageInfo(sldrThis.kneelAttackDamage), sldrThis.dodgeCheck, sldrThis.sound_Impact,
      sldrThis.intraBurstShotTime, sldrThis.intraBurstShotVariation, sldrThis.unknown_0x5080162a,
      sldrThis.unknown_0xc78b40e0, sldrThis.sound_Alert, sldrThis.gunTrackDelay,
      sldrThis.unknown_0x1b454a27, sldrThis.cloakOpacity, sldrThis.maxCloakOpacity,
      sldrThis.breakDodgeMinTime, sldrThis.breakDodgeMaxTime, sldrThis.sound_Hurled,
      sldrThis.sound_Death, sldrThis.unknown_0x8708b7d3, sldrThis.avoidDistance, 0.5f, weaponData);

  return rs_new CSpacePirate(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      *modelData, LdrToActorParameters(sldrThis.actorInformation),
      LdrToPatternedInfo(sldrThis.patterned, &sldrThis.ingPossessionData), data);
}

static void SetFuncPtrs() {
  static SSpacePirate_FuncPtrs funcPtrs;
  funcPtrs.mLoadSpacePirate = &LoadSpacePirate;
  funcPtrs.mAttachActor = &CSpacePirate::AttachActorToPirate;
  funcPtrs.mDetachActor = &CSpacePirate::DetachActorFromPirate;
  SetSSpacePirate_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSpacePirate_FuncPtrs(nullptr); }
