#include "MetroidPrime/CSafeZoneManager.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/pair.hpp"

CSafeZoneManager::SZone::SZone(const TUniqueId& id, const CVector3f& position,
                               const CVector3f& halfExtents, float scaleFactor)
: mId(id), mPosition(position), mHalfExtents(halfExtents), mScaleFactor(scaleFactor) {}

CSafeZoneManager::CSafeZoneManager() {}

int CSafeZoneManager::FindSafeZone(const TUniqueId& id) const {
  for (int i = 0; i < mZones.size(); ++i) {
    if (mZones[i].mId == id) {
      return i;
    }
  }
  return -1;
}

void CSafeZoneManager::AddOrUpdateSafeZone(CStateManager& mgr, const TUniqueId& id,
                                           const CVector3f& position, const CVector3f& halfExtents,
                                           float scaleFactor) {
  const int index = FindSafeZone(id);
  if (index == -1) {
    // Native noise value 5 has no established semantic enumerator in this header.
    mgr.InformListeners(position, static_cast< EListenNoiseType >(5));
    const SZone zone(id, position, halfExtents, scaleFactor);
    if (mZones.size() < mZones.capacity()) {
      mZones.push_back(zone);
    }
  } else {
    SZone& zone = mZones[index];
    zone.mPosition = position;
    zone.mHalfExtents = halfExtents;
    zone.mScaleFactor = scaleFactor;
  }
}

void CSafeZoneManager::RemoveSafeZone(const TUniqueId& id) {
  const int index = FindSafeZone(id);
  if (index != -1) {
    mZones.erase(mZones.begin() + index);
  }
}

void CSafeZoneManager::Render(CStateManager& mgr) const {
  rstl::reserved_vector< rstl::pair< float, CScriptSafeZone* >, 64 > visibleZones;
  const CVector3f forward = CGraphics::GetViewMatrix().GetForward();
  for (int i = 0; i < rstl::min_val(mZones.capacity(), mZones.size()); ++i) {
    CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(mZones[i].mId)));
    if (zone && mgr.IsActorVisible(*zone) &&
        mgr.GetWorld()->GetAreaAlways(zone->GetCurrentAreaId()).GetOcclusionState() ==
            CGameArea::kOS_Visible) {
      visibleZones.push_back(rstl::pair< float, CScriptSafeZone* >(
          CVector3f::Dot(forward, mZones[i].mPosition), zone));
    }
  }

  rstl::sort_by_key(visibleZones);
  for (int i = 0; i < visibleZones.size(); ++i) {
    SafeZone_ApplyRenderEffect(*visibleZones[i].second, mgr);
  }
}

bool CSafeZoneManager::IsObjectInSafeZone(const CActor& actor, const CStateManager& mgr) const {
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(it->mId)));
    if (zone && zone->HasInhabitant(actor.GetUniqueId())) {
      return true;
    }
  }
  return false;
}

bool CSafeZoneManager::IsObjectInHurtfulSafeZone(const CActor& actor,
                                                 const CStateManager& mgr) const {
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(it->mId)));
    if (zone && zone->IsHurtful() && zone->HasInhabitant(actor.GetUniqueId())) {
      return true;
    }
  }
  return false;
}

bool CSafeZoneManager::PointIsInSafeZone(const CStateManager& mgr, const CVector3f& point) const {
  return PointIsInWhichSafeZone(mgr, point) != kInvalidUniqueId;
}

bool CSafeZoneManager::PointIsInHurtfulSafeZone(const CStateManager& mgr,
                                                const CVector3f& point) const {
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(it->mId)));
    if (zone && zone->IsHurtful() && zone->IsPointInside(point)) {
      return true;
    }
  }
  return false;
}

TUniqueId CSafeZoneManager::PointIsInWhichSafeZone(const CStateManager& mgr,
                                                   const CVector3f& point) const {
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(it->mId)));
    if (zone && zone->IsPointInside(point)) {
      return zone->GetUniqueId();
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CSafeZoneManager::SphereTouchingWhichSafeZone(const CStateManager& mgr,
                                                        const CSphere& sphere) const {
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CScriptSafeZone* zone =
        TCastToPtr< CScriptSafeZone >(const_cast< CEntity* >(mgr.GetObjectById(it->mId)));
    if (zone) {
      const CVector3f delta = sphere.GetCenter() - zone->GetTranslation();
      const float radius = zone->GetScale().GetX() + sphere.GetRadius();
      if (delta.MagSquared() < radius * radius) {
        return zone->GetUniqueId();
      }
    }
  }
  return kInvalidUniqueId;
}

void CSafeZoneManager::Update(float, CStateManager& mgr) {
  int i = 0;
  while (i < mZones.size()) {
    if (!mgr.GetObjectById(mZones[i].mId)) {
      mZones.erase(mZones.begin() + i);
    } else {
      ++i;
    }
  }
}

float CSafeZoneManager::GetDarkWorldFilterAmount(const CTransform4f& cameraTransform) const {
  float closest = 1.f;
  const CVector3f position = cameraTransform.GetTranslation();
  const CVector3f forward = cameraTransform.GetForward();
  for (rstl::reserved_vector< SZone, 64 >::const_iterator it = mZones.begin(); it != mZones.end();
       ++it) {
    const CVector3f delta = it->mPosition - position;
    if (!delta.CanBeNormalized()) {
      continue;
    }
    const float distance = delta.Magnitude();
    const float surfaceDistance = distance - it->mHalfExtents.GetX();
    if (surfaceDistance < 0.f || surfaceDistance > 10.f) {
      continue;
    }
    const float cosAngle = CVector3f::Dot(forward, delta) / distance;
    const float projection = cosAngle * surfaceDistance / 10.f;
    if (projection >= 0.f && projection < closest) {
      closest = projection;
    }
  }
  return 0.7f * (0.5f * closest + 0.5f);
}
