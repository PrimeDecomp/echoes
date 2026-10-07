#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"

#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

CSurfaceAlignmentHelper::CSurfaceAlignmentHelper()
: mSurface(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), static_cast< u64 >(-1))
, mFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59, kMT_Floor, kMT_Wall, kMT_Ceiling),
      CMaterialList(kMT_Character, kMT_Player, kMT_CollisionActor)))
, mAngularRate(180.f)
, mMode(kM_None)
, mCache(rs_new CCollisionCache(CAABox::mskNullBox, 2, 2, 0xffff)) {}

CSurfaceAlignmentHelper::CSurfaceAlignmentHelper(const CMaterialFilter& filter)
: mSurface(CVector3f::Zero(), CVector3f::Right(), CVector3f::Forward(), static_cast< u64 >(-1))
, mFilter(filter)
, mAngularRate(180.f)
, mMode(kM_None)
, mCache(nullptr) {}

void CSurfaceAlignmentHelper::Update(CPhysicsActor& actor, CStateManager& mgr, float dt) {
  switch (mMode) {
  case kM_NearbySurface:
    AlignNearPosition(actor, mgr, actor.GetTranslation() + 2.f * (dt * actor.GetVelocityWR()), dt);
    break;
  case kM_WorldUp:
    OrientToSurfaceNormal(actor, CVector3f(CVector3f::Up()), dt);
    break;
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
    float bestDist = 2.f * radius * radius;
    CCollisionCacheIterator it(*mCache);
    const CCollisionCache& cache = *mCache;
    for (uint i = 0; i < cache.GetNumTriangles(); ++i) {
      const CCachedCollisionSurface* tri = cache.NextTriangle(it);
      const CCollisionSurface& candidate = tri->GetSurface();
      if (!mFilter.Passes(CMaterialList(candidate.GetSurfaceFlags()))) {
        continue;
      }
      const float dist = CollisionUtil::TriPointSqrDist_Float(
          position, candidate.GetVert(0), candidate.GetVert(1), candidate.GetVert(2), nullptr,
          nullptr);
      if (dist < bestDist && PointOnSurface(candidate, position)) {
        bestDist = dist;
        found = true;
        surface = candidate;
      }
    }
  }
  return found;
}

void CSurfaceAlignmentHelper::AlignNearPosition(CActor& actor, CStateManager& mgr,
                                                const CVector3f& position, float dt) {
  if (FindNearestSurface(mgr, position, 1.8f, mSurface) == true) {
    OrientToStoredSurface(actor, dt);
  }
}

void CSurfaceAlignmentHelper::OrientToStoredSurface(CActor& actor, float dt) const {
  OrientToSurfaceNormal(actor, CVector3f(mSurface.GetNormal()), dt);
}

void CSurfaceAlignmentHelper::OrientToSurfaceNormal(CActor& actor, const CVector3f& normal,
                                                    float dt) const {
  const float dot = CVector3f::Dot(actor.GetTransform().GetUp(), normal);
  if (close_enough(dot, 1.f) || dot < -0.999f) {
    return;
  }
  const CQuaternion arc = CQuaternion::ShortestRotationArcClamped(
      actor.GetTransform().GetUp(), normal, CRelAngle::FromDegrees(mAngularRate * dt));
  const CQuaternion local(arc.GetScalar(), actor.GetTransform().TransposeRotate(arc.GetVector()));
  actor.SetRotation((actor.GetRotation() * local).BuildNormalized());
}

bool CSurfaceAlignmentHelper::PointOnSurface(const CCollisionSurface& surface,
                                             const CVector3f& point) const {
  const CVector3f projected = ProjectPointToPlane(point, surface.GetVert(0), surface.GetNormal());
  const CVector3f normal = surface.GetNormal();
  for (int i = 0; i < 3; ++i) {
    const CVector3f& vert = surface.GetVert(i);
    const CVector3f edge = surface.GetVert((i + 2) % 3) - vert;
    const CVector3f toPoint = projected - vert;
    if (CVector3f::Dot(normal, CVector3f::Cross(toPoint, edge)) < 0.f) {
      return false;
    }
  }
  return true;
}

CVector3f CSurfaceAlignmentHelper::ProjectPointToPlane(const CVector3f& point,
                                                       const CVector3f& origin,
                                                       const CVector3f& normal) const {
  return point - CVector3f::Dot(point - origin, normal) * normal;
}
