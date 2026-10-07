#include "MetroidPrime/CMissileRepeller.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "rstl/algorithm.hpp"

CMissileRepeller::CMissileRepeller(float radius, float deflectionRate, ushort soundId,
                                   float spaceWarpStrength, const CVector3f& offset)
: mRadius(radius)
, mDeflectionRate(deflectionRate)
, mSoundId(soundId)
, mSpaceWarpStrength(spaceWarpStrength)
, mOffset(offset)
, mDeflectedProjectiles(kInvalidUniqueId)
, mSoundHandle()
, mSoundTime(0.f)
, mActive(true) {}

CMissileRepeller::~CMissileRepeller() {}

void CMissileRepeller::Update(CStateManager& mgr, const CActor& actor, float dt) {
  if (CSfxManager::IsPlaying(mSoundHandle)) {
    mSoundTime += dt;
    CSfxManager::UpdateEmitter(mSoundHandle, actor.GetTranslation(), CVector3f::Zero(), 127);
  }

  if (!mActive) {
    return;
  }

  const CVector3f center = actor.GetAimPosition(mgr, 0.f) + mOffset;
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(
      nearList, CAABox(center - mRadius * CVector3f::One(), center + mRadius * CVector3f::One()),
      filter, &actor);

  const rstl::reserved_vector< TUniqueId, 10 > previousProjectiles = mDeflectedProjectiles;
  mDeflectedProjectiles.clear();
  if (nearList.size() == 0) {
    return;
  }

  const float radiusSq = mRadius * mRadius;
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    CGameProjectile* projectile = TCastToPtr< CGameProjectile >(mgr.ObjectById(*it));
    if (!projectile ||
        !(projectile->GetType() == kWT_Missile ||
          (projectile->GetType() == kWT_Power && projectile->HasAttrib(CWeapon::kPA_ComboShot)))) {
      continue;
    }

    const CVector3f delta = projectile->GetTranslation() - center;
    if (delta.MagSquared() < radiusSq) {
      mDeflectedProjectiles.push_back(*it);
      projectile->SetMinHomingDistance(mRadius);

      CProjectileWeapon& weapon = projectile->Projectile();
      const CVector3f dir = delta + (projectile->GetTranslation() - projectile->GetPreviousPos());
      const CVector3f axis = CVector3f::Cross(dir, delta);
      if (axis.CanBeNormalized()) {
        const CQuaternion rotation = CQuaternion::AxisAngle(
            CUnitVector3f(axis), CRelAngle::FromDegrees(dt * mDeflectionRate));
        weapon.SetWorldSpaceOrientation(rotation.BuildTransform4f() *
                                        weapon.GetTransform().GetRotation());
      }
    }
  }

  for (rstl::reserved_vector< TUniqueId, 10 >::const_iterator it = mDeflectedProjectiles.begin();
       it != mDeflectedProjectiles.end(); ++it) {
    if (rstl::find(previousProjectiles.begin(), previousProjectiles.end(), *it) !=
        previousProjectiles.end()) {
      continue;
    }

    bool playSound = true;
    if (CSfxManager::IsPlaying(mSoundHandle)) {
      if (mSoundTime > 0.5f) {
        CSfxManager::SfxStop(mSoundHandle);
      } else {
        playSound = false;
      }
    }
    if (playSound) {
      mSoundHandle = CSfxManager::AddEmitter(mSoundId, actor.GetTranslation(), 127,
                                             actor.GetCurrentAreaId().Value(), true, false,
                                             CSfxManager::kMedPriority);
      mSoundTime = 0.f;
    }
    break;
  }
}

void CMissileRepeller::Render(const CStateManager& mgr, const CActor& actor) const {
  if (mDeflectedProjectiles.size() == 0) {
    return;
  }

  const CVector3f center = actor.GetAimPosition(mgr, 0.f) + mOffset;
  for (rstl::reserved_vector< TUniqueId, 10 >::const_iterator it = mDeflectedProjectiles.begin();
       it != mDeflectedProjectiles.end(); ++it) {
    const CGameProjectile* projectile = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(*it));
    if (projectile) {
      const float distance = (center - projectile->GetTranslation()).Magnitude();
      const float ratio = distance / mRadius;
      mgr.DrawSpaceWarp(projectile->GetTranslation(),
                        mSpaceWarpStrength * (1.f - rstl::min_val(ratio, 1.f)));
    }
  }
}
