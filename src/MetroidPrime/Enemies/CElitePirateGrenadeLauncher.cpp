#include "MetroidPrime/Enemies/CElitePirateGrenadeLauncher.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include <math.h>

static rstl::string skRELName = rstl::string_l("ElitePirate.rel");
static const int skLauncherAnims[] = {0, 3};
static const char* const skGrenadeLocator = "grenade_LCTR";
static const char* const skLockOnLocator = "lockon_target_LCTR";
static const float skMaxWeight = 0.5f;

CElitePirateGrenadeLauncher::CElitePirateGrenadeLauncher(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CActorParameters& actParams, TUniqueId parentId, float f)
: CActor(uid, name, info, 0, xf, mData, CMaterialList(), actParams, kInvalidUniqueId)
, mPlayerIndex(0)
, mStarted(0)
, mParentId(parentId)
, x164_(-1.f)
, x168_(1.f, 1.f, 1.f, 1.f)
, mGrenadeActorParams(actParams)
, mYaw(0.f)
, mYawVelocity(0.f)
, mPitch(0.f)
, mPitchVelocity(0.f)
, mSlowedSpeed(1.f)
, mThermalMag(0.f)
, mDamageColor(0.5f, 0.f, 0.f, 1.f)
, mDamageAddColor(0.f, 0.f, 0.f, 1.f)
, x1fc_(f)
, mLaunchGrenade(false)
, mVisible(true)
, mFollowPlayer(true)
, mTurretTransform(CTransform4f::Identity())
, mRelToken(skRELName, 0) {
  ModelData()->EnableLooping(true);
  for (int i = 0; i < 4; ++i) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(i),
                                 CPASAnimParm::FromEnum(0));
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    mAnimIds[i] = anim.second;
  }
}

CElitePirateGrenadeLauncher::~CElitePirateGrenadeLauncher() {}

void CElitePirateGrenadeLauncher::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (mLaunchGrenade) {
      LaunchGrenadeProjectile(mgr);
      mLaunchGrenade = false;
    }
    UpdateGunTracking(dt, mgr);
    UpdateAnimation(dt, mgr, true);
  }
}

void CElitePirateGrenadeLauncher::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const TUniqueId sender = msg.GetUnk();
  const EScriptObjectMessage kind = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (kind) {
  case kSM_Create:
    UpdateLauncherAnimation();
    break;
  case kSM_Start:
    if (sender == mParentId && mStarted != 1) {
      mStarted = 1;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Stop:
    if (sender == mParentId && mStarted != 0) {
      mStarted = 0;
      UpdateLauncherAnimation();
    }
    break;
  case kSM_Action:
    if (sender == mParentId && mStarted == 1) {
      mLaunchGrenade = true;
    }
    break;
  default:
    break;
  }
}

void CElitePirateGrenadeLauncher::Render(const CStateManager& mgr) const {
  if (mVisible) {
    CActor::Render(mgr);
  }
}

void CElitePirateGrenadeLauncher::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  mTurretTransform = GetTransform() * GetScaledLocatorTransform(rstl::string_l("Turret_SDK"));
}

void CElitePirateGrenadeLauncher::AddToRenderer(const CStateManager& mgr) const {
  CActor::AddToRenderer(mgr);
}

void CElitePirateGrenadeLauncher::UpdateLauncherAnimation() {
  if (HasAnimation() && mStarted >= 0 && mStarted <= 1) {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(skLauncherAnims[mStarted]));
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    if (anim.first > 0.f) {
      AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
    }
  }
}

void CElitePirateGrenadeLauncher::LaunchGrenadeProjectile(CStateManager& mgr) {
  if (mgr.IsSkippingCinematic() && HasAnimation()) {
    const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
    const rstl::pair< float, int > anim =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (anim.first > 0.f) {
      AnimationData()->AddAdditiveAnimation(anim.second, 1.f, false, true);
      const CTransform4f locator = GetLocatorTransform(rstl::string_l(skGrenadeLocator));
      const CVector3f origin = GetTranslation() + GetTransform().Rotate(locator.GetTranslation());
    }
  }
}

bool CElitePirateGrenadeLauncher::IsNearHint(const CStateManager& mgr, const CVector3f& pos) const {
  const CObjectList& list = mgr.GetObjectListById(kOL_AiWaypoint);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CScriptAIHint* hint = TCastToConstPtr< CScriptAIHint >(list[i]);
    if (hint != nullptr && hint->GetCurrentAreaId() == GetCurrentAreaId() &&
        hint->GetActive() == true && hint->GetHintType() == 0x17) {
      const CVector3f delta = CVector3f(hint->GetTranslation().GetX() - pos.GetX(),
                                        hint->GetTranslation().GetY() - pos.GetY(),
                                        hint->GetTranslation().GetZ() - pos.GetZ());
      const float distance = CVector3f(delta).Magnitude();
      if (distance < hint->GetRadius()) {
        return true;
      }
    }
  }
  return false;
}

void CElitePirateGrenadeLauncher::UpdateGunTracking(float dt, CStateManager& mgr) {
  if (HasAnimation() && mStarted == 1 && IsTrackingPlayer()) {
    const float speed = dt * mSlowedSpeed;
    const CVector3f aimPosition = mgr.GetPlayer(mPlayerIndex)->GetAimPosition(mgr, 0.f);
    float targetZ;
    if (IsNearHint(mgr, mgr.GetPlayer(mPlayerIndex)->GetTranslation()) == true) {
      targetZ = aimPosition.GetZ() + 5.f;
    } else {
      targetZ = GetTranslation().GetZ();
    }
    const CVector3f position = GetTransform().GetTranslation();
    const CVector3f targetDelta =
        CVector3f(aimPosition.GetX() - position.GetX(), aimPosition.GetY() - position.GetY(),
                  targetZ - position.GetZ());
    // The explicit copy preserves the original vector temporary layout.
    const CVector3f localTarget = GetTransform().TransposeRotate(CVector3f(targetDelta));
    if (localTarget.CanBeNormalized()) {
      const float maxAngle = M_PIF / 4.f;
      const float maxSpeed = 3.f;
      const float maxAcceleration = 10.f;
      float yaw = atan2f(localTarget.GetX(), localTarget.GetY());
      yaw = CMath::Clamp(-maxAngle, yaw, maxAngle);
      yaw = (2.f / M_PIF) * yaw;
      float velocity = (0.25f * (yaw - mYaw)) / speed;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      float acceleration = (velocity - mYawVelocity) / speed;
      mYawVelocity += speed * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      float pitch =
          atan2f(localTarget.GetZ(), CMath::SqrtF(localTarget.GetY() * localTarget.GetY() +
                                                  localTarget.GetX() * localTarget.GetX()));
      pitch = CMath::Clamp(-maxAngle, pitch, maxAngle);
      pitch = (2.f / M_PIF) * pitch;
      velocity = (0.25f * (pitch - mPitch)) / speed;
      velocity = CMath::Clamp(-maxSpeed, velocity, maxSpeed);
      acceleration = (velocity - mPitchVelocity) / speed;
      mPitchVelocity += speed * CMath::Clamp(-maxAcceleration, acceleration, maxAcceleration);
      const float nextYaw = CMath::Clamp(-skMaxWeight, speed * mYawVelocity + mYaw, skMaxWeight);
      const float nextPitch =
          CMath::Clamp(-skMaxWeight, speed * mPitchVelocity + mPitch, skMaxWeight);
      CAnimData* animData = AnimationData();
      if (nextYaw != mYaw) {
        float weight = CMath::AbsF(nextYaw);
        if (CMath::AbsF(mYaw) > 0.f && mYaw * nextYaw <= 0.f) {
          animData->DelAdditiveAnimation(mAnimIds[mYaw < 0.f ? 0 : 1]);
        }
        if (weight > 0.f) {
          animData->AddAdditiveAnimation(mAnimIds[nextYaw < 0.f ? 0 : 1], weight, false, false);
        }
      }
      if (nextPitch != mPitch) {
        float weight = CMath::AbsF(nextPitch);
        if (CMath::AbsF(mPitch) > 0.f && mPitch * nextPitch <= 0.f) {
          animData->DelAdditiveAnimation(mAnimIds[mPitch > 0.f ? 2 : 3]);
        }
        if (weight > 0.f) {
          animData->AddAdditiveAnimation(mAnimIds[nextPitch > 0.f ? 2 : 3], weight, false, false);
        }
      }
      mYaw = nextYaw;
      mPitch = nextPitch;
    }
  } else {
    CAnimData* animData = AnimationData();
    if (mYaw != 0.f) {
      animData->DelAdditiveAnimation(mAnimIds[mYaw < 0.f ? 0 : 1]);
      mYaw = 0.f;
    }
    if (mPitch != 0.f) {
      animData->DelAdditiveAnimation(mAnimIds[mPitch > 0.f ? 2 : 3]);
      mPitch = 0.f;
    }
  }
}

void CElitePirateGrenadeLauncher::SetSlowedSpeed(float speed) { mSlowedSpeed = speed; }

bool CElitePirateGrenadeLauncher::IsTrackingPlayer() const {
  if (mSlowedSpeed == 0.f) {
    return false;
  }
  return mFollowPlayer;
}
