#ifndef _CBODYCONTROLLER
#define _CBODYCONTROLLER

#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"

class CActor;
class CAnimPlaybackParms;
class CPASAnimParmData;
class CPASDatabase;
class CRandom16;
class CStateManager;

class CBodyController {
public:
  CBodyController(CActor& actor, float turnSpeed, EBodyType bodyType);
  ~CBodyController() {}

  void Activate(CStateManager& mgr, pas::EAnimationState state);
  void Update(float dt, CStateManager& mgr);
  bool HasBodyState(pas::EAnimationState state) const;
  pas::EFallState GetFallState() const;
  void SetFallState(pas::EFallState state);
  void UpdateBody(float dt, CStateManager& mgr);
  void SetLocomotionType(pas::ELocomotionType type);
  void AbortScriptedAnimations();
  void SetTurnSpeed(float speed);
  void EnableAnimation(bool enable);
  void SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop, bool noTrans);
  float GetAnimTimeRemaining() const;
  void SetPlaybackRate(float rate);
  void MultiplyPlaybackRate(float scale);
  void SetDeltaRotation(const CQuaternion& rotation);
  void FaceDirection(const CVector3f& direction, float dt);
  // Guessed name
  void FaceDirectionOnSurface(const CVector3f& direction, const CVector3f& currentDirection,
                              float dt);
  void FaceDirection3D(const CVector3f& direction, const CVector3f& currentDirection, float dt);
  const CPASDatabase& GetPASDatabase() const;
  void PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& random);
  void LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& random);
  void Freeze(float intoFreezeDuration, float frozenDuration, float breakoutDuration);
  void FrozenBreakout();
  void UnFreeze();
  float GetPercentageFrozen() const;
  void SetOnFire(float duration);
  void DouseFlames();
  void SetElectrocuting(float duration);
  void DouseElectrocuting();
  void UpdateFrozenInfo(float dt, CStateManager& mgr);
  bool HasIceBreakoutState();

  CActor& GetOwner() const { return *mActor; }

  CBodyStateCmdMgr& CommandMgr() { return mCmdMgr; }

  const CBodyStateCmdMgr& GetCommandMgr() const { return mCmdMgr; }

  CBodyStateInfo& BodyStateInfo() { return mBodyStateInfo; }

  const CBodyStateInfo& GetBodyStateInfo() const { return mBodyStateInfo; }

  pas::EAnimationState GetCurrentStateId() const { return mBodyStateInfo.GetCurrentStateId(); }

  pas::ELocomotionType GetLocomotionType() const { return mLocomotionType; }

  EBodyType GetBodyType() const { return mBodyType; }

  int GetCurrentAnimId() const { return mCurAnim; }

  float GetRestrictedFlyerMoveSpeed() const { return mRestrictedFlyerMoveSpeed; }
  void SetRestrictedFlyerMoveSpeed(float speed) { mRestrictedFlyerMoveSpeed = speed; }

  float GetTimeScale() const { return mTimeScale; } // Guessed name

  void SetTimeScale(float scale) { mTimeScale = scale; } // Guessed name

  float GetFireDamageBuildup() const { return mFireDamageBuildup; } // Guessed name.

  void SetFireDamageBuildup(float buildup) { mFireDamageBuildup = buildup; } // Guessed name.

  bool IsAnimationOver() const { return mAnimationOver; }

  bool GetIsActive() const { return mActive; }

  bool IsFrozen() const { return mFrozen; }

  bool HasBeenFrozen() const { return mHasBeenFrozen; }

  bool ShouldPlayDeathAnims() const { return mPlayDeathAnims; }

  bool IsOnFire() const { return mFireDur > 0.f; }

  bool IsElectrocuting() const { return mElectrocutionDur > 0.f; }

private:
  CBodyController(const CBodyController&);
  CBodyController& operator=(const CBodyController&);

  CActor* mActor;
  CBodyStateCmdMgr mCmdMgr;
  CBodyStateInfo mBodyStateInfo;
  CQuaternion mRot;
  pas::ELocomotionType mLocomotionType;
  pas::EFallState mFallState;
  EBodyType mBodyType;
  int mCurAnim;
  float mTurnSpeed;
  bool mAnimationOver : 1;
  bool mActive : 1;
  bool mFrozen : 1;
  bool mHasBeenFrozen : 1;
  bool mPlayDeathAnims : 1;
  float mIntoFreezeDur;
  float mFrozenDur;
  float mBreakoutDur;
  float mTimeFrozen;
  CVector3f mBackedUpForce;
  float mFireDur;
  float mElectrocutionDur;
  float mTimeOnFire;
  float mTimeElectrocuting;
  float mRestrictedFlyerMoveSpeed;
  float mTimeScale;         // Guessed name
  float mFireDamageBuildup; // Guessed name
};
CHECK_SIZEOF(CBodyController, 0x5d0)

#endif // _CBODYCONTROLLER
