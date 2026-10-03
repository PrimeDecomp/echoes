#include "MetroidPrime/CPortalArea.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/math.hpp"
#include "rstl/pair.hpp"

#include <float.h>
#include <string.h>

static int sPortalTraversalDepth;

CPortalArea::SActorPool::SActorPool() : mNodes(SActorNode()) {
  for (int i = 0; i < mNodes.size() - 1; ++i) {
    mNodes[i].mNext = &mNodes[i + 1];
  }
  mFree = mNodes.data();
}

CPortalArea::SActorNode* CPortalArea::SActorPool::AllocateNode() {
  SActorNode* node = mFree;
  mFree = node->mNext;
  memset(node, 0, sizeof(SActorNode));
  return node;
}

void CPortalArea::SActorPool::FreeNode(SActorNode* node) {
  node->mNext = mFree;
  mFree = node;
}

void CPortalArea::SActorList::AddActor(CActor& actor) {
  SActorNode* node = mPool->AllocateNode();
  node->mActor = &actor;
  node->mNext = mHead;
  mHead = node;
}

bool CPortalArea::SActorList::RemoveActor(const TUniqueId& uid) {
  SActorNode* previous = nullptr;
  for (SActorNode* node = mHead; node != nullptr; node = node->mNext) {
    if (node->mActor->GetUniqueId() == uid) {
      if (previous != nullptr) {
        previous->mNext = node->mNext;
      } else {
        mHead = node->mNext;
      }
      mPool->FreeNode(node);
      return true;
    }
    previous = node;
  }
  return false;
}

void CPortalArea::SPortalState::AddActor(CActor& actor) { mActors.AddActor(actor); }

bool CPortalArea::SPortalState::RemoveActor(const TUniqueId& uid) {
  return mActors.RemoveActor(uid);
}

CPortalArea::CPortalArea(const TLockedToken< CPortalAreaData >& data)
: mVisibilityGeneration(0)
, mActorCount(0)
, mData(data)
, mUnassignedActors(mActorPool)
, mVisibleActors(kOL_Actor, false) {
  const rstl::vector< CPortalAreaData::SVolume >& volumes = mData->GetVolumes();
  mVolumes.reserve(volumes.size());
  for (int i = 0; i < volumes.size(); ++i) {
    mVolumes.push_back_unsafe(SPortalState(mActorPool, volumes[i]));
  }
}

void CPortalArea::AddActor(CStateManager& mgr, CActor& actor) {
  rstl::reserved_vector< short, 64 > volumes;
  mData->FindOverlappingVolumes(actor.GetOtherBounds(), volumes);
  if (volumes.empty()) {
    mUnassignedActors.AddActor(actor);
  } else {
    for (int i = 0; i < volumes.size(); ++i) {
      mVolumes[volumes[i]].AddActor(actor);
    }
  }
  ++mActorCount;
}

bool CPortalArea::RemoveActor(CStateManager& mgr, const TUniqueId& uid) {
  bool removed = mUnassignedActors.RemoveActor(uid);
  for (int i = 0; i < mVolumes.size(); ++i) {
    removed |= mVolumes[i].RemoveActor(uid);
  }
  if (removed) {
    --mActorCount;
  }
  return removed;
}

void CPortalArea::UpdateActor(CStateManager& mgr, CActor& actor) {
  RemoveActor(mgr, actor.GetUniqueId());
  AddActor(mgr, actor);
}

void CPortalArea::PreRender(CStateManager& mgr, const CGameCamera& camera, const CTransform4f& xf) {
  mVisibleActors.Clear();
  ++mVisibilityGeneration;

  const float radius = 0.25f * camera.GetNearClipDistance();
  const CVector3f extent(radius, radius, radius);
  const CAABox bounds(camera.GetTranslation() - extent, camera.GetTranslation() + extent);
  rstl::reserved_vector< short, 64 > volumes;
  mData->FindOverlappingVolumes(bounds, volumes);

  const CFrustumPlanes frustum(xf, CRelAngle::FromDegrees(camera.GetFov()).AsRadians(),
                               camera.GetAspectRatio(), camera.GetNearClipDistance(), false, 100.f);
  if (volumes.empty()) {
    for (short i = 0; i < mVolumes.size(); ++i) {
      BuildVisibleActorList(mgr, camera, i, frustum, true, xf.GetTranslation());
    }
  } else {
    for (int i = 0; i < volumes.size(); ++i) {
      BuildVisibleActorList(mgr, camera, volumes[i], frustum, false, xf.GetTranslation());
    }
  }

  sPortalTraversalDepth = 0;
  for (SActorNode* node = mUnassignedActors.mHead; node != nullptr; node = node->mNext) {
    if (node->mActor != nullptr && frustum.BoxInFrustumPlanes(node->mActor->GetOtherBounds())) {
      mVisibleActors.AddObjectIfAbsent(*node->mActor);
    }
  }
}

static rstl::pair< bool, CFrustumPlanes > ClipPortal(const CGameCamera& camera,
                                                     const CFrustumPlanes& frustum,
                                                     const CPortalAreaData::SPortal& portal);

void CPortalArea::BuildVisibleActorList(CStateManager& mgr, const CGameCamera& camera,
                                        short volumeIndex, const CFrustumPlanes& frustum,
                                        bool skipPortals, const CVector3f& entryPoint) {
  ++sPortalTraversalDepth;
  if (sPortalTraversalDepth > 100) {
    return;
  }

  SPortalState& volume = mVolumes[volumeIndex];
  for (SActorNode* node = volume.mActors.mHead; node != nullptr; node = node->mNext) {
    if (node->mActor != nullptr && frustum.BoxInFrustumPlanes(node->mActor->GetOtherBounds())) {
      mVisibleActors.AddObjectIfAbsent(*node->mActor);
    }
  }

  if (!skipPortals) {
    const CPortalAreaData& data = **mData;
    const rstl::vector< ushort >& portalIndices = data.GetPortalIndices();
    const rstl::vector< ushort >& volumeIndices = data.GetVolumeIndices();
    for (int i = static_cast< short >(volume.mVolumeData->mPortalIndexStart);
         portalIndices[i] != 0xffff; ++i) {
      const CPortalAreaData::SPortal& portal =
          data.GetPortals()[static_cast< short >(portalIndices[i])];
      const rstl::pair< bool, CFrustumPlanes > clipped = ClipPortal(camera, frustum, portal);
      if (clipped.first) {
        for (int j = static_cast< short >(portal.mVolumeIndexStart); volumeIndices[j] != 0xffff;
             ++j) {
          BuildVisibleActorList(mgr, camera, static_cast< short >(volumeIndices[j]), clipped.second,
                                skipPortals, portal.GetCenterPoint());
        }
      }
    }
  }
  --sPortalTraversalDepth;
}

static CUnitVector3f TriangleNormal(const CVector3f& a, const CVector3f& b, const CVector3f& c) {
  return CUnitVector3f(CVector3f::Cross(b - a, c - a));
}

static rstl::pair< bool, CFrustumPlanes > ClipPortal(const CGameCamera& camera,
                                                     const CFrustumPlanes& frustum,
                                                     const CPortalAreaData::SPortal& portal) {
  typedef rstl::pair< bool, CFrustumPlanes > Result;
  const CVector3f& position = camera.GetTranslation();
  if (!portal.mPlane.IsFacing(position)) {
    return Result(false, CFrustumPlanes());
  }
  if (!(camera.GetNearClipDistance() <= portal.DistanceToPoint(position))) {
    return Result(true, frustum);
  }

  rstl::reserved_vector< CVector3f, 16 > polygon;
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  for (int i = 0; i < 4; ++i) {
    polygon.push_back(portal.mVertices[i]);
    bounds.AccumulateBounds(portal.mVertices[i]);
  }
  if (!frustum.BoxInFrustumPlanes(bounds)) {
    return Result(false, CFrustumPlanes());
  }

  const rstl::reserved_vector< CPlane, 6 >& planes = frustum.GetPlanes();
  for (int i = 0; i < planes.size(); ++i) {
    if (polygon.empty()) {
      return Result(false, CFrustumPlanes());
    }
    const CPlane& plane = planes[i];
    rstl::reserved_vector< CVector3f, 16 > clipped;
    CVector3f previous = polygon.back();
    float previousDistance = plane.GetHeight(previous);
    for (int j = 0; j < polygon.size(); ++j) {
      const CVector3f& current = polygon[j];
      const float distance = plane.GetHeight(current);
      if (distance <= 0.f) {
        if (distance >= 0.f) {
          clipped.push_back(current);
        } else {
          if (previousDistance > 0.f) {
            const CVector3f delta = current - previous;
            const float t = -plane.GetHeight(previous) / CVector3f::Dot(plane.GetNormal(), delta);
            clipped.push_back(previous + t * delta);
          }
          clipped.push_back(current);
        }
      } else if (previousDistance < 0.f) {
        const CVector3f delta = current - previous;
        const float t = -plane.GetHeight(previous) / CVector3f::Dot(plane.GetNormal(), delta);
        clipped.push_back(previous + t * delta);
      }
      previous = current;
      previousDistance = distance;
    }
    polygon = clipped;
  }

  float minX = FLT_MAX;
  float maxX = -FLT_MAX;
  float minY = FLT_MAX;
  float maxY = -FLT_MAX;
  for (int i = 0; i < polygon.size(); ++i) {
    const CVector3f screen = camera.ConvertToScreenSpace(polygon[i]);
    minX = rstl::min_val(minX, screen.GetX());
    maxX = rstl::max_val(maxX, screen.GetX());
    maxY = rstl::max_val(maxY, screen.GetY());
    minY = rstl::min_val(minY, screen.GetY());
  }
  minX = CMath::Clamp(-1.f, minX, 1.f);
  maxX = CMath::Clamp(-1.f, maxX, 1.f);
  minY = CMath::Clamp(-1.f, minY, 1.f);
  maxY = CMath::Clamp(-1.f, maxY, 1.f);
  if (CMath::AbsF(maxX - minX) < 1.e-5f || CMath::AbsF(minY - maxY) < 1.e-5f) {
    return Result(false, CFrustumPlanes());
  }

  const float nearZ = camera.GetNearClipDistance();
  const CMatrix4f inverse = camera.GetPerspectiveMatrix().GetInverse();
  const CTransform4f& xf = camera.GetTransform();
  const CVector3f lowerRight = xf * inverse.MultiplyOneOverW(CVector3f(maxX, minY, nearZ));
  const CVector3f upperLeft = xf * inverse.MultiplyOneOverW(CVector3f(minX, maxY, nearZ));
  const CVector3f upperRight = xf * inverse.MultiplyOneOverW(CVector3f(maxX, maxY, nearZ));
  const CVector3f lowerLeft = xf * inverse.MultiplyOneOverW(CVector3f(minX, minY, nearZ));

  rstl::reserved_vector< CPlane, 6 > resultPlanes;
  resultPlanes.push_back(planes[0]);
  resultPlanes.push_back(CPlane(position, TriangleNormal(position, lowerRight, upperRight)));
  resultPlanes.push_back(CPlane(position, TriangleNormal(position, upperLeft, lowerLeft)));
  resultPlanes.push_back(CPlane(position, TriangleNormal(position, upperRight, upperLeft)));
  resultPlanes.push_back(CPlane(position, TriangleNormal(position, lowerLeft, lowerRight)));
  if (planes.size() > 5) {
    resultPlanes.push_back(planes[5]);
  }
  return Result(true, CFrustumPlanes(resultPlanes));
}
