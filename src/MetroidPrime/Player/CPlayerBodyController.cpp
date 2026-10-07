#include "MetroidPrime/Player/CPlayerBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include <string.h>

const TStateMachineState< CPlayerBodyController >::STriggerFunction
    CPlayerBodyController::skTriggerFunctions[22] = {
        {"StateOver", &CPlayerBodyController::StateOver},
        {"Jump", &CPlayerBodyController::Jump},
        {"Grappling", &CPlayerBodyController::Grappling},
        {"Dash", &CPlayerBodyController::Dash},
        {"TransitionToBall", &CPlayerBodyController::TransitionToBall},
        {"TransitionToPlayer", &CPlayerBodyController::TransitionToPlayer},
        {"HeavyHit", &CPlayerBodyController::HeavyHit},
        {"Delay", &CPlayerBodyController::Delay},
        {"IsMorphball", &CPlayerBodyController::IsMorphball},
        {"IsFirstPerson", &CPlayerBodyController::IsFirstPerson},
        {"Shocked", &CPlayerBodyController::Shocked},
        {"ShouldUnFreeze", &CPlayerBodyController::ShouldUnFreeze},
        {"ShouldFlinch", &CPlayerBodyController::ShouldFlinch},
        {"ShouldAim", &CPlayerBodyController::ShouldAim},
        {"AdditiveStateOver", &CPlayerBodyController::AdditiveStateOver},
        {"TransitionToScrewAttack", &CPlayerBodyController::TransitionToScrewAttack},
        {"WallSlideJump", &CPlayerBodyController::WallSlideJump},
        {"ScrewAttackExpired", &CPlayerBodyController::ScrewAttackExpired},
        {"ScrewAttackHitGround", &CPlayerBodyController::ScrewAttackHitGround},
        {"StartWallJump", &CPlayerBodyController::StartWallJump},
        {"WallSlideExpired", &CPlayerBodyController::WallSlideExpired},
        {"WallJumpScrewAttackExpired", &CPlayerBodyController::WallJumpScrewAttackExpired},
};

const TStateMachineState< CPlayerBodyController >::SStateFunction
    CPlayerBodyController::skStateFunctions[26] = {
        {"Start", &CPlayerBodyController::Start},
        {"Locomotion", &CPlayerBodyController::Locomotion},
        {"Morphball", &CPlayerBodyController::Morphball},
        {"MorphToBall", &CPlayerBodyController::MorphToBall},
        {"MorphToPlayer", &CPlayerBodyController::MorphToPlayer},
        {"PlayerJump", &CPlayerBodyController::PlayerJump},
        {"PlayerGrapple", &CPlayerBodyController::PlayerGrapple},
        {"PlayerDash", &CPlayerBodyController::PlayerDash},
        {"KnockBack", &CPlayerBodyController::KnockBack},
        {"Dead", &CPlayerBodyController::Dead},
        {"GibDeath", &CPlayerBodyController::GibDeath},
        {"AdditiveIdle", &CPlayerBodyController::AdditiveIdle},
        {"AdditiveFlinch", &CPlayerBodyController::AdditiveFlinch},
        {"AdditiveShock", &CPlayerBodyController::AdditiveShock},
        {"AdditiveUnFreeze", &CPlayerBodyController::AdditiveUnFreeze},
        {"AdditiveAim", &CPlayerBodyController::AdditiveAim},
        {"MorphToScrewAttack", &CPlayerBodyController::MorphToScrewAttack},
        {"ScrewAttack", &CPlayerBodyController::ScrewAttack},
        {"WallSlide", &CPlayerBodyController::WallSlide},
        {"TransitionOutOfWallSlide", &CPlayerBodyController::TransitionOutOfWallSlide},
        {"WallSlideLand", &CPlayerBodyController::WallSlideLand},
        {"TransitionOutOfWallScrewAttack", &CPlayerBodyController::TransitionOutOfWallScrewAttack},
        {"TransitionToWallScrewAttack", &CPlayerBodyController::TransitionToWallScrewAttack},
        {"WallJumpScrewAttack", &CPlayerBodyController::WallJumpScrewAttack},
        {"TransitionOutOfScrewAttack", &CPlayerBodyController::TransitionOutOfScrewAttack},
        {"TransitionOutOfScrewAttackFloor",
         &CPlayerBodyController::TransitionOutOfScrewAttackFloor},
};

CPlayerBodyController::CPlayerBodyController(CPlayer& player, CAssetId stateMachine)
: CEntity(kInvalidUniqueId, NullEntityInfo, rstl::string_l("PlayerAnimCtrl"), 0)
, mPlayer(&player)
, mBodyStatePhase(kSP_Invalid)
, mAdditiveStatePhase(kSP_Invalid)
, mLocomotion(player)
, mAdditiveAim(player)
, mStateMachineResource(gpSimplePool->GetObj(SObjectTag('AFSM', stateMachine)))
, mAnimationId(-1)
, mAnimationDuration(0.f)
, mAnimationOver(false)
, mStateMachinesInitialized(false)
, mMoving(false)
, mFastLocomotion(false)
, mLocomotionActive(false)
, mAiming(false)
, mUnfreezing(false)
, mMorphTransitionActive(false)
, mDeathReactionOver(false)
, mDeathReactionActive(false) {
  mStateMachineResource.Lock();

  const CAnimData& animData = *player.GetAnimationData();
  const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphDuration),
                                    CPASAnimParm::FromEnum(1));
  const int animation = animData.FindBestAnimation(parameters);
  if (animation != -1) {
    mAnimationDuration = animData.GetAnimationDuration(animation);
  }
}

void CPlayerBodyController::ResetStates(CStateManager& mgr) {
  if (!mStateMachinesInitialized) {
    return;
  }

  if (mBodyState.GetState() == nullptr || strcmp("Start", mBodyState.GetName()) != 0) {
    mBodyState.SetState(mgr, *this, rstl::string_l("Start"));
  }
  if (mAdditiveState.GetState() == nullptr ||
      strcmp("AdditiveIdle", mAdditiveState.GetName()) != 0) {
    mAdditiveState.SetState(mgr, *this, rstl::string_l("AdditiveIdle"));
  }
}

void CPlayerBodyController::Update(float dt, CStateManager& mgr) {
  if (mStateMachinesInitialized) {
    SetPlaybackRate(1.f);
    mAnimationOver =
        !mPlayer->GetAnimationData()->IsAnimTimeRemaining(dt, rstl::string_l("Whole Body"));
    CheckDeathCommands(mgr);
    mBodyState.Update(mgr, *this, dt);
    mAdditiveState.Update(mgr, *this, dt);
    mCommandMgr.ClearCmds();

    if (mPendingAnimation.valid()) {
      mPlayer->AnimationData()->SetAnimation(mPendingAnimation->mParameters,
                                             mPendingAnimation->mNoTransition);
      mPlayer->ModelData()->EnableLooping(mPendingAnimation->mLooping);
      mAnimationId = mPendingAnimation->mParameters.GetAnimationId();
      mPendingAnimation = rstl::optional_object< SAnimationRequest >();
    }
  } else {
    TrySetupStateMachines(mgr);
  }
}

bool CPlayerBodyController::IsAnimationLooping() const {
  return mPlayer->GetModelData()->GetIsLoop();
}

void CPlayerBodyController::SetAnimationChangeDisabled(bool disabled) {
  mLocomotion.SetAnimationChangeDisabled(disabled);
}

void CPlayerBodyController::RequestAnimation(const CAnimPlaybackParms& parameters, bool looping,
                                             bool noTransition) {
  mPendingAnimation = SAnimationRequest(parameters, looping, noTransition);
}

void CPlayerBodyController::SetPlaybackRate(float rate) {
  mPlayer->AnimationData()->SetPlaybackRate(rate);
}

void CPlayerBodyController::MultiplyPlaybackRate(float rate) {
  mPlayer->AnimationData()->MultiplyPlaybackRate(rate);
}

const CPASDatabase& CPlayerBodyController::GetPASDatabase() const {
  return mPlayer->GetAnimationData()->GetPASDatabase();
}

void CPlayerBodyController::SelectAnimation(const CPASAnimParmData& parameters, CRandom16& random) {
  const int animation = GetPASDatabase().FindBestAnimation(parameters, random, -1).second;
  RequestAnimation(CAnimPlaybackParms(animation, -1, 1.f, true), false, false);
}

void CPlayerBodyController::SelectLoopingAnimation(const CPASAnimParmData& parameters,
                                                   CRandom16& random) {
  const int animation = GetPASDatabase().FindBestAnimation(parameters, random, -1).second;
  RequestAnimation(CAnimPlaybackParms(animation, -1, 1.f, true), true, false);
}

const CStateMachine* CPlayerBodyController::GetStateMachine() {
  if (mStateMachineResource.IsLoaded()) {
    return mStateMachineResource.GetObject();
  }
  return nullptr;
}

void CPlayerBodyController::TrySetupStateMachines(CStateManager& mgr) {
  if (mBodyState.GetState() == nullptr && GetStateMachine() != nullptr) {
    SetupStateMachines(mgr);
  }
}

void CPlayerBodyController::SetupStateMachines(CStateManager& mgr) {
  mBodyState.Setup(GetStateMachine());
  mAdditiveState.Setup(GetStateMachine());
  mBodyState.SetTriggerFunctions(skTriggerFunctions, 22);
  mBodyState.SetStateFunctions(skStateFunctions, 26);
  mAdditiveState.SetTriggerFunctions(skTriggerFunctions, 22);
  mAdditiveState.SetStateFunctions(skStateFunctions, 26);
  mStateMachinesInitialized = true;
  ResetStates(mgr);
}

void CPlayerBodyController::CheckDeathCommands(CStateManager& mgr) {
  if (mCommandMgr.GetCmd(kPBSC_DeathReaction) != nullptr &&
      (mBodyState.GetState() == nullptr || !mDeathReactionActive && !mDeathReactionOver)) {
    mBodyState.SetState(mgr, *this, rstl::string_l("Dead"));
  }
  if (mCommandMgr.GetCmd(kPBSC_GibDeath) != nullptr &&
      (mBodyState.GetState() == nullptr || !mDeathReactionActive && !mDeathReactionOver)) {
    mBodyState.SetState(mgr, *this, rstl::string_l("GibDeath"));
  }
}

void CPlayerBodyController::PlayGunReaction(CStateManager& mgr) {
  if (mPlayer != nullptr &&
      (static_cast< const CPlayer* >(mPlayer)->GetPlayerGun()->GetFiring() & 0xf) != 0) {
    const CPASDatabase& database = GetPASDatabase();
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_GunReaction));
    const rstl::pair< float, int > best = database.FindBestAnimation(parameters, *mgr.Random(), -1);
    if (best.second != -1) {
      mPlayer->AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
    }
  }
}

bool CPlayerBodyController::StateOver(CStateManager&, const float&) {
  return mBodyStatePhase == kSP_Over;
}

bool CPlayerBodyController::Jump(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_Jump) != nullptr;
}

bool CPlayerBodyController::Grappling(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_Grapple) != nullptr;
}

bool CPlayerBodyController::Dash(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_Dash) != nullptr;
}

bool CPlayerBodyController::TransitionToBall(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_MorphToBall) != nullptr;
}

bool CPlayerBodyController::TransitionToPlayer(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_MorphToPlayer) != nullptr;
}

bool CPlayerBodyController::HeavyHit(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_KnockBack) != nullptr;
}

bool CPlayerBodyController::Delay(CStateManager&, const float& arg) {
  const float delay = arg;
  return mBodyState.GetTime() > delay;
}

bool CPlayerBodyController::IsMorphball(CStateManager&, const float&) {
  CPlayer* player = TCastToPtr< CPlayer >(mPlayer);
  return player != nullptr && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed;
}

bool CPlayerBodyController::IsFirstPerson(CStateManager&, const float&) {
  CPlayer* player = TCastToPtr< CPlayer >(mPlayer);
  return player != nullptr && player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed;
}

void CPlayerBodyController::Start(CStateManager&, int, float) {
  mDeathReactionOver = false;
  mDeathReactionActive = false;
  mMorphTransitionActive = false;
}

void CPlayerBodyController::Locomotion(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mLocomotion.Start(mgr, *this);
    mLocomotionActive = true;
    break;
  case kStateMsg_Update:
    mMoving = mLocomotion.mCategory != SLocomotionState::kC_Idle;
    mFastLocomotion = mLocomotion.mCategory == SLocomotionState::kC_ForwardFast;
    mLocomotion.Update(dt, mgr, *this);
    break;
  case kStateMsg_Deactivate:
    mLocomotion.Shutdown(*this);
    mFastLocomotion = false;
    mLocomotionActive = false;
    break;
  }
}

void CPlayerBodyController::Morphball(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                      CPASAnimParm::FromEnum(8), CPASAnimParm::FromEnum(0));
    SelectAnimation(parameters, *mgr.Random());
    break;
  }
  }
}

void CPlayerBodyController::MorphToBall(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPBCMorphToBallCmd* command =
        static_cast< const CPBCMorphToBallCmd* >(mCommandMgr.GetCmd(kPBSC_MorphToBall));
    if (command != nullptr) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                        CPASAnimParm::FromEnum(command->GetTransitionType()),
                                        CPASAnimParm::FromEnum(command->GetAnimationVariant()),
                                        CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
      SelectAnimation(parameters, *mgr.Random());
      mBodyStatePhase = kSP_Active;
      mMoving = command->GetAnimationVariant() != 1;
      mMorphTransitionActive = true;
    } else {
      mBodyStatePhase = kSP_Over;
    }
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mMorphTransitionActive = false;
    break;
  }
}

void CPlayerBodyController::MorphToPlayer(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPBCMorphToPlayerCmd* command =
        static_cast< const CPBCMorphToPlayerCmd* >(mCommandMgr.GetCmd(kPBSC_MorphToPlayer));
    if (command != nullptr) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                        CPASAnimParm::FromEnum(command->GetTransitionType()),
                                        CPASAnimParm::FromEnum(command->GetAnimationVariant()),
                                        CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
      SelectAnimation(parameters, *mgr.Random());
      mBodyStatePhase = kSP_Active;
      mMoving = command->GetAnimationVariant() != 1;
      mMorphTransitionActive = true;
    } else {
      mBodyStatePhase = kSP_Over;
    }
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    } else if (mCommandMgr.GetCmd(kPBSC_ContinueLocomotion) != nullptr &&
               mBodyState.GetTime() > 0.3f) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mMorphTransitionActive = false;
    break;
  }
}

void CPlayerBodyController::PlayerJump(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mMoving = true;
    mBodyStatePhase = kSP_Active;
    mJump.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mJump.Update(mgr, *this)) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mJump.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::PlayerGrapple(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mMoving = true;
    mBodyStatePhase = kSP_Active;
    mGrapple.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mGrapple.Update(mgr, *this)) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mGrapple.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::PlayerDash(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mMoving = true;
    mBodyStatePhase = kSP_Active;
    mDash.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mDash.Update(mgr, *this)) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mDash.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::KnockBack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyStatePhase = kSP_Active;
    mKnockBack.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mKnockBack.Update(mgr, *this)) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mKnockBack.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::Dead(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    mMoving = false;
    mDeathReactionActive = true;
    const CPBCDeathReactionCmd* command =
        static_cast< const CPBCDeathReactionCmd* >(mCommandMgr.GetCmd(kPBSC_DeathReaction));
    if (command != nullptr) {
      const CVector3f localDirection =
          mPlayer->GetTransform().TransposeRotate(command->GetDirection());
      const float angle = CMath::ClampRadians(atan2(localDirection.GetY(), localDirection.GetX()));
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_DeathReaction),
                                        CPASAnimParm::FromReal32((180.f / M_PIF) * angle),
                                        CPASAnimParm::FromEnum(command->GetMode()));
      const CPASDatabase& database = GetPASDatabase();
      const rstl::pair< float, int > best =
          database.FindBestAnimation(parameters, *mgr.Random(), -1);
      RequestAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
      const CPASAnimState* state = database.GetAnimState(kPAS_DeathReaction);
      const CPASAnimParm adjust = state->GetAnimParmData(best.second, 2);
      if (adjust.GetBoolValue()) {
        const CPASAnimParm offset = state->GetAnimParmData(best.second, 0);
        const float offsetAngle = (M_PIF / 180.f) * offset.GetReal32Value();
        const CRelAngle rotation = CRelAngle::FromRadians(angle - offsetAngle);
        const CQuaternion& current = CQuaternion::FromMatrix(mPlayer->GetTransform());
        const CQuaternion& orientation = current * CQuaternion::ZRotation(rotation);
        CPlayer& player = *mPlayer;
        player.SetTransform(orientation.BuildTransform4f(player.GetTranslation()));
      }
    }
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mDeathReactionOver = true;
    }
    break;
  }
}

void CPlayerBodyController::GibDeath(CStateManager&, int msg, float) {
  if (msg == kStateMsg_Activate) {
    mMoving = false;
    mDeathReactionOver = true;
    mDeathReactionActive = true;
  }
}

bool CPlayerBodyController::Shocked(CStateManager&, const float&) {
  const CPBCAdditiveReactionCmd* command =
      static_cast< const CPBCAdditiveReactionCmd* >(mCommandMgr.GetCmd(kPBSC_AdditiveReaction));
  return command != nullptr && command->GetType() == CPBCAdditiveReactionCmd::kART_Shock;
}

bool CPlayerBodyController::ShouldUnFreeze(CStateManager&, const float&) {
  const CPBCAdditiveReactionCmd* command =
      static_cast< const CPBCAdditiveReactionCmd* >(mCommandMgr.GetCmd(kPBSC_AdditiveReaction));
  return command != nullptr && command->GetType() == CPBCAdditiveReactionCmd::kART_UnFreeze;
}

bool CPlayerBodyController::ShouldFlinch(CStateManager&, const float&) {
  return mCommandMgr.GetCmd(kPBSC_Flinch) != nullptr;
}

bool CPlayerBodyController::ShouldAim(CStateManager&, const float&) {
  const CPBCAimCmd* command = static_cast< const CPBCAimCmd* >(mCommandMgr.GetCmd(kPBSC_Aim));
  return command != nullptr && command->GetDirection().IsNonZero();
}

bool CPlayerBodyController::AdditiveStateOver(CStateManager&, const float&) {
  return mAdditiveStatePhase == kSP_Over;
}

void CPlayerBodyController::AdditiveIdle(CStateManager&, int, float) {}

void CPlayerBodyController::AdditiveFlinch(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAdditiveStatePhase = kSP_Active;
    mAdditiveFlinch.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mAdditiveFlinch.Update(mgr, *this)) {
      mAdditiveStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mAdditiveFlinch.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::AdditiveShock(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAdditiveReaction.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    mAdditiveReaction.Update(mgr, *this);
    break;
  case kStateMsg_Deactivate:
    mAdditiveReaction.Shutdown(*this);
    break;
  }
}

void CPlayerBodyController::AdditiveUnFreeze(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mAdditiveStatePhase = kSP_Active;
    mUnfreezing = true;
    mAdditiveReaction.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    if (!mAdditiveReaction.Update(mgr, *this)) {
      mAdditiveStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    mAdditiveReaction.Shutdown(*this);
    mUnfreezing = false;
    break;
  }
}

void CPlayerBodyController::AdditiveAim(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAiming = true;
    mAdditiveAim.Start(mgr, *this);
    break;
  case kStateMsg_Update:
    mAdditiveAim.Update(dt, mgr, *this);
    break;
  case kStateMsg_Deactivate:
    mAiming = false;
    mAdditiveAim.Shutdown(*this);
    break;
  }
}

bool CPlayerBodyController::WallSlideJump(CStateManager&, const float&) {
  const CPBCMorphToScrewAttackCmd* command =
      static_cast< const CPBCMorphToScrewAttackCmd* >(mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack));
  if (command != nullptr) {
    return command->GetTransitionType() == 4;
  }
  return false;
}

bool CPlayerBodyController::TransitionToScrewAttack(CStateManager&, const float&) {
  const CPBCMorphToScrewAttackCmd* command =
      static_cast< const CPBCMorphToScrewAttackCmd* >(mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack));
  if (command != nullptr) {
    return command->GetTransitionType() == 2;
  }
  return false;
}

bool CPlayerBodyController::ScrewAttackExpired(CStateManager&, const float&) {
  CPlayer* player = mPlayer;
  if (player != nullptr && player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    const CMorphBall* ball = player->GetMorphBall();
    if (!ball->InScrewAttackMode()) {
      return true;
    }
    if (ball->GetBallState() == CMorphBall::kBS_ScrewAttackRecovery &&
        player->GetPlayerMovementState() != NPlayer::kMS_OnGround) {
      return true;
    }
    if (mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack) != nullptr) {
      return true;
    }
  }
  return false;
}

bool CPlayerBodyController::ScrewAttackHitGround(CStateManager&, const float&) {
  CPlayer* player = mPlayer;
  if (player != nullptr) {
    const bool onGround = player->GetPlayerMovementState() == NPlayer::kMS_OnGround;
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
        player->GetMorphBall()->GetScrewAttackGroundedFrames() > 1) {
      return true;
    }
    if ((player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed ||
         player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) &&
        onGround) {
      return true;
    }
  }
  return false;
}

bool CPlayerBodyController::StartWallJump(CStateManager&, const float&) {
  const CPBCMorphToScrewAttackCmd* command =
      static_cast< const CPBCMorphToScrewAttackCmd* >(mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack));
  if (command != nullptr) {
    return command->GetTransitionType() == 5;
  }
  return false;
}

bool CPlayerBodyController::WallSlideExpired(CStateManager&, const float&) {
  if (mPlayer != nullptr) {
    if (mPlayer->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      return true;
    }
    if (mPlayer->GetMorphBall()->GetBallState() != CMorphBall::kBS_ScrewAttackWallJump) {
      return true;
    }
  }
  return false;
}

bool CPlayerBodyController::WallJumpScrewAttackExpired(CStateManager&, const float&) {
  if (mPlayer != nullptr) {
    if (mPlayer->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      return true;
    }
    if (mPlayer->GetMorphBall()->GetBallState() != CMorphBall::kBS_ScrewAttackWallJump) {
      return true;
    }
  }
  return false;
}

void CPlayerBodyController::MorphToScrewAttack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPBCMorphToScrewAttackCmd* command = static_cast< const CPBCMorphToScrewAttackCmd* >(
        mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack));
    if (command != nullptr) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                        CPASAnimParm::FromEnum(command->GetTransitionType()),
                                        CPASAnimParm::FromEnum(command->GetAnimationVariant()),
                                        CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
      SelectAnimation(parameters, *mgr.Random());
      mBodyStatePhase = kSP_Active;
      mMoving = true;
    } else {
      mBodyStatePhase = kSP_Over;
    }
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::ScrewAttack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                      CPASAnimParm::FromEnum(9), CPASAnimParm::FromEnum(0));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    break;
  }
  case kStateMsg_Update:
    if (mCommandMgr.GetCmd(kPBSC_Locomotion) != nullptr) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                        CPASAnimParm::FromEnum(9), CPASAnimParm::FromEnum(0));
      SelectAnimation(parameters, *mgr.Random());
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::WallSlide(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                      CPASAnimParm::FromEnum(10), CPASAnimParm::FromEnum(0));
    SelectLoopingAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::TransitionOutOfWallSlide(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                      CPASAnimParm::FromEnum(6), CPASAnimParm::FromEnum(1),
                                      CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    mFastLocomotion = false;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Jump),
                                        CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(2));
      SelectLoopingAnimation(parameters, *mgr.Random());
    }
    if (mPlayer != nullptr && mPlayer->GetPlayerMovementState() == NPlayer::kMS_OnGround) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::WallSlideLand(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPBCJumpCmd* command = static_cast< const CPBCJumpCmd* >(mCommandMgr.GetCmd(kPBSC_Jump));
    const int parameter = command != nullptr ? command->GetJumpParameter() : 3;
    const int variant = command != nullptr ? command->GetAnimationVariant() : 0;
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Jump),
                                      CPASAnimParm::FromEnum(variant),
                                      CPASAnimParm::FromEnum(parameter));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    mFastLocomotion = false;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::TransitionOutOfWallScrewAttack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                      CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1),
                                      CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
    SelectAnimation(parameters, *mgr.Random());
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::TransitionToWallScrewAttack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                      CPASAnimParm::FromEnum(4), CPASAnimParm::FromEnum(1),
                                      CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::WallJumpScrewAttack(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Locomotion),
                                      CPASAnimParm::FromEnum(9), CPASAnimParm::FromEnum(0));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::TransitionOutOfScrewAttack(CStateManager& mgr, int msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPBCMorphToScrewAttackCmd* command = static_cast< const CPBCMorphToScrewAttackCmd* >(
        mCommandMgr.GetCmd(kPBSC_MorphToScrewAttack));
    if (command != nullptr) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                        CPASAnimParm::FromEnum(command->GetTransitionType()),
                                        CPASAnimParm::FromEnum(command->GetAnimationVariant()),
                                        CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
      SelectAnimation(parameters, *mgr.Random());
    } else {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                        CPASAnimParm::FromEnum(3), CPASAnimParm::FromEnum(1),
                                        CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
      SelectAnimation(parameters, *mgr.Random());
    }
    mMoving = false;
    mBodyStatePhase = kSP_Active;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_Jump),
                                        CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(2));
      SelectLoopingAnimation(parameters, *mgr.Random());
    }
    if (ScrewAttackHitGround(mgr, static_cast< float >(dt))) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CPlayerBodyController::TransitionOutOfScrewAttackFloor(CStateManager& mgr, int msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CPASAnimParmData parameters(static_cast< pas::EAnimationState >(kPAS_MorphTransition),
                                      CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(3),
                                      CPASAnimParm::FromEnum(GetAnimationSet(mgr)));
    SelectAnimation(parameters, *mgr.Random());
    mBodyStatePhase = kSP_Active;
    mMoving = true;
    break;
  }
  case kStateMsg_Update:
    if (IsAnimationOver()) {
      mBodyStatePhase = kSP_Over;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

int CPlayerBodyController::GetAnimationSet(CStateManager& mgr) const {
  int set = 0;
  if (mgr.IsMultiplayer()) {
    set = 2;
  }
  return set;
}
