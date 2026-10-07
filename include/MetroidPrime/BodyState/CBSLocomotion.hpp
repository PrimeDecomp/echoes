#ifndef _CBSLOCOMOTION
#define _CBSLOCOMOTION

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

namespace rstl {
typedef reserved_vector< pair< int, float >, 8 > TLocomotionAnimRow;
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(TLocomotionAnimRow)
} // namespace rstl

class CActor;

class CBSLocomotion : public CBodyState {
public:
  CBSLocomotion();

  // CBodyState
  ~CBSLocomotion() override {}
  bool IsMoving() const override = 0;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;

  virtual bool IsPitchable() const;
  virtual float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const = 0;
  virtual float ApplyLocomotionPhysics(float dt, CBodyController& bc);
  virtual float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc,
                                          bool init) = 0;
  virtual pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);
  virtual void ReStartBodyState(CBodyController& bc, bool maintainVel);

protected:
  float GetStartVelocityMagnitude(CBodyController& bc) const;
  float ComputeWeightPercentage(const rstl::pair< int, float >& a,
                                const rstl::pair< int, float >& b, float velocity) const;

  pas::ELocomotionType mLocomotionType;
};
CHECK_SIZEOF(CBSLocomotion, 0x8)

class CBSBiPedLocomotion : public CBSLocomotion {
public:
  explicit CBSBiPedLocomotion(CActor& actor);

  // CBodyState
  ~CBSBiPedLocomotion() override {}
  bool IsMoving() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;

  // CBSLocomotion
  float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;

  virtual bool IsStrafing(CBodyController& bc) const;

protected:
  float UpdateRun(float velocity, CBodyController& bc, pas::ELocomotionAnim anim);
  float UpdateWalk(float velocity, CBodyController& bc, pas::ELocomotionAnim anim);
  float UpdateStrafe(float velocity, CBodyController& bc, pas::ELocomotionAnim anim);
  const rstl::pair< int, float >& GetLocoAnimation(pas::ELocomotionType type,
                                                   pas::ELocomotionAnim anim) const;

  rstl::reserved_vector< rstl::reserved_vector< rstl::pair< int, float >, 8 >, 15 > mAnims;
  pas::ELocomotionAnim mAnim;
  float mPrimeTime;
};
CHECK_SIZEOF(CBSBiPedLocomotion, 0x410)

class CBSRestrictedLocomotion : public CBSLocomotion {
public:
  explicit CBSRestrictedLocomotion(CActor& actor);

  // CBodyState
  ~CBSRestrictedLocomotion() override {}
  bool IsMoving() const override;

  // CBSLocomotion
  float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;

private:
  rstl::reserved_vector< int, 15 > mAnims;
  pas::ELocomotionAnim mAnim;
};
CHECK_SIZEOF(CBSRestrictedLocomotion, 0x4c)

class CBSFlyerLocomotion : public CBSBiPedLocomotion {
public:
  CBSFlyerLocomotion(CActor& actor, bool pitchable);

  // CBodyState
  ~CBSFlyerLocomotion() override;

  // CBSLocomotion
  bool IsPitchable() const override;
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;

private:
  bool mPitchable;
};
CHECK_SIZEOF(CBSFlyerLocomotion, 0x414)

class CBSWallWalkerLocomotion : public CBSBiPedLocomotion {
public:
  explicit CBSWallWalkerLocomotion(CActor& actor);

  // CBodyState
  ~CBSWallWalkerLocomotion() override;

  // CBSLocomotion
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
};
CHECK_SIZEOF(CBSWallWalkerLocomotion, 0x410)

class CBSAiMovedFlyerLocomotion : public CBSBiPedLocomotion {
public:
  explicit CBSAiMovedFlyerLocomotion(CActor& actor);

  // CBodyState
  ~CBSAiMovedFlyerLocomotion() override;

  // CBSLocomotion
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;
};
CHECK_SIZEOF(CBSAiMovedFlyerLocomotion, 0x410)

class CBSFloaterLocomotion : public CBSRestrictedLocomotion {
public:
  explicit CBSFloaterLocomotion(CActor& actor);

  // CBodyState
  ~CBSFloaterLocomotion() override;

  // CBSLocomotion
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
};
CHECK_SIZEOF(CBSFloaterLocomotion, 0x4c)

// Guessed name: Echoes-specific directional animation-blending locomotion.
class CBSBlendedLocomotion : public CBSBiPedLocomotion {
public:
  CBSBlendedLocomotion(CActor& actor, float turnSpeed);

  // CBodyState
  ~CBSBlendedLocomotion() override;

  // CBSLocomotion
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;
  void ReStartBodyState(CBodyController& bc, bool maintainVel) override;

private:
  CVector3f mDirection;
  float mTurnSpeed;
  float mTimeMoving;
};
CHECK_SIZEOF(CBSBlendedLocomotion, 0x424)

#endif // _CBSLOCOMOTION
