#include "MetroidPrime/Enemies/CCollisionTracker.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CSurfaceParticleEffect.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "WorldFormat/CCollisionPrimitiveData.hpp"

#include <float.h>
#include <math.h>

// Guessed names: the material filter used to build the collision cache.
static EMaterialTypes sCacheIncludeMaterial = kMT_Solid;
static EMaterialTypes sCacheExcludeMaterial0 = kMT_Character;
static EMaterialTypes sCacheExcludeMaterial1 = kMT_Player;
static EMaterialTypes sCacheExcludeMaterial2 = kMT_CollisionActor;

SSurfacePathSegment::SSurfacePathSegment(const CQuaternion& orientation, float length,
                                         const CVector3f& position, const CVector3f& direction)
: mOrientation(orientation)
, mTransform(orientation.BuildTransform())
, mPosition(position)
, mDirection(direction)
, mLength(length) {}

CSurfacePath::CSurfacePath(int reserveCount) : mTotalLength(0.f), mRandom(99) {
  mSegments.reserve(reserveCount > 4 ? reserveCount : 4);
}

float SBlobStrand::GetCurrentLength() const {
  return 2.f * (mStartLength + (mEndLength - mStartLength) * mAge / mLifetime);
}

bool SBlobStrand::IsFinished() const {
  if (mAge >= mLifetime) {
    return true;
  }
  const float tail = mPathDistance + GetCurrentLength();
  if (tail > mPath.GetTotalLength()) {
    return true;
  }
  return CMath::AbsF(tail - mPath.GetTotalLength()) < 0.00001f;
}

void CSurfacePath::Simplify() {
  if (mSegments.size() < 2) {
    return;
  }
  int out = 0;
  for (int i = 1; i < mSegments.size() - 1; ++i) {
    SSurfacePathSegment& current = mSegments[out];
    const SSurfacePathSegment& next = mSegments[i];
    if (close_enough(current.mOrientation, next.mOrientation, 0.001f) &&
        CMath::AbsF(CVector3f::Dot(current.mDirection, next.mDirection) - 1.f) < 0.00001f) {
      current.mLength += next.mLength;
    } else {
      ++out;
      mSegments[out] = next;
    }
  }
  mSegments[out + 1] = mSegments[mSegments.size() - 1];
  while (mSegments.size() > out + 2) {
    mSegments.pop_back();
  }
}

static bool SlideOffPlane(const rstl::optional_object< STriangleSlot >& previous,
                          const STriangleSlot* triangle, float magnitude, float inverseMagnitude,
                          CVector3f& velocity);
static CVector3f ProjectOntoPlane(const CVector3f& vector, float normalDot, float magnitude,
                                  const CVector3f& normal);
static int FindExitEdge(const STriangleSlot* triangle, const CVector3f& position,
                        const CVector3f& velocity, CVector3f& barycentric, int enteredEdge,
                        float& distance);
static bool IsMovingAlongEdge(const STriangleSlot* triangle, const CVector3f& velocity,
                              const CVector3f& barycentric, float magnitude);
static int FindSharedEdge(ushort triangleIndex, int edge, ushort neighborIndex,
                          const CCollisionCacheIterator& iterator);
static bool CastRay(CCollisionCache& cache, const CVector3f& from, const CVector3f& to,
                    const STriangleSlot** hitTriangle, const STriangleSlot* ignoreA,
                    const STriangleSlot* ignoreB, float* hitFraction,
                    CCollisionCacheIterator& hitIterator);

bool CSurfacePath::Trace(float distance, const CVector3f& start, const CVector3f& direction,
                         CCollisionCache& cache, int maxSteps,
                         const rstl::optional_object< TStepCallback >& cb) {
  const STriangleSlot* triangle = nullptr;
  CVector3f previousPosition = start;
  CVector3f position = start;
  CVector3f unused = CVector3f::Zero();
  CVector3f velocity = direction;
  float magnitude = velocity.Magnitude();
  float inverseMagnitude = 1.f / magnitude;
  rstl::optional_object< STriangleSlot > previous;
  rstl::optional_object< STriangleSlot > current;
  CCollisionCacheIterator iterator(cache);
  int enteredEdge = -1;
  int stepCount = 0;
  float remaining = distance;

  if (!FindNearestTriangle(2.f, position, nullptr, velocity, cache, &triangle, &unused, &position,
                           iterator)) {
    return false;
  }

  while (true) {
    const CVector3f& normal = triangle->mPlane.GetNormal();
    const float normalDot = CVector3f::Dot(velocity, normal);
    const float cosine = normalDot * inverseMagnitude;
    if (CMath::AbsF(cosine) > 0.2f) {
      if (!SlideOffPlane(previous, triangle, magnitude, inverseMagnitude, velocity)) {
        if (CMath::AbsF(cosine) >= 0.8f) {
          break;
        }
        velocity = ProjectOntoPlane(velocity, normalDot, magnitude, normal);
      }
    } else {
      velocity = ProjectOntoPlane(velocity, normalDot, magnitude, normal);
    }

    if (previous.valid()) {
      AddSegment(&previous.data(), previousPosition, position - previousPosition, false);
    }
    previousPosition = position;

    CVector3f barycentric = CVector3f::Zero();
    float stepDistance = -1.f;
    int edge = FindExitEdge(triangle, position, velocity, barycentric, enteredEdge, stepDistance);
    if (edge == -1 || stepDistance < 0.0011920929f) {
      stepDistance = 1.f;
    }

    if (IsMovingAlongEdge(triangle, velocity, barycentric, magnitude)) {
      if (!SlideOffPlane(previous, triangle, magnitude, inverseMagnitude, velocity)) {
        const float u = mRandom.Float();
        const float v = (1.f - u) * mRandom.Float();
        const CVector3f bary(u, v, 1.f - (u + v));
        const CVector3f point = CMath::BaryToWorld(triangle->mTriangle.mSurface.GetVert(0),
                                                   triangle->mTriangle.mSurface.GetVert(1),
                                                   triangle->mTriangle.mSurface.GetVert(2), bary);
        velocity = (point - position).AsNormalized() * magnitude;
      }
      edge = FindExitEdge(triangle, position, velocity, barycentric, enteredEdge, stepDistance);
      if (edge == -1) {
        break;
      }
    }

    CCollisionCacheIterator hitIterator(cache);
    const STriangleSlot* hitTriangle;
    float hitFraction;
    bool crossed = false;
    if (CastRay(cache, position, position + velocity * stepDistance, &hitTriangle,
                previous.valid() ? &previous.data() : nullptr, triangle, &hitFraction,
                hitIterator)) {
      const float hitDistance = stepDistance * hitFraction;
      if (hitDistance < remaining && hitDistance > 0.0011920929f) {
        stepDistance = hitDistance;
        previous = *triangle;
        crossed = true;
        triangle = hitTriangle;
        iterator = hitIterator;
        enteredEdge = -1;
      }
    }

    remaining -= stepDistance;
    if (remaining < 0.f) {
      stepDistance += remaining;
      position += velocity * stepDistance;
      if (crossed) {
        AddSegment(&previous.data(), previousPosition, position - previousPosition, false);
      } else {
        AddSegment(triangle, previousPosition, position - previousPosition, false);
      }
      break;
    }

    position += velocity * stepDistance;
    if (stepCount >= maxSteps) {
      break;
    }
    ++stepCount;
    if (cb.valid()) {
      cb.data()(position, velocity);
      magnitude = velocity.Magnitude();
      inverseMagnitude = 1.f / magnitude;
    }
    if (crossed) {
      continue;
    }

    previous = *triangle;
    ushort neighbor = 0xffff;
    if (edge != -1) {
      neighbor = iterator.GetGeometry().GetTriangleNeighbors(triangle->mTriangle.mIndex)[edge];
    }
    if (neighbor == 0xffff || neighbor == triangle->mTriangle.mIndex) {
      const float searchRadius = edge == -1 ? 0.1f : 2.f;
      CVector3f closest = CVector3f::Zero();
      if (!FindNearestTriangle(searchRadius, position, triangle, velocity, cache, &triangle,
                               &unused, &closest, iterator)) {
        break;
      }
      position = closest;
      enteredEdge = -1;
    } else {
      const ushort previousIndex = triangle->mTriangle.mIndex;
      const CCollisionSurface surface = iterator.GetGeometry().GetTriangle(
          neighbor, iterator.GetTransform(), iterator.GetMaterialFlags());
      current = STriangleSlot(surface, neighbor);
      triangle = &current.data();
      enteredEdge = FindSharedEdge(previousIndex, edge, neighbor, iterator);
    }
  }

  if (GetSegmentCount() != 0) {
    AddSegment(triangle, position, CVector3f::Up(), true);
  }
  return true;
}

static bool IsSameTriangle(const STriangleSlot* a, const STriangleSlot* b) {
  return a != nullptr && b != nullptr && a->mTriangle.mIndex == b->mTriangle.mIndex &&
         close_enough(a->mTriangle.mSurface.GetVert(0), b->mTriangle.mSurface.GetVert(0),
                      0.0001f) &&
         close_enough(a->mTriangle.mSurface.GetVert(1), b->mTriangle.mSurface.GetVert(1),
                      0.0001f) &&
         close_enough(a->mTriangle.mSurface.GetVert(2), b->mTriangle.mSurface.GetVert(2), 0.0001f);
}

static CVector3f ProjectOntoPlane(const CVector3f& vector, float normalDot, float magnitude,
                                  const CVector3f& normal) {
  return (vector - normal * normalDot).AsNormalized() * magnitude;
}

static bool SlideOffPlane(const rstl::optional_object< STriangleSlot >& previous,
                          const STriangleSlot* triangle, float magnitude, float inverseMagnitude,
                          CVector3f& velocity) {
  if (!previous.valid()) {
    return false;
  }
  const CPlane& plane = previous->mPlane;
  // 1 when the triangle straddles the previous plane, 2 when above it, 3 when below it.
  int side = 0;
  for (int i = 0; i < 3; ++i) {
    const float dist = CVector3f::Dot(triangle->mTriangle.mSurface.GetVert(i), plane.GetNormal()) -
                       plane.GetConstant();
    if (CMath::AbsF(dist) > 0.00011920929f) {
      const int vertexSide = dist > 0.f ? 2 : 3;
      if (side == 0) {
        side = vertexSide;
      } else if (side != vertexSide) {
        side = 1;
        break;
      }
    }
  }
  const float sign = side == 3 ? -1.f : 1.f;
  const CVector3f& triangleNormal = triangle->mPlane.GetNormal();
  velocity = plane.GetNormal() * magnitude * sign;
  const float normalDot = CVector3f::Dot(velocity, triangleNormal);
  if (CMath::AbsF(normalDot * inverseMagnitude) > 0.95f) {
    return false;
  }
  velocity = ProjectOntoPlane(velocity, normalDot, magnitude, triangleNormal);
  return true;
}

static int GetDominantAxis(const CVector3f& normal) {
  int axis = CMath::AbsF(normal[1]) > CMath::AbsF(normal[0]) ? 1 : 0;
  if (CMath::AbsF(normal[2]) > CMath::AbsF(normal[axis])) {
    axis = 2;
  }
  return axis;
}

static float GetInverseArea(const STriangleSlot* triangle) {
  const CCollisionSurface& s = triangle->mTriangle.mSurface;
  const CVector3f& v0 = s.GetVert(0);
  const CVector3f& v1 = s.GetVert(1);
  const CVector3f& v2 = s.GetVert(2);
  switch (GetDominantAxis(triangle->mPlane.GetNormal())) {
  case 2:
    return 1.f / ((v2.GetX() - v0.GetX()) * (v1.GetY() - v0.GetY()) -
                  (v1.GetX() - v0.GetX()) * (v2.GetY() - v0.GetY()));
  case 1:
    return 1.f / ((v2.GetX() - v0.GetX()) * (v1.GetZ() - v0.GetZ()) -
                  (v1.GetX() - v0.GetX()) * (v2.GetZ() - v0.GetZ()));
  default:
    return 1.f / ((v2.GetZ() - v0.GetZ()) * (v1.GetY() - v0.GetY()) -
                  (v1.GetZ() - v0.GetZ()) * (v2.GetY() - v0.GetY()));
  }
}

static CVector3f GetBarycentric(const STriangleSlot* triangle, const CVector3f& point,
                                float inverseArea) {
  const CCollisionSurface& s = triangle->mTriangle.mSurface;
  const CVector3f& v0 = s.GetVert(0);
  const CVector3f& v1 = s.GetVert(1);
  const CVector3f& v2 = s.GetVert(2);
  float a1, a2, b1, b2, c1, c2;
  switch (GetDominantAxis(triangle->mPlane.GetNormal())) {
  case 2:
    a1 = v1.GetX() - point.GetX();
    a2 = v1.GetY() - point.GetY();
    b1 = v2.GetX() - point.GetX();
    b2 = v2.GetY() - point.GetY();
    c1 = v0.GetX() - point.GetX();
    c2 = v0.GetY() - point.GetY();
    break;
  case 1:
    a1 = v1.GetX() - point.GetX();
    a2 = v1.GetZ() - point.GetZ();
    b1 = v2.GetX() - point.GetX();
    b2 = v2.GetZ() - point.GetZ();
    c1 = v0.GetX() - point.GetX();
    c2 = v0.GetZ() - point.GetZ();
    break;
  default:
    a1 = v1.GetZ() - point.GetZ();
    a2 = v1.GetY() - point.GetY();
    b1 = v2.GetZ() - point.GetZ();
    b2 = v2.GetY() - point.GetY();
    c1 = v0.GetZ() - point.GetZ();
    c2 = v0.GetY() - point.GetY();
    break;
  }
  return CVector3f((a1 * b2 - b1 * a2) * inverseArea, (b1 * c2 - c1 * b2) * inverseArea,
                   (c1 * a2 - a1 * c2) * inverseArea);
}

static int FindExitEdge(const STriangleSlot* triangle, const CVector3f& position,
                        const CVector3f& velocity, CVector3f& barycentric, int enteredEdge,
                        float& distance) {
  const float inverseArea = GetInverseArea(triangle);
  barycentric = GetBarycentric(triangle, position, inverseArea);
  const CVector3f next = GetBarycentric(triangle, position + velocity, inverseArea);
  const CVector3f delta = next - barycentric;

  float t0 = 0.f;
  float t1 = 0.f;
  float t2 = 0.f;
  if (CMath::AbsF(delta.GetX()) > 0.00059604645f) {
    t0 = -barycentric.GetX() / delta.GetX();
  }
  if (CMath::AbsF(delta.GetY()) > 0.00059604645f) {
    t1 = -barycentric.GetY() / delta.GetY();
  }
  if (CMath::AbsF(delta.GetZ()) > 0.00059604645f) {
    t2 = -barycentric.GetZ() / delta.GetZ();
  }

  int edge = -1;
  const bool valid = t0 > 0.00059604645f && enteredEdge != 1;
  distance = valid ? t0 : FLT_MAX;
  if (valid) {
    edge = 1;
  }
  if (t1 > 0.00059604645f && t1 < distance && enteredEdge != 2) {
    distance = t1;
    edge = 2;
  }
  if (t2 > 0.00059604645f && t2 < distance && enteredEdge != 0) {
    distance = t2;
    edge = 0;
  }
  return edge;
}

static CVector3f GetOppositeEdge(const STriangleSlot* triangle, int vertex) {
  const CCollisionSurface& s = triangle->mTriangle.mSurface;
  if (vertex == 0) {
    return s.GetVert(2) - s.GetVert(1);
  }
  if (vertex == 1) {
    return s.GetVert(0) - s.GetVert(2);
  }
  return s.GetVert(1) - s.GetVert(0);
}

static bool IsMovingAlongEdge(const STriangleSlot* triangle, const CVector3f& velocity,
                              const CVector3f& barycentric, float magnitude) {
  for (int i = 0; i < 3; ++i) {
    if (CMath::AbsF(barycentric[i] - 0.f) < 0.00001f) {
      const CVector3f edge = GetOppositeEdge(triangle, i).AsNormalized();
      if (CMath::AbsF(CMath::AbsF(CVector3f::Dot(velocity, edge)) - magnitude) < 0.00001f) {
        return true;
      }
    }
  }
  return false;
}

static int FindSharedEdge(ushort triangleIndex, int edge, ushort neighborIndex,
                          const CCollisionCacheIterator& iterator) {
  const ushort* triangleEdges = iterator.GetGeometry().GetTriangleEdgeIndices(triangleIndex);
  const ushort sharedEdge = triangleEdges[edge];
  const ushort* neighborEdges = iterator.GetGeometry().GetTriangleEdgeIndices(neighborIndex);
  if (sharedEdge == neighborEdges[0]) {
    return 0;
  }
  if (sharedEdge == neighborEdges[1]) {
    return 1;
  }
  if (sharedEdge == neighborEdges[2]) {
    return 2;
  }
  return -1;
}

static bool CastRay(CCollisionCache& cache, const CVector3f& from, const CVector3f& to,
                    const STriangleSlot** hitTriangle, const STriangleSlot* ignoreA,
                    const STriangleSlot* ignoreB, float* hitFraction,
                    CCollisionCacheIterator& hitIterator) {
  CAABox bounds(from, from);
  bounds.AccumulateBounds(to);
  const CVector3f delta = to - from;
  const float length = delta.Magnitude();
  const float inverseLength = 1.f / length;
  const CVector3f direction = delta * inverseLength;
  const STriangleSlot* nearest = nullptr;
  float nearestDistance = FLT_MAX;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() && !cache.GetLeafBounds(iterator)->DoBoundsOverlap(bounds)) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const STriangleSlot* slot =
        reinterpret_cast< const STriangleSlot* >(cache.NextTriangle(iterator));
    if (IsSameTriangle(ignoreA, slot) || IsSameTriangle(ignoreB, slot)) {
      continue;
    }
    float distance = length;
    if (CollisionUtil::RayTriangleIntersection(from, direction,
                                               &slot->mTriangle.mSurface.GetVert(0), distance) &&
        distance < nearestDistance && distance <= length) {
      nearestDistance = distance;
      nearest = slot;
      hitIterator = iterator;
    }
  }
  if (nearest == nullptr) {
    return false;
  }
  *hitFraction = nearestDistance * inverseLength;
  *hitTriangle = nearest;
  return true;
}

bool CSurfacePath::FindNearestTriangle(float radius, const CVector3f& position,
                                       const STriangleSlot* exclude, const CVector3f& velocity,
                                       CCollisionCache& cache, const STriangleSlot** triangle,
                                       CVector3f* barycentric, CVector3f* closest,
                                       CCollisionCacheIterator& foundIterator) {
  const CSphere sphere(position, radius);
  float nearestDistSq = radius * radius;
  rstl::optional_object< CVector3f > nearest;
  CCollisionCacheIterator iterator(cache);
  while (!iterator.AtEnd()) {
    if (iterator.AtLeafStart() &&
        !CollisionUtil::AABoxSphereIntersection(*cache.GetLeafBounds(iterator), sphere)) {
      cache.SkipLeaf(iterator);
      continue;
    }
    const STriangleSlot* slot =
        reinterpret_cast< const STriangleSlot* >(cache.NextTriangle(iterator));
    if (IsSameTriangle(exclude, slot)) {
      continue;
    }
    float baryU;
    float baryV;
    const float distSq = CollisionUtil::TriPointSqrDist_Float(
        position, slot->mTriangle.mSurface.GetVert(0), slot->mTriangle.mSurface.GetVert(1),
        slot->mTriangle.mSurface.GetVert(2), &baryU, &baryV);
    if (distSq > nearestDistSq) {
      continue;
    }
    if (triangle != nullptr && *triangle != nullptr &&
        CMath::AbsF(nearestDistSq - distSq) < 0.000011920929f &&
        CVector3f::Dot(slot->mPlane.GetNormal(), velocity) <
            CVector3f::Dot((*triangle)->mPlane.GetNormal(), velocity)) {
      continue;
    }
    nearestDistSq = distSq;
    foundIterator = iterator;
    *barycentric = CVector3f(1.f - (baryV + baryU), baryU, baryV);
    nearest =
        CMath::BaryToWorld(slot->mTriangle.mSurface.GetVert(0), slot->mTriangle.mSurface.GetVert(1),
                           slot->mTriangle.mSurface.GetVert(2), *barycentric);
    *triangle = slot;
  }
  if (!nearest.valid()) {
    return false;
  }
  *closest = *nearest;
  return true;
}

void CSurfacePath::AddSegment(const STriangleSlot* slot, const CVector3f& start,
                              const CVector3f& step, bool isLast) {
  const float length = step.Magnitude();
  if (!isLast && length < 0.000011920929f) {
    return;
  }

  CVector3f direction;
  if (isLast) {
    direction = mSegments[mSegments.size() - 1].mTransform.GetColumn(kDZ);
  } else {
    direction = step * (1.f / length);
  }

  if (CMath::AbsF(CVector3f::Dot(slot->mPlane.GetNormal(), direction)) > 0.95f) {
    if (mSegments.size() != 0) {
      mSegments[mSegments.size() - 1].mLength += length;
      mTotalLength += length;
    }
    return;
  }

  const CVector3f side = CVector3f::Cross(direction, slot->mPlane.GetNormal()).AsNormalized();
  const CVector3f up = CVector3f::Cross(side, direction);
  const CQuaternion orientation = CQuaternion::FromMatrixColumns(direction, up, side);
  if (mSegments.size() == mSegments.capacity()) {
    mSegments.reserve(mSegments.capacity() * 2);
  }
  if (!isLast) {
    const SSurfacePathSegment segment(orientation, length, start, direction);
    mSegments.push_back_unsafe(segment);
    mTotalLength += segment.mLength;
  } else {
    const SSurfacePathSegment segment(orientation, 0.f, start, direction);
    mSegments.push_back_unsafe(segment);
  }
}

const float CCollisionTracker::skDefaultExtents = 10.f;

CCollisionTracker::CCollisionTracker(const TLockedToken< CGenDescription >& desc, TUniqueId uid,
                                     TAreaId area, bool active, const rstl::string& name,
                                     const CTransform4f& xf, TUniqueId owner, uint flags,
                                     float trackRadius)
: CEffect(uid, CEntityInfo(area, CEntity::NullConnectionList, active, kInvalidEditorId), name, xf)
, mRandom(CStopwatch::GetGlobalMicros())
, mParticleSystem(rs_new CElementGen(desc, CElementGen::kMOT_One, CElementGen::kOSF_One))
, mModel(*desc->mPMDL, -0.2f, 0.2f, CVector3f(0.25f, 0.f, 0.f), CVector3f::Zero(),
         CVector3f(-0.25f, 0.f, 0.f))
, mCollisionCache(rs_new CCollisionCache(CAABox::MakeNullBox(), 2, 2, uid.value & 0x3ff))
, mLightId(kInvalidUniqueId)
, mParticleAssetId(CToken(desc).GetTag().GetId())
, mParticleEmissionRateScalar(1.f)
, mSpawnRemainder(0.f)
, mMinimumPathLength(4.f)
, x19c_unknown(0)
, mOwner(owner)
, mTrackBounds(CVector3f(-trackRadius, -trackRadius, -trackRadius),
               CVector3f(trackRadius, trackRadius, trackRadius))
, mTrackRadius(trackRadius) {
  SetHighlightedInDarkVisor(true);
  mStrands.reserve(8);
}

CCollisionTracker::~CCollisionTracker() {}

CElementGen* CCollisionTracker::ParticleSystem() const { return mParticleSystem.get(); }

CElementGen* CCollisionTracker::GetParticleSystem() const { return mParticleSystem.get(); }

void CCollisionTracker::SetMinimumPathLength(float length) { mMinimumPathLength = length; }

float CCollisionTracker::GetMinimumPathLength() const { return mMinimumPathLength; }

float CCollisionTracker::GetParticleEmissionRateScalar() const {
  return mParticleEmissionRateScalar;
}

void CCollisionTracker::SetParticleEmissionRateScalar(float intensity) {
  mParticleEmissionRateScalar = intensity;
}

void CCollisionTracker::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CCollisionTracker::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CCollisionTracker::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();

  switch (message) {
  case kSM_Create:
    if (mParticleSystem->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticleSystem->GetLight(),
                                      mParticleAssetId, 1, 0.f));
    }
    break;
  case kSM_Delete:
    if (mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
      mLightId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(mgr, msg);
  if (mLightId != kInvalidUniqueId) {
    mgr.SendScriptMsg(mLightId, sender, message, kInvalidUniqueId);
  }
}

void CCollisionTracker::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() == CGameArea::kOS_Occluded) {
    return;
  }

  float maxLength = 0.f;
  CAABox bounds(GetTranslation(), GetTranslation());
  CGenDescription* desc = mParticleSystem->mLoadedGenDesc;
  rstl::vector< SBlobStrand >::iterator it = mStrands.begin();
  while (it != mStrands.end()) {
    if (desc->mVEL1 != nullptr) {
      CVector3f velocity(it->mSpeed, 0.f, 0.f);
      CVector3f position = CVector3f::Zero();
      desc->mVEL1->GetValue(0, velocity, position);
      it->mSpeed = velocity.GetX();
    }
    it->mPathDistance += it->mSpeed * dt;
    it->mSegmentDistance += it->mSpeed * dt;
    it->mAge += dt;
    if (it->IsFinished()) {
      if (it + 1 == mStrands.end()) {
        mStrands.pop_back();
        break;
      }
      *it = mStrands.back();
      mStrands.pop_back();
      if (it != mStrands.end() && it->IsFinished()) {
        continue;
      }
    }
    while (it->mSegmentDistance > it->mPath.GetSegment(it->mSegmentIndex).mLength) {
      it->mSegmentDistance -= it->mPath.GetSegment(it->mSegmentIndex).mLength;
      ++it->mSegmentIndex;
    }
    const SSurfacePathSegment& segment = it->mPath.GetSegment(it->mSegmentIndex);
    bounds.AccumulateBounds(segment.mPosition + segment.mDirection * it->mSegmentDistance);
    maxLength = CMath::Max(CMath::Max(maxLength, it->mStartLength), it->mEndLength);
    ++it;
  }

  bounds.AccumulateBounds(bounds.GetMaxPoint() + CVector3f(maxLength, maxLength, maxLength));
  bounds.AccumulateBounds(bounds.GetMinPoint() - CVector3f(maxLength, maxLength, maxLength));
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);

  mParticleSystem->DestroyParticles();
  mParticleSystem->SetGeneratorRate(mParticleEmissionRateScalar);
  mSpawnRemainder += mParticleSystem->GetGenerationRate();
  const int wanted = static_cast< int >(CMath::FloorF(mSpawnRemainder));
  const int existing = mParticleSystem->GetParticleCount();
  int spawnCount = mParticleSystem->mMAXP - existing;
  if (wanted < spawnCount) {
    spawnCount = wanted;
  }
  if (existing == 0 && spawnCount == 0) {
    return;
  }
  mSpawnRemainder -= static_cast< float >(spawnCount);
  mParticleSystem->SetGeneratorRate(0.f);
  mParticleSystem->SetTranslation(GetTranslation());
  mParticleSystem->SetOrientation(CQuaternion::FromMatrix(GetTransform()).BuildTransform4f());
  mParticleSystem->ForceParticleCreation(spawnCount);

  const int particleCount = mParticleSystem->GetParticleCount();
  if (particleCount != 0) {
    const CVector3f position = GetTranslation();
    const CAABox cacheBounds = mTrackBounds.GetTransformedAABox(CTransform4f::Translate(position));
    const float cacheRadius = mTrackRadius / 10.f;
    const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
        CMaterialList(sCacheIncludeMaterial),
        CMaterialList(sCacheExcludeMaterial0, sCacheExcludeMaterial1, sCacheExcludeMaterial2));
    if (mLastCachePosition.valid() &&
        (*mLastCachePosition - position).MagSquared() < cacheRadius * cacheRadius) {
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      mgr.BuildNearList(nearList, mCollisionCache->GetBounds(), filter, this);
      CGameCollision::UpdateCollisionCache(mgr, *mCollisionCache, nearList,
                                           CGameCollision::kCUP_KeepNearListIds);
    } else {
      mCollisionCache->SetBounds(cacheBounds);
      CGameCollision::BuildCollisionCache(mgr, *mCollisionCache, filter);
      mLastCachePosition = position;
    }
  }

  for (int i = 0; i < particleCount; ++i) {
    const CElementGen::CParticle& particle = mParticleSystem->mParticles[i];
    if (mStrands.size() == mStrands.capacity()) {
      mStrands.reserve(mStrands.capacity() * 2);
    }
    mStrands.push_back_unsafe(SBlobStrand());
    const float lifetime = static_cast< float >(particle.mEndFrame - particle.mStartFrame);
    SBlobStrand& strand = mStrands.back();
    const float speed = particle.mVel.Magnitude();
    const CVector3f start =
        particle.mPos - particle.mVel * particle.mLineLengthOrSize * (1.f / speed);
    const CVector3f direction = particle.mVel * 0.6f;
    strand.mPath.Trace(lifetime, start, direction, *mCollisionCache, 64,
                       rstl::optional_object< CSurfacePath::TStepCallback >());
    if (strand.mPath.GetTotalLength() < mMinimumPathLength) {
      strand = SBlobStrand();
    }
    if (strand.mPath.GetSegmentCount() > 1) {
      strand.mPath.Simplify();
      strand.mStartLength = 2.f * particle.mLineLengthOrSize;
      strand.mEndLength = 0.05f;
      strand.mSpeed = 60.f * speed;
      strand.mPathDistance = 0.f;
      strand.mSegmentDistance = 0.f;
      strand.mAge = 0.f;
      const float ratio =
          (60.f * strand.mPath.GetTotalLength()) / (lifetime * (0.6f * strand.mSpeed));
      strand.mSpeed = strand.mSpeed * ratio;
      strand.mLifetime = (lifetime * ratio) / 60.f;
    } else {
      mStrands.pop_back();
    }
  }
}

void CCollisionTracker::Render(const CStateManager& mgr) const {
  int alpha = GetRenderAlphaBufferAlpha(mgr);
  if (alpha != -1 && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    gpRender->SetDestinationAlpha(alpha);
  } else {
    alpha = -1;
  }
  RenderStrands();
  if (alpha != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

SPathPosition SBlobStrand::GetPathPosition(float offset) const {
  int index = mSegmentIndex;
  float distance = mSegmentDistance + offset;
  while (distance > mPath.GetSegment(index).mLength) {
    distance -= mPath.GetSegment(index).mLength;
    ++index;
    if (index == mPath.GetSegmentCount() - 1) {
      return SPathPosition(index - 1, mPath.GetSegment(index - 1).mLength);
    }
  }
  return SPathPosition(index, distance);
}

CTransform4f CCollisionTracker::BuildStrandTransform(const SPathPosition& start,
                                                     const CVector3f& startPoint,
                                                     const SPathPosition& end,
                                                     const CVector3f& endPoint,
                                                     const SBlobStrand& strand, float scale) const {
  const CVector3f middle = startPoint + (endPoint - startPoint) * 0.5f;
  const SSurfacePathSegment& firstSegment = strand.mPath.GetSegment(start.first);
  if (start.first == end.first) {
    CTransform4f transform(firstSegment.mTransform, middle);
    transform.ScaleBy(scale);
    return transform;
  }

  CQuaternion orientation = firstSegment.mOrientation;
  float travelled = firstSegment.mLength - start.second;
  for (int i = start.first + 1; i != end.first; ++i) {
    const SSurfacePathSegment& segment = strand.mPath.GetSegment(i);
    travelled += segment.mLength;
    orientation =
        CAnimMathUtils::SlerpLocal(orientation, segment.mOrientation, segment.mLength / travelled);
  }
  travelled += end.second;
  orientation = CAnimMathUtils::SlerpLocal(
      orientation, strand.mPath.GetSegment(end.first).mOrientation, end.second / travelled);
  return CTransform4f(CMatrix3f(orientation.BuildTransform(), scale), middle);
}

void CCollisionTracker::RenderStrands() const {
  CAnimMathUtils::sUseFastSlerp = true;
  CGraphics::DisableAllLights();
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::SetUseNormalMatrix(false);
  mModel.SetMaterialCurrent(GetModelFlags());

  int strandIndex = 0;
  while (strandIndex < static_cast< int >(mStrands.size())) {
    int batch = 0;
    for (; batch < 3 && strandIndex < static_cast< int >(mStrands.size()); ++batch, ++strandIndex) {
      const SBlobStrand& strand = mStrands[strandIndex];
      const float length = strand.GetCurrentLength();
      const SPathPosition start(strand.mSegmentIndex, strand.mSegmentDistance);
      const SSurfacePathSegment& segment = strand.mPath.GetSegment(start.first);
      if (segment.mLength > start.second + length) {
        const CVector3f quarter =
            segment.mPosition + segment.mDirection * (start.second + 0.25f * length);
        const CVector3f half =
            segment.mPosition + segment.mDirection * (start.second + 0.5f * length);
        const CVector3f threeQuarters =
            segment.mPosition + segment.mDirection * (start.second + 0.75f * length);
        CTransform4f lower(segment.mTransform, quarter);
        lower.ScaleBy(length);
        CTransform4f middle(segment.mTransform, half);
        middle.ScaleBy(length);
        CTransform4f upper(segment.mTransform, threeQuarters);
        upper.ScaleBy(length);
        mModel.SetSegmentTransforms(batch, lower, middle, upper);
      } else {
        const CVector3f startPoint = segment.mPosition + segment.mDirection * start.second;
        const SPathPosition quarter = strand.GetPathPosition(0.25f * length);
        const SSurfacePathSegment& quarterSegment = strand.mPath.GetSegment(quarter.first);
        const CVector3f quarterPoint =
            quarterSegment.mPosition + quarterSegment.mDirection * quarter.second;
        const SPathPosition half = strand.GetPathPosition(0.5f * length);
        const SSurfacePathSegment& halfSegment = strand.mPath.GetSegment(half.first);
        const CVector3f halfPoint = halfSegment.mPosition + halfSegment.mDirection * half.second;
        const SPathPosition threeQuarters = strand.GetPathPosition(0.75f * length);
        const SSurfacePathSegment& threeQuartersSegment =
            strand.mPath.GetSegment(threeQuarters.first);
        const CVector3f threeQuartersPoint =
            threeQuartersSegment.mPosition + threeQuartersSegment.mDirection * threeQuarters.second;
        const SPathPosition end = strand.GetPathPosition(length);
        const SSurfacePathSegment& endSegment = strand.mPath.GetSegment(end.first);
        const CVector3f endPoint = endSegment.mPosition + endSegment.mDirection * end.second;
        mModel.SetSegmentTransforms(
            batch, BuildStrandTransform(start, startPoint, half, halfPoint, strand, length),
            BuildStrandTransform(quarter, quarterPoint, threeQuarters, threeQuartersPoint, strand,
                                 length),
            BuildStrandTransform(half, halfPoint, end, endPoint, strand, length));
      }
    }
    while (batch > 0) {
      --batch;
      mModel.DrawDisplayList(batch);
    }
  }

  mModel.ResetRenderState();
  CGraphics::SetUseNormalMatrix(true);
  CAnimMathUtils::sUseFastSlerp = false;
}

void CCollisionTracker::AddToRenderer(const CStateManager& mgr) const {
  if (!GetPreRenderClipped()) {
    Render(mgr);
  }
}

void CCollisionTracker::PreRenderAllViewports(CStateManager& mgr) { UpdatePortalSystemState(mgr); }

CEffect* LoadGeomBlobV2(const TLockedToken< CGenDescription >& desc, TUniqueId uid, TAreaId area,
                        bool active, const rstl::string& name, const CTransform4f& xf,
                        TUniqueId owner, uint flags) {
  return rs_new CCollisionTracker(desc, uid, area, active, name, xf, owner, flags,
                                  CCollisionTracker::skDefaultExtents);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SGeomBlobV2_FuncPtrs funcPtrs;
  funcPtrs.mFactory = &LoadGeomBlobV2;
  SetSGeomBlobV2_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() {
  SetFuncPtrs();
  SetSurfaceParticleEffectFuncPtrs();
}

extern "C" void RELExit() {
  SetSGeomBlobV2_FuncPtrs(nullptr);
  ClearSurfaceParticleEffectFuncPtrs();
}
#endif
