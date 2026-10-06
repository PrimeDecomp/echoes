#include "MetroidPrime/Weapons/CAnnihilatorProjectile.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"

const CMaterialFilter CAnnihilatorProjectile::kTargetFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_SeekerTarget), CMaterialList(kMT_NoPlatformCollision, kMT_Trigger));
const CMaterialFilter CAnnihilatorProjectile::kRayFilter = CMaterialFilter::MakeIncludeExclude(
    CMaterialList(kMT_Unknown59), CMaterialList(kMT_NoPlatformCollision));
float CAnnihilatorProjectile::sNextTargetSeekOffset = 0.f;

CAnnihilatorProjectile::CAnnihilatorProjectile(
    const TToken< CWeaponDescription >& description, EWeaponType type, const CTransform4f& xf,
    EMaterialTypes excludeMaterial, const CDamageInfo& damage, TUniqueId uid, TAreaId areaId,
    TUniqueId owner, TUniqueId homingTarget, uint attributes, bool underwater,
    const CVector3f& scale, float projectileSpeed, float projectileTurnRate)
: CEnergyProjectile(true, description, type, xf, excludeMaterial, damage, uid, areaId, owner,
                    homingTarget, attributes, underwater, scale, CImpactVisorEffect(), false, true,
                    false, 1.f, 4.f, 4.f)
, mTargetSeekTimer(sNextTargetSeekOffset)
, mProjectileSpeed(projectileSpeed)
, mProjectileTurnRate(projectileTurnRate) {
  sNextTargetSeekOffset += 1.f / 60.f;
  if (sNextTargetSeekOffset > 1.f / 6.f) {
    sNextTargetSeekOffset = 0.f;
  }
}

CAnnihilatorProjectile::~CAnnihilatorProjectile() {}

void CAnnihilatorProjectile::Think(float dt, CStateManager& mgr) {
  if (mActive && !HasExploded()) {
    if (mHomingTargetId != kInvalidUniqueId) {
      bool clearTarget = false;
      const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(mHomingTargetId));
      if (target != nullptr) {
        if (!target->GetMaterialList().HasMaterial(kMT_SeekerTarget)) {
          clearTarget = true;
        } else if (mgr.IsMultiplayer()) {
          clearTarget = !CanTargetActor(mgr, mOwnerId, *target);
        }
      } else {
        clearTarget = true;
      }
      if (clearTarget) {
        mHomingTargetId = kInvalidUniqueId;
      }
    }
    if (mHomingTargetId == kInvalidUniqueId) {
      mTargetSeekTimer += dt;
      if (mTargetSeekTimer > 0.125f) {
        mTargetSeekTimer = 0.f;
        rstl::vector< STargetCandidate > candidates;
        GatherTargetCandidates(mgr, candidates, *this, GetTransform(), mOwnerId, nullptr,
                               mProjectileSpeed, mProjectileTurnRate, 30.f, 30.f, 999.f);
        if (candidates.size() != 0) {
          mHomingTargetId = candidates[0].mActor->GetUniqueId();
        }
      }
    }
  }
  CEnergyProjectile::Think(dt, mgr);
}

void CAnnihilatorProjectile::GatherTargetCandidates(
    CStateManager& mgr, rstl::vector< STargetCandidate >& candidates, const CActor& source,
    const CTransform4f& xf, TUniqueId owner, const CGameCamera* projection, float projectileSpeed,
    float projectileTurnRate, float radius, float height, float turnTestDistance) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  CAABox box(CVector3f(-radius, 0.f, -radius), CVector3f(radius, height, radius));
  box = box.GetTransformedAABox(source.GetTransform());
  mgr.BuildNearList(nearList, box, kTargetFilter, &source);
  candidates.reserve(nearList.size());

  const bool multiplayer = mgr.IsMultiplayer();
  for (const TUniqueId* it = nearList.begin(); it != nearList.end(); ++it) {
    if (*it == owner) {
      continue;
    }
    const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it));
    if (actor == nullptr) {
      continue;
    }
    if (multiplayer && !CanTargetActor(mgr, owner, *actor)) {
      continue;
    }

    const CVector3f aimPosition = actor->GetAimPosition(mgr, 0.f);
    CVector3f screenPosition = CVector3f::Zero();
    if (projection != nullptr) {
      screenPosition = projection->ConvertToScreenSpace(aimPosition);
      if (CMath::AbsF(screenPosition.GetX()) > 1.f || CMath::AbsF(screenPosition.GetY()) > 1.f ||
          screenPosition.GetZ() >= 1.f) {
        continue;
      }
    }

    const CVector3f origin = xf.GetTranslation();
    CVector3f toTarget = aimPosition - origin;
    if (!toTarget.CanBeNormalized()) {
      continue;
    }
    const float distance = toTarget.Magnitude();
    toTarget.Normalize();
    if (distance < turnTestDistance) {
      const CVector3f forward = xf.GetColumn(kDY);
      float timeToTarget = distance * CVector3f::Dot(forward, toTarget) / projectileSpeed;
      if (timeToTarget <= 0.f) {
        continue;
      }
      timeToTarget = rstl::max_val(0.f, timeToTarget - 1.f);
      const float angle = CVector3f::GetAngleDiff(forward, toTarget);
      if (57.29578f * (2.f * angle) > projectileTurnRate * timeToTarget) {
        continue;
      }
    }

    if (!mgr.RayCollideWorld(origin, aimPosition, nearList, kRayFilter, actor)) {
      continue;
    }
    STargetCandidate candidate;
    candidate.mActor = const_cast< CActor* >(actor);
    candidate.mDistance = distance;
    candidate.mProjectedAimPosition = screenPosition;
    candidate.x14_ = distance;
    candidates.push_back_unsafe(candidate);
  }
}

bool CAnnihilatorProjectile::CanTargetActor(CStateManager& mgr, TUniqueId owner,
                                            const CActor& target) {
  const CPlayer* targetPlayer = TCastToConstPtr< CPlayer >(&target);
  if (targetPlayer != nullptr &&
      targetPlayer->GetPlayerState()->GetItemAmount(CPlayerState::kIT_Invisibility, true) != 0) {
    const CPlayer* ownerPlayer = TCastToConstPtr< CPlayer >(mgr.GetObjectById(owner));
    if (ownerPlayer != nullptr &&
        ownerPlayer->GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
      return true;
    }
    return false;
  }
  return true;
}
