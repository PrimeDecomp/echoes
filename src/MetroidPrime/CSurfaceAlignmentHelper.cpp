#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"

CVector3f CSurfaceAlignmentHelper::ProjectPointToPlane(const CVector3f& point,
                                                       const CVector3f& origin,
                                                       const CVector3f& normal) const {
  const CVector3f offset = point - origin;
  return point - normal * CVector3f::Dot(offset, normal);
}

bool CSurfaceAlignmentHelper::PointOnSurface(const CCollisionSurface& surface,
                                             const CVector3f& point) const {
  const CVector3f normal = surface.GetNormal();
  const CVector3f projected = ProjectPointToPlane(point, surface.GetVert(0), normal);
  const CVector3f surfaceNormal = surface.GetNormal();
  for (int i = 0; i < 3; ++i) {
    const CVector3f toPoint = projected - surface.GetVert(i);
    const CVector3f edge = surface.GetVert((i + 2) % 3) - surface.GetVert(i);
    if (CVector3f::Dot(surfaceNormal, CVector3f::Cross(toPoint, edge)) < 0.f) {
      return false;
    }
  }
  return true;
}

void CSurfaceAlignmentHelper::OrientToSurfaceNormal(CActor& actor, const CVector3f& normal,
                                                    float dt) const {
  const CVector3f up = actor.GetTransform().GetUp();
  const float dot = CVector3f::Dot(up, normal);
  if (!close_enough(dot, 1.f) && dot >= -0.999f) {
    const CRelAngle angle = CRelAngle::FromDegrees(mAngularRate * dt);
    const CQuaternion rotation = CQuaternion::ShortestRotationArcClamped(up, normal, angle);
    const CQuaternion localRotation = CQuaternion::ScalarVector(
        rotation.GetScalar(), actor.GetTransform().TransposeRotate(rotation.GetVector()));
    actor.SetRotation((actor.GetRotation() * localRotation).BuildNormalized());
  }
}

void CSurfaceAlignmentHelper::OrientToStoredSurface(CActor& actor, float dt) const {
  const CVector3f normal = mSurface.GetNormal();
  OrientToSurfaceNormal(actor, normal, dt);
}

void CSurfaceAlignmentHelper::AlignNearPosition(CActor& actor, CStateManager& mgr,
                                                const CVector3f& position, float dt) {
  if (FindNearestSurface(mgr, position, 1.8f, mSurface)) {
    OrientToStoredSurface(actor, dt);
  }
}

bool CSurfaceAlignmentHelper::FindNearestSurface(CStateManager& mgr, const CVector3f& position,
                                                 float radius, CCollisionSurface& surface) {
  bool found = false;
  if (!mCache.null()) {
    const CAABox bounds(position - CVector3f(radius, radius, radius),
                        position + CVector3f(radius, radius, radius));
    mCache->SetBounds(bounds);
    CGameCollision::BuildCollisionCache(mgr, *mCache, mFilter);

    float bestSqrDist = 2.f * radius * radius;
    CCollisionCacheIterator iterator(*mCache);
    const CCollisionCache* cache = mCache.get();
    for (uint i = 0; i < cache->GetNumTriangles(); ++i) {
      const CCollisionSurface& candidate = cache->NextTriangle(iterator)->GetSurface();
      if (mFilter.Passes(CMaterialList(candidate.GetSurfaceFlags()))) {
        const float sqrDist = CollisionUtil::TriPointSqrDist_Float(
            position, candidate.GetVert(0), candidate.GetVert(1), candidate.GetVert(2), nullptr,
            nullptr);
        if (sqrDist < bestSqrDist && PointOnSurface(candidate, position)) {
          bestSqrDist = sqrDist;
          found = true;
          surface = candidate;
        }
      }
    }
  }
  return found;
}

void CSurfaceAlignmentHelper::Update(CPhysicsActor& actor, CStateManager& mgr, float dt) {
  switch (mMode) {
  case kM_NearbySurface:
    AlignNearPosition(actor, mgr, actor.GetTranslation() + 2.f * (dt * actor.GetVelocityWR()), dt);
    break;
  case kM_WorldUp:
    OrientToSurfaceNormal(actor, CVector3f::Up(), dt);
    break;
  default:
    break;
  }
}

CSurfaceAlignmentHelper::CSurfaceAlignmentHelper(const CMaterialFilter& filter)
: mSurface(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), u64(-1))
, mFilter(filter)
, mAngularRate(180.f)
, mMode(kM_None) {}

CSurfaceAlignmentHelper::CSurfaceAlignmentHelper()
: mSurface(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), u64(-1))
, mFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59, kMT_Floor, kMT_Wall, kMT_Ceiling),
      CMaterialList(kMT_Character, kMT_Player, kMT_CollisionActor)))
, mAngularRate(180.f)
, mMode(kM_None)
, mCache(rs_new CCollisionCache(CAABox::MakeNullBox(), 2, 2, 0xFFFF)) {}
