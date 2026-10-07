#include "MetroidPrime/Player/CPlayerKnockBackMgr.hpp"

#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerBodyController.hpp"
#include "MetroidPrime/Player/CPlayerRagDoll.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"

#include <float.h>

const int CPlayerKnockBackMgr::skAnimationStates[5] = {-1, 6, 8, 5, 5};
EMaterialTypes CPlayerKnockBackMgr::sDamageMaterial = kMT_Unknown59;

CPlayerKnockBackMgr::CPlayerKnockBackMgr()
: CKnockBackMgr(gpResourceFactory->GetResourceIdByName("RULE_Player")->GetId())
, mBurnRemainingTime(0.f)
, mBurnDamagePerSecond(0.f)
, mBurnOwner(kInvalidUniqueId)
, mBallExtinguishRemainingTime(0.f)
, mBurnDeathRemainingTime(0.f)
, mElectrocutionRemainingTime(0.f)
, mElectrocutionDamagePerSecond(0.f)
, mElectrocutionOwner(kInvalidUniqueId)
, mRagDollDelay(0.f)
, mFreezeDuration(0.f)
, mWasBall(false)
, mWasFrozen(false)
, mWasOnGround(false)
, mLaggedBurnDeath(false)
, mImploding(false)
, mBurnDeath(false)
, mRagDollPending(false)
, mDeathAnimationStarted(false)
, mFreezePending(false)
, mExplosionDeathStarted(false) {
  mEnableShock = true;
}

void CPlayerKnockBackMgr::Update(float dt, CStateManager& mgr, CActor& actor) {
  CPlayer* player = TCastToPtr< CPlayer >(&actor);
  if (!player) {
    return;
  }

  CKnockBackMgr::Update(dt, mgr, *player);
  if (mFreezePending && !player->IsMorphBallTransitioning()) {
    player->Freeze(mFreezeDuration, mgr, kInvalidAssetId, CSfxManager::kInternalInvalidSfxId,
                   kInvalidAssetId);
    mFreezePending = false;
  }
  UpdateBurning(dt, mgr, *player);
  UpdateImplosion(mgr, *player);
  UpdateElectrocution(dt, mgr, *player);

  mRagDollDelay -= dt;
  if (mRagDollPending && mRagDollDelay <= 0.f) {
    mRagDollPending = false;
    player->PlayerRagDoll() =
        rs_new CPlayerRagDoll(mgr, player, CSfxManager::kInternalInvalidSfxId, 0);
  }
  if (mBurnDeath && dt < mBurnDeathRemainingTime) {
    mBurnDeathRemainingTime -= dt;
  }
}

void CPlayerKnockBackMgr::KnockBack(CStateManager& mgr, CActor& actor, const CKnockBackInfo& info) {
  CPlayer* player = TCastToPtr< CPlayer >(&actor);
  if (!player || mBurnDeath || mLaggedBurnDeath ||
      player->BodyController()->IsDeathReactionActive()) {
    return;
  }

  const float power =
      info.GetDamageInfo().GetKnockBackPower(*player->GetDamageVulnerability(), 0.f);
  if (IsAlive(*player) && power <= 0.f) {
    return;
  }

  const CPlayer::EPlayerMorphBallState morphState = player->GetMorphballTransitionState();
  mWasBall = morphState == CPlayer::kMS_Morphed || morphState == CPlayer::kMS_Morphing;
  mWasFrozen = player->GetFrozenState();
  mWasOnGround = player->GetPlayerMovementState() == NPlayer::kMS_OnGround;
  CKnockBackMgr::KnockBack(mgr, *player, info);
  if (CanApplyKnockBackForce(mgr, *player, info)) {
    ApplyPlayerKnockBackForce(*player, info.GetDirection(), power, 1.f);
  }
}

void CPlayerKnockBackMgr::ResetEffects(CStateManager& mgr, CPlayer& player) {
  CActorModelParticles& particles = *mgr.ActorModelParticles();
  StopBurnDeath(mgr, player);
  particles.StopFire(player);
  particles.StopImplosion(player);
  DouseFlames();
  DouseElectrocution();
  if (player.GetFrozenState()) {
    player.BreakFrozenState(mgr, CPlayer::kBFS_Break, false);
  }

  mBurnDeathRemainingTime = 0.f;
  mLaggedBurnDeath = false;
  mImploding = false;
  mBurnDeath = false;
  mDeathAnimationStarted = false;
  mExplosionDeathStarted = false;
}

float CPlayerKnockBackMgr::GetBurnDeathAlpha() const {
  return mBurnDeath ? mBurnDeathRemainingTime * 0.5f : 1.f;
}

bool CPlayerKnockBackMgr::IsAlive(const CActor& actor) const {
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(&actor)) {
    return player->GetPlayerState()->IsPlayerAlive();
  }
  return false;
}

CKnockBackMgr::ECharacterState CPlayerKnockBackMgr::GetCharacterState(const CActor& actor) const {
  if (const CPlayer* player = TCastToConstPtr< CPlayer >(&actor)) {
    return player->GetPlayerState()->IsPlayerAlive() ? kCS_Alive : kCS_Dead;
  }
  return kCS_Invalid;
}

bool CPlayerKnockBackMgr::HasAnimReaction(const CActor& actor, EAnimReaction reaction) const {
  const int state = skAnimationStates[reaction];
  return state != -1 && actor.GetAnimationData()->GetPASDatabase().HasState(state);
}

void CPlayerKnockBackMgr::DoKnockBackAnimation(const CVector3f& direction, CStateManager& mgr,
                                               CActor& actor, float magnitude) {
  CPlayer* player = TCastToPtr< CPlayer >(&actor);
  if (!player) {
    return;
  }

  const CPlayer::EPlayerMorphBallState morphState = player->GetMorphballTransitionState();
  const bool biped = morphState == CPlayer::kMS_Unmorphed || morphState == CPlayer::kMS_Unmorphing;
  CPlayerBodyStateCmdMgr& commands = player->BodyController()->CommandMgr();
  switch (mActiveParameters.mReaction) {
  case kAR_Flinch:
    commands.DeliverCmd(CPBCFlinchCmd(-direction));
    return;
  case kAR_KnockBack:
    commands.DeliverCmd(CPBCKnockBackCmd(-direction));
    return;
  case kAR_Fall:
    if (!player->GetPlayerRagDoll() && biped &&
        player->GetPlayerMovementState() == NPlayer::kMS_OnGround) {
      player->RemoveMaterial(kMT_Orbit, kMT_Target, kMT_Unknown59, mgr);
      player->SetDeathFadeEnabled(true);
      player->SetDeathFadeDuration(1.f);
      player->SetDeathFadeDelay(1.5f);
      const EFollowUp followUp = mActiveParameters.mFollowUp;
      const bool burn = followUp == kFU_BurnDeath || followUp == kFU_LaggedBurnDeath ||
                        followUp == kFU_ImmediateDisintegration || followUp == kFU_BlackDeath;
      commands.DeliverCmd(CPBCDeathReactionCmd(-direction, burn ? CPBCDeathReactionCmd::kDRM_Burning
                                                                : CPBCDeathReactionCmd::kDRM_Fall));
      mDeathAnimationStarted = true;
      return;
    }
    break;
  case kAR_Hurled:
    break;
  default:
    return;
  }

  if (!mRagDollPending && !player->GetPlayerRagDoll() && biped) {
    player->RemoveMaterial(kMT_Orbit, kMT_Target, kMT_Unknown59, mgr);
    player->SetDeathFadeEnabled(true);
    player->SetDeathFadeDuration(1.f);
    player->SetDeathFadeDelay(1.5f);
    // The target evaluates these calls but uses the original, unnormalized vector.
    (void)CMath::SqrtF(7.5f * -player->GetGravity());
    const CVector3f velocity = direction + 4.f * CVector3f::Up();
    if (velocity.CanBeNormalized()) {
      (void)velocity.AsNormalized();
      player->SetVelocityWR(2.f * velocity);
      player->SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
    commands.DeliverCmd(CPBCDeathReactionCmd(-direction, CPBCDeathReactionCmd::kDRM_Hurled));
    mDeathAnimationStarted = true;
    mRagDollDelay = 0.1f;
    mRagDollPending = true;
  }
}

void CPlayerKnockBackMgr::Burn(float duration, float damagePerSecond, TUniqueId owner) {
  mBurnRemainingTime = duration;
  mBallExtinguishRemainingTime = 0.75f;
  mBurnDamagePerSecond = damagePerSecond;
  mBurnOwner = owner;
}

void CPlayerKnockBackMgr::DouseFlames() {
  mBurnRemainingTime = 0.f;
  mBurnDamagePerSecond = 0.f;
  mBurnOwner = kInvalidUniqueId;
}

void CPlayerKnockBackMgr::StopBurnDeath(CStateManager& mgr, CPlayer& player) {
  mgr.ActorModelParticles()->StopBurnDeath(player);
}

void CPlayerKnockBackMgr::UpdateBurning(float dt, CStateManager& mgr, CPlayer& player) {
  if (player.GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
      player.GetVelocityWR().Magnitude() > 5.f) {
    mBallExtinguishRemainingTime -= dt;
    if (mBallExtinguishRemainingTime <= 0.f) {
      mBurnRemainingTime = 0.f;
    }
  }
  if (mBurnRemainingTime > 0.f) {
    mBurnRemainingTime -= dt;
  }

  if (!(mBurnRemainingTime > 0.f)) {
    DouseFlames();
  } else if (player.GetPlayerState()->IsPlayerAlive()) {
    mgr.ActorModelParticles()->LightDudeOnFire(player);
    CDamageInfo damage;
    damage.SetWeaponMode(CWeaponMode(kWT_Light));
    damage.SetDamage(mBurnDamagePerSecond * dt);
    damage.SetRadiusDamage(damage.GetDamage());
    damage.SetKnockBackPower(FLT_EPSILON);
    EnableAnimReaction(kAR_Flinch, false);
    EnableAnimReaction(kAR_KnockBack, false);
    mgr.ApplyDamage(player.GetUniqueId(), player.GetUniqueId(), mBurnOwner, damage,
                    CMaterialFilter::MakeInclude(CMaterialList(sDamageMaterial)),
                    CVector3f::Zero());
    EnableAnimReaction(kAR_Flinch, true);
    EnableAnimReaction(kAR_KnockBack, true);
  }
}

void CPlayerKnockBackMgr::StartBurnDeath(CStateManager& mgr, CPlayer& player, EBurnDeathType type) {
  player.SetDeathFadeEnabled(false);
  player.SetDeathFadeDuration(1.f);
  player.SetDeathFadeDelay(0.f);
  mLaggedBurnDeath = type == kBDT_Lagged;
  mBurnDeath = true;
  mBurnDeathRemainingTime = 2.f;
  DouseFlames();

  CActorModelParticles& particles = *mgr.ActorModelParticles();
  particles.StopFire(player);
  particles.StartBurnDeath(player, mgr);
  if (!mLaggedBurnDeath) {
    particles.DoFirePop(player);
    particles.StartAsh(player);
  }
}

void CPlayerKnockBackMgr::ExplodeDeath(CStateManager& mgr, CPlayer& player,
                                       EExplosionDeathType type, TUniqueId source) {
  if (mExplosionDeathStarted) {
    return;
  }
  mExplosionDeathStarted = true;
  player.BodyController()->CommandMgr().DeliverCmd(CPlayerBodyStateCmd(kPBSC_GibDeath));
  if (player.GetPlayerState()->IsPlayerAlive()) {
    mgr.KillPlayer(player.GetPlayerState()->GetHealthInfo().GetHP(), player.GetUniqueId(), source);
    player.BreakFrozenState(mgr, CPlayer::kBFS_Break, false);
  }

  if (!player.GetPlayerState()->IsPlayerAlive() &&
      CMath::IsEpsilon(player.GetDeathTime(), 0.f, 0.00001f)) {
    player.SetDeathRenderingSuppressed(true);
    player.RemoveMaterial(kMT_Orbit, kMT_Target, kMT_Unknown59, mgr);
    const bool ball = player.GetMorphballTransitionState() != CPlayer::kMS_Unmorphed;
    if (mgr.IsMultiplayer()) {
      uint event = uint(-1);
      switch (type) {
      case kEDT_Normal:
        event = ball ? 'BXDG' : 'XDMG';
        break;
      case kEDT_Ice:
        event = ball ? 'BIDG' : 'IDMG';
        break;
      }
      gpGameState->GetGameMode().NotifyGenericEvent(mgr, player.GetUniqueId(), event);
    } else if (ball) {
      const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("SinglePlayerMorphGib");
      if (tag) {
        const TUniqueId uid = mgr.AllocateUniqueId();
        const CLightParameters lights(false, 1.f, CLightParameters::kST_Zero, 0.f, 0.f,
                                      CColor::Black(), true, CLightParameters::kLO_NoShadowCast,
                                      CLightParameters::kLR_FourFrames, CVector3f::Zero(), 2, 2,
                                      false, 0, true);
        const CGameSplineDesc spline(SLdrSpline(), CMotionSpline::kST_Bezier, 1.f, false);
        CScriptEffect* effect = rs_new CScriptEffect(
            uid, rstl::string_l("Morphball gib"),
            CEntityInfo(player.GetCurrentAreaId(), rstl::vector< SConnection >(), true),
            CTransform4f::Translate(player.GetTranslation()), CVector3f::One(), tag->GetId(), true,
            false, true, false, 1.f, 1.f, 0.f, 0.f, false, 1.f, 2.f, 1.f, true, true, true, lights,
            false, spline, false, false, false, CScriptEffect::kRO_Normal);
        mgr.AddObject(effect);
        player.SetDeathEffectId(uid);
      }
    }
  }
}

void CPlayerKnockBackMgr::Freeze(float duration, CPlayer& player) {
  if (!player.GetMorphBall()->InScrewAttackMode()) {
    mFreezePending = true;
    mFreezeDuration = mActiveParameters.mFollowUpDuration;
  }
}

void CPlayerKnockBackMgr::Shock(float duration, float damagePerSecond, CPlayer& player,
                                TUniqueId owner) {
  mElectrocutionRemainingTime = duration;
  mElectrocutionDamagePerSecond = damagePerSecond;
  mElectrocutionOwner = owner;
  player.GetPlayerState()->StaticInterference().AddSource(kInvalidUniqueId, 0.6f, duration);
  if (player.GetStaticTimer() < duration) {
    player.SetHudDisable(duration);
  }
}

void CPlayerKnockBackMgr::DouseElectrocution() {
  mElectrocutionRemainingTime = 0.f;
  mElectrocutionDamagePerSecond = 0.f;
  mElectrocutionOwner = kInvalidUniqueId;
}

void CPlayerKnockBackMgr::UpdateElectrocution(float dt, CStateManager& mgr, CPlayer& player) {
  if (mElectrocutionRemainingTime > 0.f) {
    mElectrocutionRemainingTime -= dt;
  }

  CActorModelParticles& particles = *mgr.ActorModelParticles();
  if (mElectrocutionRemainingTime > 0.f) {
    if (player.GetPlayerState()->IsPlayerAlive()) {
      particles.StartElectric(player);
      CDamageInfo damage;
      damage.SetWeaponMode(CWeaponMode(kWT_Annihilator));
      damage.SetDamage(mElectrocutionDamagePerSecond * dt);
      damage.SetRadiusDamage(damage.GetDamage());
      mgr.ApplyDamage(mElectrocutionOwner, player.GetUniqueId(), mElectrocutionOwner, damage,
                      CMaterialFilter::MakeInclude(CMaterialList(sDamageMaterial)),
                      CVector3f::Zero());
      player.BodyController()->CommandMgr().DeliverCmd(
          CPBCAdditiveReactionCmd(CPBCAdditiveReactionCmd::kART_Shock, true));
    } else {
      particles.StopElectric(player);
    }
  } else {
    if (mElectrocutionDamagePerSecond > 0.f) {
      particles.StopElectric(player);
    }
    DouseElectrocution();
  }
}

void CPlayerKnockBackMgr::ApplyPlayerKnockBackForce(CPlayer& player, const CVector3f& direction,
                                                    float power, float unused) {
  float force = power * 500.f;
  if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed &&
      player.GetSurfaceRestraint() == CPlayer::kSR_Air) {
    force /= 3.5f;
  }

  float maximumSpeed = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed ? 35.f : 40.f;
  maximumSpeed = rstl::max_val(maximumSpeed, player.GetVelocityWR().Magnitude());
  player.ApplyImpulseWR(force * direction, CAxisAngle::Identity());
  player.UseCollisionImpulses();
  player.SetMinimalAccelerationTimer(rstl::min_val(rstl::max_val(power / 50.f, 0.1f), 0.6f));

  const CVector3f velocity = player.GetVelocityWR();
  const float speed = velocity.Magnitude();
  const float limitedSpeed = rstl::min_val(speed, maximumSpeed);
  if (CMath::IsEpsilon(limitedSpeed, 0.f, 0.00001f)) {
    player.SetVelocityWR(CVector3f::Zero());
  } else {
    const CVector3f velocityDirection = (1.f / speed) * velocity;
    float adjustedSpeed = limitedSpeed;
    if (player.GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      const CVector3f axis = player.GetSurfaceRestraint() == CPlayer::kSR_Air
                                 ? player.GetTransform().GetRight()
                                 : player.GetTransform().GetForward();
      adjustedSpeed *= 0.65f + (1.f - 0.65f) * CMath::AbsF(CVector3f::Dot(axis, velocityDirection));
    }
    player.SetVelocityWR(adjustedSpeed * velocityDirection);
  }
}

bool CPlayerKnockBackMgr::CanApplyKnockBackForce(CStateManager& mgr, CPlayer& player,
                                                 const CKnockBackInfo& info) const {
  if (player.GetMaterialList().HasMaterial(kMT_Immovable)) {
    return false;
  }

  const bool screwAttack = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed &&
                           player.GetMorphBall()->InScrewAttackMode();
  if (screwAttack && !TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(info.GetSourceId()))) {
    const CScriptSpecialFunction* special =
        TCastToConstPtr< CScriptSpecialFunction >(mgr.GetObjectById(info.GetSourceId()));
    if (!special || special->GetFunction() != CScriptSpecialFunction::kSF_RadialDamage) {
      return false;
    }
  }
  return !(mActiveParameters.mFlags & kRF_DisablePhysics);
}

void CPlayerKnockBackMgr::ApplyFollowUp(CActor& actor, CStateManager& mgr, TUniqueId source,
                                        TUniqueId owner) {
  CPlayer* player = TCastToPtr< CPlayer >(&actor);
  if (!player) {
    return;
  }

  EFollowUp followUp = mActiveParameters.mFollowUp;
  const float health = CMath::AbsF(player->GetHealthInfo()->GetHP());
  const bool burnDeath = followUp == kFU_LaggedBurnDeath ||
                         followUp == kFU_ImmediateDisintegration || followUp == kFU_BlackDeath ||
                         followUp == kFU_BurnDeath;
  const bool ball = player->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
                    player->GetMorphballTransitionState() == CPlayer::kMS_Morphing;
  if ((!burnDeath && !player->GetPlayerState()->IsPlayerAlive() && health > 15.f) ||
      (followUp == kFU_Death && ball)) {
    followUp = player->GetFrozenState() ? kFU_IceDeath : kFU_ExplodeDeath;
  }

  switch (followUp) {
  case kFU_Freeze:
    mFreezePending = true;
    mFreezeDuration = mActiveParameters.mFollowUpDuration;
    break;
  case kFU_Shock:
  case kFU_ImmediateExplosion: {
    const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(owner));
    const TUniqueId damageOwner = weapon ? weapon->GetOwnerId() : owner;
    Shock(mActiveParameters.mFollowUpDuration, 2.f, *player, damageOwner);
    break;
  }
  case kFU_BlackDeath:
    StartBlackHoleDeath(mgr, source, *player);
    break;
  case kFU_Burn:
    if (source != player->GetUniqueId()) {
      const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(owner));
      const TUniqueId damageOwner = weapon ? weapon->GetOwnerId() : owner;
      Burn(mActiveParameters.mFollowUpDuration, gpTweakPlayerGun->GetPlayerBurnDamage(),
           damageOwner);
    }
    break;
  case kFU_Death:
    player->SetDeathFadeEnabled(true);
    player->SetDeathFadeDuration(1.f);
    player->SetDeathFadeDelay(1.5f);
    break;
  case kFU_ExplodeDeath:
    ExplodeDeath(mgr, *player, kEDT_Normal, owner);
    break;
  case kFU_IceDeath:
    ExplodeDeath(mgr, *player, kEDT_Ice, owner);
    break;
  case kFU_LaggedBurnDeath:
    StartBurnDeath(mgr, *player, kBDT_Lagged);
    break;
  case kFU_BurnDeath:
  case kFU_ImmediateDisintegration:
    StartBurnDeath(mgr, *player, kBDT_Normal);
    break;
  default:
    break;
  }
}

void CPlayerKnockBackMgr::ApplyKnockBackEffects(CActor& actor, CStateManager& mgr,
                                                const CKnockBackInfo& info) {
  if (CPlayer* player = TCastToPtr< CPlayer >(&actor)) {
    if (player->GetFrozenState() && (mActiveParameters.mFlags & kRF_BreakFreeze)) {
      player->BreakFrozenState(mgr, CPlayer::kBFS_BreakWithEffects, false);
    }
    if (mBurnRemainingTime > 0.f && (mActiveParameters.mFlags & kRF_DouseFire)) {
      DouseFlames();
    }
    if (mElectrocutionRemainingTime > 0.f && (mActiveParameters.mFlags & kRF_StopElectrocution)) {
      DouseElectrocution();
    }
    if (mActiveParameters.mFlags & kRF_BreakOrbit) {
      player->SetOrbitRequestForTarget(player->GetOrbitTargetId(), CPlayer::kOR_KnockBack, mgr);
    }
  }
}

void CPlayerKnockBackMgr::StartBlackHoleDeath(CStateManager& mgr, TUniqueId source,
                                              CPlayer& player) {
  if (const CActor* sourceActor = TCastToConstPtr< CActor >(mgr.GetObjectById(source))) {
    const CVector3f position = sourceActor->GetTranslation();
    mImploding = true;
    StartBurnDeath(mgr, player, kBDT_Normal);
    CActorModelParticles& particles = *mgr.ActorModelParticles();
    particles.StopFire(player);
    particles.StartBurnDeath(player, mgr);
    particles.StartImplosion(player, position, true);
  }
}

void CPlayerKnockBackMgr::UpdateImplosion(CStateManager& mgr, CPlayer& player) {
  if (mImploding && player.GetDeathTime() > 1.5f) {
    mgr.ActorModelParticles()->StopImplosion(player);
  }
}

bool CPlayerKnockBackMgr::WasOnGround() const { return mWasOnGround; }

bool CPlayerKnockBackMgr::WasFrozen() const { return mWasFrozen; }

bool CPlayerKnockBackMgr::IsBall() const { return mWasBall; }
