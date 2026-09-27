#include "MetroidPrime/BodyState/CBSLocomotion.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

bool CBSFlyerLocomotion::IsPitchable() const { return mPitchable; }

CBSFlyerLocomotion::~CBSFlyerLocomotion() {}

CBSWallWalkerLocomotion::~CBSWallWalkerLocomotion() {}

CBSAiMovedFlyerLocomotion::~CBSAiMovedFlyerLocomotion() {}

float CBSRestrictedLocomotion::GetLocomotionSpeed(pas::ELocomotionType type,
                                                  pas::ELocomotionAnim anim) const {
  return 0.f;
}

bool CBSRestrictedLocomotion::IsMoving() const { return false; }

CBSFloaterLocomotion::~CBSFloaterLocomotion() {}

CBSLocomotion::CBSLocomotion() : mLocomotionType(pas::kLT_Invalid) {}

void CBSLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  mLocomotionType = bc.GetLocomotionType();
  ReStartBodyState(bc, bc.CommandMgr().GetCmd(kBSC_MaintainVelocity) != nullptr);
}

pas::EAnimationState CBSLocomotion::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    const float velocity = ApplyLocomotionPhysics(dt, bc);
    UpdateLocomotionAnimation(dt, velocity, bc, false);
  }
  return state;
}

void CBSLocomotion::Shutdown(CBodyController& bc) { bc.MultiplyPlaybackRate(1.f); }

float CBSLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  // TODO: Recover facing and pitch-limited movement; nonpitchable speed uses the XY projection.
  return 0.f;
}

void CBSLocomotion::ReStartBodyState(CBodyController& bc, bool maintainVel) {
  UpdateLocomotionAnimation(0.f, maintainVel ? GetStartVelocityMagnitude(bc) : 0.f, bc, true);
}

float CBSLocomotion::GetStartVelocityMagnitude(CBodyController& bc) const {
  // TODO: Normalize the owner's velocity by the selected locomotion state's maximum speed.
  return 0.f;
}

float CBSLocomotion::ComputeWeightPercentage(const rstl::pair< int, float >& a,
                                             const rstl::pair< int, float >& b,
                                             float velocity) const {
  const float range = b.second - a.second;
  return range > FLT_EPSILON ? CMath::Clamp(0.f, (velocity - a.second) / range, 1.f) : 0.f;
}

pas::EAnimationState CBSLocomotion::GetBodyStateTransition(float dt, CBodyController& bc) {
  // TODO: Recover the ordered command priorities using Echoes's command and state enums.
  return pas::kAS_Invalid;
}

CBSBiPedLocomotion::CBSBiPedLocomotion(CActor& actor)
: mAnims(15,
         rstl::reserved_vector< rstl::pair< int, float >, 8 >(8, rstl::pair< int, float >(0, 0.f)))
, mAnim(pas::kLA_Invalid) {
  // TODO: Resolve the 15-by-8 PAS animation table and each non-idle animation's average velocity.
}

float CBSBiPedLocomotion::GetLocomotionSpeed(pas::ELocomotionType type,
                                             pas::ELocomotionAnim anim) const {
  return GetLocoAnimation(type, anim).second;
}

void CBSBiPedLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  mPrimeTime = 0.f;
  CBSLocomotion::Start(bc, mgr);
}

pas::EAnimationState CBSBiPedLocomotion::UpdateBody(float dt, CBodyController& bc,
                                                    CStateManager& mgr) {
  if (mPrimeTime < 0.2f) {
    mPrimeTime += dt;
  }
  return CBSLocomotion::UpdateBody(dt, bc, mgr);
}

float CBSBiPedLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc,
                                                    bool init) {
  // TODO: Select idle, walk, run or strafe while respecting the animation-change delay.
  return 1.f;
}

const rstl::pair< int, float >&
CBSBiPedLocomotion::GetLocoAnimation(pas::ELocomotionType type, pas::ELocomotionAnim anim) const {
  return mAnims[type][anim];
}

bool CBSBiPedLocomotion::IsStrafing(CBodyController& bc) const {
  return !close_enough(bc.GetCommandMgr().GetMoveVector(), CVector3f::Zero()) &&
         !close_enough(bc.GetCommandMgr().GetFaceVector(), CVector3f::Zero());
}

float CBSBiPedLocomotion::UpdateStrafe(float velocity, CBodyController& bc,
                                       pas::ELocomotionAnim anim) {
  // TODO: Select a local-space strafe animation and scale its playback rate.
  return 1.f;
}

float CBSBiPedLocomotion::UpdateWalk(float velocity, CBodyController& bc,
                                     pas::ELocomotionAnim anim) {
  // TODO: Select the walk animation and compute its playback weight from the idle/walk speeds.
  return 1.f;
}

float CBSBiPedLocomotion::UpdateRun(float velocity, CBodyController& bc,
                                    pas::ELocomotionAnim anim) {
  // TODO: Select walk/run around the target blend threshold and scale playback.
  return 1.f;
}

CBSRestrictedLocomotion::CBSRestrictedLocomotion(CActor& actor)
: mAnims(15, -1), mAnim(pas::kLA_Invalid) {
  // TODO: Resolve the idle animation for each of Echoes's 15 locomotion types.
}

float CBSRestrictedLocomotion::UpdateLocomotionAnimation(float dt, float velMag,
                                                         CBodyController& bc, bool init) {
  const pas::ELocomotionAnim anim = init ? pas::kLA_Invalid : mAnim;
  if (anim != pas::kLA_Idle) {
    const int newAnim = mAnims[mLocomotionType];
    if (newAnim != bc.GetCurrentAnimId()) {
      const CAnimPlaybackParms parms(newAnim, -1, 1.f, true);
      bc.SetCurrentAnimation(parms, true, false);
    }
    mAnim = pas::kLA_Idle;
  }
  return 1.f;
}

CBSFlyerLocomotion::CBSFlyerLocomotion(CActor& actor, bool pitchable)
: CBSBiPedLocomotion(actor), mPitchable(pitchable) {}

float CBSFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  // TODO: Apply base locomotion and the restricted vertical impulse.
  return 0.f;
}

CBSWallWalkerLocomotion::CBSWallWalkerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSWallWalkerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  // TODO: Recover surface-relative facing, impulse and normalized movement speed.
  return 0.f;
}

CBSAiMovedFlyerLocomotion::CBSAiMovedFlyerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSAiMovedFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  bc.FaceDirection(bc.CommandMgr().GetFaceVector(), dt);
  return 0.f;
}

float CBSAiMovedFlyerLocomotion::UpdateLocomotionAnimation(float dt, float velMag,
                                                           CBodyController& bc, bool init) {
  // TODO: Select the dominant local-space movement axis and its directional animation.
  return 1.f;
}

CBSRestrictedLocomotion::~CBSRestrictedLocomotion() {}

CBSFloaterLocomotion::CBSFloaterLocomotion(CActor& actor) : CBSRestrictedLocomotion(actor) {}

float CBSFloaterLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  // TODO: Face the commanded direction and apply the mass-scaled restricted-flyer impulse.
  return 0.f;
}

CBSBiPedLocomotion::~CBSBiPedLocomotion() {}

CBSBlendedLocomotion::CBSBlendedLocomotion(CActor& actor, float turnSpeed)
: CBSBiPedLocomotion(actor), mDirection(0.f, 1.f, 0.f), mTurnSpeed(turnSpeed), mTimeMoving(0.f) {}

float CBSBlendedLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    bc.FaceDirection(bc.CommandMgr().GetFaceVector(), dt);
  }
  return 0.f;
}

float CBSBlendedLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc,
                                                      bool init) {
  // TODO: Turn the cached local direction and blend forward/backward with lateral animations.
  return 1.f;
}

void CBSBlendedLocomotion::ReStartBodyState(CBodyController& bc, bool maintainVel) {
  mTimeMoving = 0.f;
  CBSLocomotion::ReStartBodyState(bc, maintainVel);
}

bool CBSLocomotion::IsPitchable() const { return false; }

bool CBSLocomotion::CanShoot() const { return true; }

bool CBSBiPedLocomotion::IsMoving() const { return mAnim != pas::kLA_Idle; }

CBSBlendedLocomotion::~CBSBlendedLocomotion() {}
