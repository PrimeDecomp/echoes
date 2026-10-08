#include "MetroidPrime/Cameras/CCameraColliderGroup.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "math.h"

static CMaterialList kLineOfSightIncludeList = CMaterialList(kMT_Solid);
static CMaterialList kLineOfSightExcludeList =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
static CMaterialFilter kLineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude(kLineOfSightIncludeList, kLineOfSightExcludeList);

CCameraCollider::CCameraCollider(float radius, CVector3f position, float scale)
: mRadius(radius)
, mLastLocalPos(position)
, mLocalPos(position)
, mScaledWorldPos(position)
, mLastWorldPos(position)
, mOcclusionCount(0)
, mScale(scale) {}

CCameraColliderGroup::CCameraColliderGroup()
: mCentroid(0.f, 0.f, 0.f)
, mLookPosition(CVector3f::Zero())
, mClearColliderThreshold(0.2f)
, x30_(0)
, x34_(0)
, mColliderIterator(0)
, x3c_24_(true) {}

void CCameraColliderGroup::UpdateCollidersDistances(float xMag, float zMag, float angleOffset) {
  float theta = angleOffset;
  for (int i = 0; i < mColliders.size(); ++i) {
    float z = zMag * cosf(theta);
    if (theta > M_PIF / 2.f) {
      z *= 0.25f;
    }

    const float x = xMag * CMath::Limit(sinf(theta), 1.f);
    mColliders[i].SetDesiredPosition(CVector3f(x, 0.f, z));
    theta += 2.f * M_PIF / mColliders.size();
  }
}

void CCameraColliderGroup::UpdateColliders(const CTransform4f& xf, const CVector3f& lookPosition,
                                           int count, float tolerance,
                                           const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                           CStateManager& mgr) {
  if (mColliderIterator < mColliders.size()) {
    const float inverseTolerance = 1.f / tolerance;
    mLookPosition = lookPosition;
    const CVector3f center = xf.GetTranslation();

    for (int i = 0; i < count; ++i) {
      CVector3f localPos = mColliders[mColliderIterator].GetDesiredPosition();
      CVector3f worldPos = xf.Rotate(localPos) + center;
      if ((mColliders[mColliderIterator].GetRealPosition() - worldPos).Magnitude() < 0.1f) {
        localPos = mColliders[mColliderIterator].GetPosition();
        worldPos = mColliders[mColliderIterator].GetRealPosition();
      }

      CVector3f centerToCollider = worldPos - center;
      const float magnitude = centerToCollider.Magnitude();
      if (centerToCollider.IsMagnitudeSafe()) {
        centerToCollider.Normalize();
        TUniqueId intersectId = kInvalidUniqueId;
        const CRayCastResult result = mgr.RayWorldIntersection(
            intersectId, xf.GetTranslation(), centerToCollider,
            magnitude + mColliders[mColliderIterator].GetRadius(), kLineOfSightFilter, nearList);
        if (result.IsValid()) {
          worldPos =
              xf.GetTranslation() +
              centerToCollider * (result.GetTime() - mColliders[mColliderIterator].GetRadius());
          localPos = xf.GetRotation().GetInverse() * (worldPos - xf.GetTranslation());
        }
      }

      mColliders[mColliderIterator].SetRealPosition(worldPos);
      mColliders[mColliderIterator].SetPosition(localPos);
      const CVector3f scaledWorldPos =
          inverseTolerance * (magnitude * centerToCollider) + mLookPosition;
      mColliders[mColliderIterator].SetLookAtPosition(scaledWorldPos);
      if (mgr.RayCollideWorld(worldPos, scaledWorldPos, nearList, kLineOfSightFilter, nullptr)) {
        mColliders[mColliderIterator].SetOcclusionCount(0);
      } else {
        mColliders[mColliderIterator].SetOcclusionCount(
            mColliders[mColliderIterator].GetOcclusionCount() + 1);
      }

      ++mColliderIterator;
      if (mColliderIterator == mColliders.size()) {
        mColliderIterator = 0;
      }
    }

    mCentroid = CalculateCollidersCentroid();
  }
}

CVector3f CCameraColliderGroup::CalculateCollidersCentroid() const {
  const int colliderCount = mColliders.size();
  if (colliderCount < 3) {
    return CVector3f(0.f, 1.f, 0.f);
  }

  float accumCross = 0.f;
  float accumX = 0.f;
  float accumZ = 0.f;
  int obscuredEdges = 0;
  int previous = colliderCount - 1;
  for (int i = 0; i < colliderCount; ++i) {
    if (mColliders[previous].GetOcclusionCount() < 2 && mColliders[i].GetOcclusionCount() < 2) {
      const float scale = mColliders[previous].GetScale();
      const CVector3f p0 = scale * mColliders[previous].GetPosition();
      const CVector3f p1 = scale * mColliders[i].GetPosition();
      const float cross = p0.GetX() * p1.GetZ() - p1.GetX() * p0.GetZ();
      accumCross += cross;
      accumX += cross * (p1.GetX() + p0.GetX());
      accumZ += cross * (p1.GetZ() + p0.GetZ());
    } else {
      ++obscuredEdges;
    }
    previous = i;
  }

  if (static_cast< float >(obscuredEdges) / static_cast< float >(colliderCount) <=
      mClearColliderThreshold) {
    return CVector3f(0.f, 1.f, 0.f);
  }

  if (accumCross != 0.f) {
    const float baryCross = 3.f * accumCross;
    return CVector3f(accumX / baryCross, 0.f, accumZ / baryCross);
  }

  return CVector3f(0.f, 2.f, 0.f);
}

int CCameraColliderGroup::CountObscuredColliders() const {
  int count = 0;
  for (int i = 0; i < mColliders.size(); ++i) {
    if (mColliders[i].GetOcclusionCount() >= 2) {
      ++count;
    }
  }
  return count;
}

void CCameraColliderGroup::SetupColliders(float xMag, float zMag, float radius, int count,
                                          float startAngle) {
  mColliders.reserve(count);
  for (int i = 0; i < count; ++i) {
    float z = zMag * static_cast< float >(cos(startAngle));
    if (startAngle > M_PIF / 2.f) {
      z *= 0.25f;
    }

    const CVector3f position(xMag * static_cast< float >(sin(startAngle)), 0.f, z);
    mColliders.push_back_unsafe(CCameraCollider(radius, position, 1.f));
    startAngle += 2.f * M_PIF / float(count);
  }
}

CAABox CCameraColliderGroup::CalculateCollidersBoundingBox() const {
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  for (int i = 0; i < mColliders.size(); ++i) {
    bounds.AccumulateBounds(mColliders[i].GetRealPosition());
  }
  bounds.AccumulateBounds(mLookPosition);
  return bounds;
}

void CCameraColliderGroup::TeleportColliders(CVector3f position) {
  for (int i = 0; i < mColliders.size(); ++i) {
    mColliders[i].SetRealPosition(position);
    mColliders[i].SetDesiredPosition(position);
    mColliders[i].SetLookAtPosition(position);
  }
}
