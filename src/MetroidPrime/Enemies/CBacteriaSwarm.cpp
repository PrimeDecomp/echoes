#include "MetroidPrime/Enemies/CBacteriaSwarm.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrBacteriaSwarm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "WorldFormat/CAreaOctTree.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "rstl/math.hpp"

#include <float.h>

CTransform4f CBacteriaSwarm::ShortestRotationArcWrapped(const CVector3f& a, const CVector3f& b,
                                                        const CRelAngle& angle) {
  const float dot = CVector3f::Dot(a, b);
  if (close_enough(dot, 1.f)) {
    return CTransform4f::Identity();
  }
  if (dot > -0.99981f) {
    return CQuaternion::ShortestRotationArcClamped(a, b, angle).BuildTransform4f();
  }
  if (!(a == CVector3f::Right()) && !(b == CVector3f::Right())) {
    return CQuaternion::AxisAngle(CUnitVector3f(CVector3f::Cross(a, CVector3f::Right())), angle)
        .BuildTransform4f();
  }
  return CQuaternion::AxisAngle(CUnitVector3f(CVector3f::Cross(a, CVector3f::Up())), angle)
      .BuildTransform4f();
}

CBacteriaSwarm::CBoid::CBoid(const CTransform4f& xf, uint index)
: mTransform(xf)
, mVelocity(0.f, 0.f, 0.f)
, mAmbientLighting(0.3f, 0.3f, 0.3f, 1.f)
, mNext(nullptr)
, mSurface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f), CVector3f(0.f, 0.f, 1.f), ~0)
, mSpeed(0.f)
, mColorBlend(0.f)
, mIndex(index)
, mActive(false)
, mInFrustum(false)
, mInSafeZone(false)
, mPursuingPlayer(false) {}

CBacteriaSwarm::CBacteriaSwarm(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    const CVector3f& boundingBoxExtent, const CTransform4f& xf, CActorParameters actorParameters,
    const CBasicSwarmData& data, float surfaceStickPriority, float containmentPriority,
    float patrolTurnSpeed, float avoidSafeZoneTurnSpeed, float patrolSpeed,
    float safeZoneEscapeSpeed, float playerPursuitSpeed, float acceleration, float deceleration,
    CAssetId particleEffect, const CColor& patrolColor, const CColor& pursuitColor,
    float colorChangeTime, float minPatrolSoundTime, float maxPatrolSoundTime,
    float patrolSoundWeight, float minPursuitSoundTime, float maxPursuitSoundTime,
    float pursuitSoundWeight, ushort patrolSound, ushort pursuitSound, float soundFallOff,
    float maxAudibleDistance, uchar minVolume, uchar maxVolume, const CStaticRes& scanModel,
    bool spawnInstantly, bool unknownFlag)
: CActor(uid, name, info, 0, xf,
         scanModel.GetId() == kInvalidAssetId ? CModelData(CModelData::None())
                                              : CModelData(scanModel),
         CMaterialList(kMT_Scannable, kMT_Trigger, kMT_NonSolidDamageable, kMT_RadarObject),
         actorParameters, kInvalidUniqueId)
, mAabox(CVector3f(0.f, 0.f, 0.f), CVector3f(0.f, 0.f, 0.f))
, mThinkCounter(0)
, mOccludedTimer(5.f)
, mBoundingBoxExtent(boundingBoxExtent)
, mSeparationRadius(data.mInfluenceRadius)
, mCohesionMagnitude(data.mCohesionPriority)
, mSeparationMagnitude(data.mSeparationPriority)
, mAttractionMagnitude(data.mPlayerAttractPriority)
, mAttractionRadius(data.mPlayerAttractDistance)
, mPartitionedBoidLists(125, nullptr)
, mOutlierBoidList(nullptr)
, mBoidGenRate(data.mSpawnSpeed)
, mBoidGenCooldownTimer(0.f)
, mDamageCooldownTimer(0.f)
, mDamageCooldown(data.mDamageWaitTime)
, mBoidRadius(data.mCollisionRadius)
, mTouchRadius(data.mTouchRadius)
, mPlayerTouchRadius(data.mDamageRadius)
, mDamage(data.mContactDamage)
, mNumBoids(data.mCount)
, mMaxCreatedBoids(data.mMaxCount)
, mCreatedBoids(0)
, mSurfaceProbeScale(1.5f)
, mSafeZoneAvoidance(data.mSafeZoneAvoidancePriority)
, mSurfaceStickPriority(surfaceStickPriority)
, mContainmentPriority(containmentPriority)
, mPatrolTurnSpeed((M_PIF / 180.f) * patrolTurnSpeed)
, mAvoidSafeZoneTurnSpeed((M_PIF / 180.f) * avoidSafeZoneTurnSpeed)
, mPatrolSpeed(patrolSpeed)
, mSafeZoneEscapeSpeed(safeZoneEscapeSpeed)
, mPlayerPursuitSpeed(playerPursuitSpeed)
, mAcceleration(acceleration)
, mDeceleration(deceleration)
, mColorChangeTime(colorChangeTime)
, mPatrolColor(patrolColor)
, mPursuitColor(pursuitColor)
, mNumDeathParticles(data.mNumDeathParticles)
, mPatrolSound(patrolSound)
, mMinPatrolSoundTime(minPatrolSoundTime)
, mMaxPatrolSoundTime(maxPatrolSoundTime)
, mNextPatrolSoundTime(minPatrolSoundTime)
, mPatrolSoundWeight(patrolSoundWeight)
, mPatrolSoundTimer(0.f)
, mPursuitSound(pursuitSound)
, mMinPursuitSoundTime(minPursuitSoundTime)
, mMaxPursuitSoundTime(maxPursuitSoundTime)
, mNextPursuitSoundTime(minPursuitSoundTime)
, mPursuitSoundWeight(pursuitSoundWeight)
, mPursuitSoundTimer(0.f)
, mSoundFallOff(soundFallOff)
, mMaxAudibleDistance(maxAudibleDistance)
, mMinVolume(minVolume)
, mMaxVolume(maxVolume)
, mLockOnIndex(-1)
, mLastOrbitPosition(CVector3f::Zero())
, mLockOnBlendStart(CVector3f::Zero())
, mLockOnBlend(0.f)
, mScanModelData(scanModel.GetId() == kInvalidAssetId ? CModelData(CModelData::None())
                                                      : CModelData(scanModel))
, mBlendingLockOn(false)
, mSpawnInstantly(spawnInstantly)
, x504_26_(unknownFlag)
, mAnyPatrolling(false)
, mAnyPursuing(false) {
  if (data.mDeathParticleEffect != kInvalidAssetId) {
    mDeathParticleDescription =
        rstl::optional_object< TLockedToken< CGenDescription > >(TLockedToken< CGenDescription >(
            gpSimplePool->GetObj(SObjectTag('PART', data.mDeathParticleEffect))));
    mDeathParticleGenerator = rs_new CElementGen(*mDeathParticleDescription);
    mDeathParticleGenerator->SetParticleEmission(false);
  }
  if (particleEffect != kInvalidAssetId) {
    mBoidParticleDescription = rstl::optional_object< TLockedToken< CGenDescription > >(
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particleEffect))));
    mBoidParticleGenerator = rs_new CElementGen(*mBoidParticleDescription);
    mBoidParticleGenerator->SetParticleEmission(false);
  }
}

CBacteriaSwarm::~CBacteriaSwarm() {}

void CBacteriaSwarm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Create:
    mBoids.reserve(mNumBoids);
    for (int i = 0; i < mBoids.capacity(); ++i) {
      mBoids.push_back_unsafe(CBoid(CTransform4f::Identity(), i));
    }
    AddDoorRepulsors(mgr);
    SetDrawShadow(false);
    AddMaterial(kMT_Scannable, mgr);
    break;
  case kSM_AreaLoaded:
    if (mSpawnInstantly) {
      uint count = mBoids.size();
      for (uint i = 0; i < count; ++i) {
        const CBoid& boid = mBoids[i];
        if (!boid.mActive) {
          CreateBoid(mgr, i, true);
          ++mCreatedBoids;
        }
      }
    }
    break;
  default:
    break;
  }
}

void CBacteriaSwarm::CreateBoid(CStateManager& mgr, int index, bool forceParticle) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f center = bounds.GetCenterPoint();
  const CVector3f size = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const float signX = mgr.Random()->Float() > 0.5f ? 1.f : -1.f;
  const float signY = mgr.Random()->Float() > 0.5f ? 1.f : -1.f;
  const float signZ = mgr.Random()->Float() > 0.5f ? 1.f : -1.f;
  const float z = 0.5f * size.GetZ() * signZ * mgr.Random()->Float() + center.GetZ();
  const float y = 0.5f * size.GetY() * signY * mgr.Random()->Float() + center.GetY();
  const float x = 0.5f * size.GetX() * signX * mgr.Random()->Float() + center.GetX();
  const CVector3f position(x, y, z);
  const CRelAngle angle = CRelAngle::FromDegrees(360.f * mgr.Random()->Float());
  const CMatrix3f rotation = CMatrix3f::RotateZ(angle);
  const TUniqueId waypointId = GetWaypointForState(kSS_Patrol, mgr);
  const CScriptWaypoint* waypoint =
      TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId));
  const CTransform4f xf = (waypoint && waypoint->GetActive()) ? waypoint->GetTransform()
                                                              : CTransform4f(rotation, position);
  mBoids[index].mTransform = xf;
  mBoids[index].mActive = true;
  mBoids[index].mVelocity = CVector3f::Zero();
  mBoids[index].mSpeed = mPatrolSpeed;
  if (mBoidParticleGenerator.get() && forceParticle) {
    mBoidParticleGenerator->SetTranslation(mBoids[index].GetTranslation());
    mBoidParticleGenerator->ForceParticleCreation(1);
  }
}

void CBacteriaSwarm::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox bounds = GetBoundingBox();
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

CAABox CBacteriaSwarm::GetBoundingBox() const {
  const CVector3f extent(0.5f * mBoundingBoxExtent.GetX(), 0.5f * mBoundingBoxExtent.GetY(),
                         0.5f * mBoundingBoxExtent.GetZ());
  const CAABox bounds(-extent, extent);
  return bounds.GetTransformedAABox(GetTransform());
}

rstl::optional_object< CAABox > CBacteriaSwarm::GetTouchBounds() const { return mAabox; }

CVector3f CBacteriaSwarm::ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                              const CVector3f& normal) {
  return point - CVector3f::Dot(point - planePoint, normal) * normal;
}

CVector3f CBacteriaSwarm::ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal) {
  return point - CVector3f::Dot(point, normal) * normal;
}

bool CBacteriaSwarm::PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos,
                                    const CPlane& plane) {
  const CVector3f projected = ProjectPointToPlane(pos, surface.GetVert(0), plane.GetNormal());
  for (int i = 0; i < 3; ++i) {
    const int next = i + 2;
    const int previous = next == 2 ? next : next - 3;
    const CVector3f edge2 = surface.GetVert(previous) - surface.GetVert(i);
    const CVector3f edge1 = projected - surface.GetVert(i);
    const CVector3f cross = CVector3f::Cross(edge1, edge2);
    if (CVector3f::Dot(plane.GetNormal(), cross) < 0.f) {
      return false;
    }
  }
  return true;
}

CCollisionSurface CBacteriaSwarm::FindBestCollisionInBox(CStateManager& mgr, const CVector3f& pos) {
  CCollisionSurface result(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                           CVector3f(0.f, 0.f, 1.f), ~0);
  const CAABox& bounds = GetBoundingBox();
  const CVector3f extent = 0.5f * (bounds.GetMaxPoint() - bounds.GetMinPoint());
  for (float scale = 0.1f; scale < 1.f; scale += 0.1f) {
    const CAABox searchBounds(pos - extent * scale, pos + extent * scale);
    CAreaCollisionCache cache(searchBounds);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    if (FindBestSurface(cache, pos, 2.f * (extent * scale).Magnitude(), result)) {
      return result;
    }
  }
  return result;
}

bool CBacteriaSwarm::FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos, float radius,
                                     CCollisionSurface& out) {
  bool found = false;
  float minDistance = radius;
  CSphere sphere(pos, radius);
  for (int i = 0; i < int(cache.GetNumCaches()); ++i) {
    const CMetroidAreaCollider::COctreeLeafCache& leafCache = cache.GetOctreeLeafCache(i);
    for (int j = 0; j < leafCache.GetNumLeaves(); ++j) {
      const CAreaOctTree::Node& node = leafCache.GetLeaf(j);
      if (CCollidableSphere::Sphere_AABox_Bool(sphere, node.GetBoundingBox())) {
        const CAreaOctTree::TriListReference triangles = node.GetTriangleArray();
        const CAreaOctTree& tree = node.GetOwner();
        const int triangleCount = triangles.GetSize();
        for (int k = 0; k < triangleCount; ++k) {
          const CCollisionSurface surface(tree.GetTriangle(triangles.GetAt(k)));
          if (!CMaterialList(surface.GetSurfaceFlags()).HasMaterial(kMT_AIPassthrough)) {
            const CPlane plane = surface.GetPlane();
            const float distance = CMath::AbsF(plane.GetHeight(pos));
            if (distance < minDistance && PointOnSurface(surface, pos, plane)) {
              sphere = CSphere(pos, distance);
              out = surface;
              found = true;
              minDistance = distance;
            }
          }
        }
      }
    }
  }
  return found;
}

void CBacteriaSwarm::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt,
                                CBoid& boid, int partitionIndex) {
  rstl::reserved_vector< CBoid*, 50 > nearList;
  BuildBoidNearList(boid, mSeparationRadius, nearList);
  CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                            CVector3f(0.f, 0.f, 1.f), ~0);
  bool found = false;
  const float radius = 2.f * mBoidRadius;
  const float boidRadius = mBoidRadius;
  const float speed = boid.mVelocity.Magnitude();
  float distance = speed * dt;
  CVector3f pos = boid.GetTranslation();
  const CVector3f step = boidRadius * ((1.f / speed) * -boid.mVelocity);
  while (distance >= 0.f && !found) {
    const CVector3f predicted = pos + mSurfaceProbeScale * (dt * boid.mVelocity);
    if (FindBestSurface(cache, predicted, radius, surface)) {
      boid.mTransform = ShortestRotationArcWrapped(boid.GetTransform().GetUp(), surface.GetNormal(),
                                                   CRelAngle::FromRadians(M_PIF))
                            .MultiplyIgnoreTranslation(boid.GetTransform());
      found = true;
    }
    distance -= boidRadius;
    pos += step;
  }
  CVector3f ahead = boid.GetTransform().GetForward();
  if (found) {
    ahead += mSurfaceStickPriority * surface.GetPlane().GetNormal();
  }
  ApplySteeringBehaviors(mgr, boid, ahead, nearList);
  float newSpeed = boid.mSpeed;
  if (boid.mInSafeZone) {
    newSpeed = rstl::min_val(mSafeZoneEscapeSpeed, newSpeed + mAcceleration);
  } else if (boid.mPursuingPlayer) {
    newSpeed = rstl::min_val(mPlayerPursuitSpeed, newSpeed + mAcceleration);
  } else {
    newSpeed = rstl::max_val(mPatrolSpeed, newSpeed - mDeceleration);
  }
  boid.mSpeed = newSpeed;
  float turnSpeed = mPatrolTurnSpeed;
  if (boid.mInSafeZone || boid.mTouchingSafeZone) {
    turnSpeed = mAvoidSafeZoneTurnSpeed;
  }
  if (boid.mPursuingPlayer) {
    boid.mColorBlend = rstl::min_val(mColorChangeTime, boid.mColorBlend + dt);
    mAnyPursuing = true;
  } else {
    boid.mColorBlend = rstl::max_val(0.f, boid.mColorBlend - dt);
    mAnyPatrolling = true;
  }
  boid.mTransform =
      ShortestRotationArcWrapped(boid.GetTransform().GetForward(), ahead.AsNormalized(),
                                 CRelAngle::FromRadians(turnSpeed * dt))
          .MultiplyIgnoreTranslation(boid.GetTransform());
}

void CBacteriaSwarm::ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                            const rstl::reserved_vector< CBoid*, 50 >& nearList) {
  boid.mInSafeZone = false;
  boid.mTouchingSafeZone = false;
  boid.mPursuingPlayer = false;
  for (int i = 0; i < 7; ++i) {
    if (boid.mInSafeZone || boid.mTouchingSafeZone) {
      break;
    }
    switch (i) {
    case 0:
      for (rstl::vector< CRepulsor >::iterator it = mDoorRepulsors.begin();
           it != mDoorRepulsors.end(); ++it) {
        if ((it->mCenter - boid.GetTranslation()).MagSquared() < it->mMagnitude * it->mMagnitude) {
          ApplySeparation(boid, it->mCenter, it->mMagnitude, 4.5f, ahead);
        }
      }
      break;
    case 1:
      ApplySafeZoneAvoidance(mgr, boid, nearList, ahead);
      break;
    case 2:
      ApplyPlayerAttraction(boid, mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f), *mgr.GetPlayer(0),
                            ahead, mAttractionRadius, mAttractionMagnitude);
      break;
    case 3:
      ApplyBoundsAvoidance(boid, nearList, ahead);
      break;
    case 5:
      ApplySeparation(boid, nearList, ahead);
      break;
    case 6:
      ApplyCohesion(boid, nearList, ahead);
      break;
    default:
      break;
    }
    if (ahead.MagSquared() >= 9.f) {
      break;
    }
  }
}

void CBacteriaSwarm::AddDoorRepulsors(CStateManager& mgr) {
  CObjectList& objects = mgr.ObjectListById(kOL_PhysicsActor);
  int count = 0;
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(objects[i])) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        ++count;
      }
    }
  }
  mDoorRepulsors.reserve(count);
  for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
    if (CScriptDoor* door = TCastToPtr< CScriptDoor >(objects[i])) {
      if (door->GetCurrentAreaId() == GetCurrentAreaId()) {
        rstl::optional_object< CAABox > bounds = door->GetTouchBounds();
        if (bounds.valid()) {
          float diagonal = (bounds->GetMinPoint() - bounds->GetMaxPoint()).Magnitude();
          mDoorRepulsors.push_back_unsafe(CRepulsor(bounds->GetCenterPoint(), 0.75f * diagonal));
        }
      }
    }
  }
}

void CBacteriaSwarm::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mAnyPatrolling = false;
  mAnyPursuing = false;
  if (!GetActive()) {
    return;
  }
  SetTransformDirty();
  mBoidGenCooldownTimer -= dt;
  mDamageCooldownTimer -= dt;
  ++mThinkCounter;
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    if (mOccludedTimer > 0.f) {
      mOccludedTimer -= dt;
    }
    if (mOccludedTimer <= 0.f) {
      return;
    }
    if (mThinkCounter & 2) {
      return;
    }
  } else {
    mOccludedTimer = 7.f;
  }
  UpdateParticles(dt);
  int prevLockOnIndex = mLockOnIndex;
  mLockOnIndex = GetLockOnIndex(mgr);
  UpdateLockOnBlend(prevLockOnIndex, mLockOnIndex, dt);
  const CPlayerState* playerState = mgr.GetPlayer(0)->GetPlayerState();
  if (playerState->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      playerState->GetTransitioningVisor() == CPlayerState::kPV_Scan && mLockOnIndex != -1) {
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
    mLastOrbitPosition = mBoids[mLockOnIndex].GetTranslation();
  } else {
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  }
  while ((mMaxCreatedBoids == 0 || mCreatedBoids < mMaxCreatedBoids) &&
         mBoidGenCooldownTimer <= 0.f) {
    bool created = false;
    for (int i = 0; i < mBoids.size(); ++i) {
      if (!mBoids[i].mActive) {
        CreateBoid(mgr, i, true);
        ++mCreatedBoids;
        mBoidGenCooldownTimer += 1.f / mBoidGenRate;
        created = true;
        break;
      }
    }
    if (!created) {
      mBoidGenCooldownTimer += 1.f / mBoidGenRate;
      break;
    }
  }
  UpdatePartition();
  const CAABox bounds = GetBoundingBox();
  int count = 0;
  mAabox = GetBoundingBox();
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        CBoid* boid = mPartitionedBoidLists[rowIndex + z * 25];
        if (boid != nullptr) {
          CAreaCollisionCache cache = GetAreaCollisionCacheForPartition(x, y, z);
          if (ShouldBuildAreaCollisionCacheForPartition(rowIndex + z * 25)) {
            CGameCollision::BuildAreaCollisionCache(mgr, cache);
          }
          for (; boid != nullptr; boid = boid->mNext) {
            ++count;
            if (boid->mActive) {
              mAabox.AccumulateBounds(boid->GetTranslation());
            }
            if ((mThinkCounter & 1) == (count & 1) && boid->mActive) {
              UpdateBoid(cache, mgr, dt, *boid, rowIndex + z * 25);
            }
          }
        }
      }
    }
  }
  for (CBoid* boid = mOutlierBoidList; boid != nullptr; boid = boid->mNext) {
    ++count;
    if (boid->mActive) {
      mAabox.AccumulateBounds(boid->GetTranslation());
    }
    if ((mThinkCounter & 1) == (count & 1) && boid->mActive) {
      const float margin = 1.5f + (0.5f + mBoidRadius);
      const CVector3f extent(margin, margin, margin);
      const CAABox boidBounds(boid->GetTranslation() - extent, boid->GetTranslation() + extent);
      CAreaCollisionCache cache(boidBounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      UpdateBoid(cache, mgr, dt, *boid, -1);
    }
  }
  UpdateSwarmAnimations(mgr, dt);
  UpdateAllBoidMovement(dt);
  if (mAnyPatrolling) {
    mPatrolSoundTimer += dt;
    if (mPatrolSoundTimer >= mNextPatrolSoundTime) {
      mPatrolSoundTimer = 0.f;
      mNextPatrolSoundTime =
          mMinPatrolSoundTime + (mMaxPatrolSoundTime - mMinPatrolSoundTime) * mgr.Random()->Float();
    }
  }
  if (mAnyPursuing) {
    mPursuitSoundTimer += dt;
    if (mPursuitSoundTimer >= mNextPursuitSoundTime) {
      mPursuitSoundTimer = 0.f;
      mNextPursuitSoundTime = mMinPursuitSoundTime +
                              (mMaxPursuitSoundTime - mMinPursuitSoundTime) * mgr.Random()->Float();
    }
  }
  if (mBoidParticleGenerator.get()) {
    const CVector3f playerPos = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
    const int particleCount =
        rstl::min_val(mBoidParticleGenerator->GetParticleCount(), int(mBoids.size()));
    for (int i = 0; i < particleCount; ++i) {
      CBoid& boid = mBoids[i];
      if (boid.mActive) {
        CElementGen::CParticle& particle = mBoidParticleGenerator->mParticles[i];
        particle.mPos = boid.GetTranslation();
        particle.mPrevPos = boid.GetTranslation();
        particle.mEndFrame = 0;
        particle.mColor =
            CColor::Lerp(mPatrolColor, mPursuitColor, boid.mColorBlend / mColorChangeTime);
        const float distanceSquared = (playerPos - boid.GetTranslation()).MagSquared();
        if (mAnyPatrolling && mPatrolSoundTimer == 0.f && !boid.mPursuingPlayer &&
            mgr.Random()->Float() <= mPatrolSoundWeight) {
          PlayBoidSound(mPatrolSound, boid.GetTranslation(), distanceSquared);
        }
        if (mAnyPursuing && mPursuitSoundTimer == 0.f && boid.mPursuingPlayer &&
            mgr.Random()->Float() <= mPursuitSoundWeight) {
          PlayBoidSound(mPursuitSound, boid.GetTranslation(), distanceSquared);
        }
      }
    }
  }
}

void CBacteriaSwarm::UpdateAllBoidMovement(float dt) {
  uint count = mBoids.size();
  for (uint i = 0; i < count; ++i) {
    MoveBoid(mBoids[i], mBoids[i].mSpeed * CVector3f::Forward(), dt);
  }
}

void CBacteriaSwarm::UpdateSwarmAnimations(CStateManager& mgr, float dt) {}

void CBacteriaSwarm::MoveBoid(CBoid& boid, const CVector3f& offsetDelta, float dt) {
  if (boid.mActive) {
    boid.mVelocity = (1.f / dt) * boid.GetTransform().Rotate(offsetDelta);
    boid.mTransform.AddTranslation(dt * boid.mVelocity);
  }
}

void CBacteriaSwarm::UpdatePartition() {
  mPartitionedBoidLists.clear();
  for (int i = 0; i < 125; ++i) {
    mPartitionedBoidLists.push_back(nullptr);
  }
  mOutlierBoidList = nullptr;
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size = extent / 5.f;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      const CVector3f& pos = it->GetTranslation();
      const CVector3f delta = pos - bounds.GetMinPoint();
      const int x = CCast::ToInt32(delta.GetX() / size.GetX());
      const int y = CCast::ToInt32(delta.GetY() / size.GetY());
      const int z = CCast::ToInt32(delta.GetZ() / size.GetZ());
      const int index = x + 5 * y + 25 * z;
      if (index < 0 || index >= 125 || x < 0 || x >= 5 || y < 0 || y >= 5 || z < 0 || z >= 5) {
        it->mNext = mOutlierBoidList;
        mOutlierBoidList = it.get_pointer();
      } else {
        it->mNext = mPartitionedBoidLists[index];
        mPartitionedBoidLists[index] = it.get_pointer();
      }
    }
  }
}

CBacteriaSwarm::CBoid* CBacteriaSwarm::GetListAt(const CVector3f& pos) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f delta = pos - bounds.GetMinPoint();
  const int index = CCast::ToInt32(delta.GetX() / (bounds.GetWidth() / 5.f)) +
                    CCast::ToInt32(delta.GetY() / (bounds.GetHeight() / 5.f)) * 5 +
                    CCast::ToInt32(delta.GetZ() / (bounds.GetDepth() / 5.f)) * 25;
  if (index < 0 || index >= 125) {
    return mOutlierBoidList;
  }
  return mPartitionedBoidLists[index];
}

CAABox CBacteriaSwarm::BoxForPosition(int x, int y, int z, float margin) const {
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size(extent.GetX() / 5.f, extent.GetY() / 5.f, extent.GetZ() / 5.f);
  return CAABox(CVector3f(x * size.GetX() + bounds.GetMinPoint().GetX() - margin,
                          y * size.GetY() + bounds.GetMinPoint().GetY() - margin,
                          z * size.GetZ() + bounds.GetMinPoint().GetZ() - margin),
                CVector3f((x + 1) * size.GetX() + bounds.GetMinPoint().GetX() + margin,
                          (y + 1) * size.GetY() + bounds.GetMinPoint().GetY() + margin,
                          (z + 1) * size.GetZ() + bounds.GetMinPoint().GetZ() + margin));
}

void CBacteriaSwarm::PreRender(CStateManager& mgr) {
  bool active = false;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      it->mInFrustum = mgr.GetFrustumPlanes().SphereInFrustumPlanes(
          CSphere(it->GetTranslation(), 2.f * mBoidRadius));
      active = true;
    } else {
      it->mInFrustum = false;
    }
  }
  SetPreRenderClipped(!active);
}

void CBacteriaSwarm::HardwareLight(const CStateManager& mgr, const CAABox& bounds) const {
  CActorLights lights(8, CVector3f::Zero(), 4, 4);
  lights.SetNeedsRelight(true);
  lights.SetCastShadows(false);
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), bounds);
  lights.BuildDynamicLightList(mgr, bounds);
  lights.ActivateLights();
}

CColor CBacteriaSwarm::SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const {
  CActorLights lights(8, CVector3f::Zero(), 4, 4);
  lights.SetNeedsRelight(true);
  lights.SetCastShadows(false);
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), bounds);
  lights.BuildDynamicLightList(mgr, bounds);
  CColor result = lights.GetAmbientColor();
  const CVector3f center = bounds.GetCenterPoint();
  for (uint i = 0; i < lights.GetActiveLightCount(); ++i) {
    const CLight& light = lights.GetLight(i);
    const float distance = (light.GetPosition() - center).Magnitude();
    const float attenuation = rstl::min_val(
        1.f, 1.f / (distance * (distance * light.GetAttenuationQuadratic()) +
                    (distance * light.GetAttenuationLinear() + light.GetAttenuationConstant())));
    result =
        CColor::Add(result, CColor::Lerp(CColor::Black(), light.GetColor(), 0.8f * attenuation));
  }
  return result;
}

bool CBacteriaSwarm::CanRenderUnsorted(const CStateManager& mgr) const { return true; }

void CBacteriaSwarm::RenderBoid(const CBoid* boid, uint& drawMask, const CModelFlags& flags) const {
}

void CBacteriaSwarm::AddToRenderer(const CStateManager& mgr) const {
  if (GetActive()) {
    RenderParticles();
    if (!GetPreRenderClipped()) {
      EnsureRendered(mgr);
    }
  }
}

void CBacteriaSwarm::Render(const CStateManager& mgr) const {
  const int alpha = GetRenderAlphaBufferAlpha(mgr);
  if (alpha != -1) {
    gpRender->SetDestinationAlpha(alpha);
  }
  uint drawMask = -1;
  const CModelFlags flags(CModelFlags::kT_Opaque, 1.f);
  const uint lights = CGraphics::GetLightMask();
  CGX::SetChanCtrl(CGX::Channel0, GX_FALSE, GX_SRC_REG, GX_SRC_REG,
                   static_cast< GXLightID >(lights), lights ? GX_DF_CLAMP : GX_DF_NONE,
                   lights ? GX_AF_SPOT : GX_AF_NONE);
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        const int index = rowIndex + z * 25;
        CBoid* boid = mPartitionedBoidLists[index];
        if (boid != nullptr) {
          const CAABox bounds = BoxForPosition(x, y, z, 0.f);
          if ((index & 3) == (mThinkCounter & 3)) {
            const CColor color = SoftwareLight(mgr, bounds);
            for (CBoid* it = boid; it != nullptr; it = it->mNext) {
              if (it->mActive) {
                it->mAmbientLighting = CColor::Lerp(it->mAmbientLighting, color, 0.3f);
              }
            }
          }
          for (; boid != nullptr; boid = boid->mNext) {
            if (boid->mInFrustum && boid->mActive) {
              RenderBoid(boid, drawMask, flags);
            }
          }
        }
      }
    }
  }
  CBoid* boid = mOutlierBoidList;
  int index = 0;
  for (; boid != nullptr; boid = boid->mNext) {
    ++index;
    if (boid->mInFrustum && boid->mActive) {
      const CVector3f pos = boid->GetTranslation();
      const CVector3f extent(mBoidRadius, mBoidRadius, mBoidRadius);
      const CAABox bounds = CAABox(pos - extent, pos + extent);
      if ((index & 3) == (mThinkCounter & 3)) {
        const CColor color = SoftwareLight(mgr, bounds);
        if (boid->mActive) {
          boid->mAmbientLighting = CColor::Lerp(boid->mAmbientLighting, color, 0.3f);
        }
      }
      RenderBoid(boid, drawMask, flags);
    }
  }
  if (mBoidParticleGenerator.get()) {
    mBoidParticleGenerator->Render();
  }
  if (alpha != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

void CBacteriaSwarm::BuildBoidNearList(const CBoid& boid, float radius,
                                       rstl::reserved_vector< CBoid*, 50 >& nearList) {
  CBoid* other = GetListAt(boid.GetTranslation());
  const CVector3f pos = boid.GetTranslation();
  while (other != nullptr && nearList.size() < 50) {
    const float distance = (other->GetTranslation() - pos).MagSquared();
    if (distance != 0.f && distance < radius) {
      nearList.push_back(other);
    }
    other = other->mNext;
  }
}

void CBacteriaSwarm::ApplySeparation(CBoid& boid,
                                     const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                     CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f closest(0.f, 0.f, 0.f);
    float minDistance = FLT_MAX;
    for (rstl::reserved_vector< CBoid*, 50 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      const CVector3f delta = boid.GetTranslation() - (*it)->GetTranslation();
      const float distance = delta.MagSquared();
      if (distance != 0.f && distance < minDistance) {
        minDistance = distance;
        closest = (*it)->GetTranslation();
      }
    }
    ApplySeparation(boid, closest, mSeparationRadius, mSeparationMagnitude, ahead);
  }
}

void CBacteriaSwarm::ApplySeparation(CBoid& boid, const CVector3f& pos, float radius,
                                     float magnitude, CVector3f& ahead) {
  const CVector3f delta = boid.GetTranslation() - pos;
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distance < radiusSquared) {
      const float factor = 1.f - distance / radiusSquared;
      ahead += factor * delta.AsNormalized() * magnitude;
    }
  }
}

void CBacteriaSwarm::ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                   CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f center(0.f, 0.f, 0.f);
    for (rstl::reserved_vector< CBoid*, 50 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      center += (*it)->GetTranslation();
    }
    center = (1.f / nearList.size()) * center;
    ApplyCohesion(boid, center, mSeparationRadius, mCohesionMagnitude, ahead);
  }
}

void CBacteriaSwarm::ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                                   CVector3f& ahead) {
  const CVector3f delta = pos - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    const float factor = distance > radiusSquared ? 1.f : distance / radiusSquared;
    ahead += factor * delta.AsNormalized() * magnitude;
  }
}

void CBacteriaSwarm::ApplyPlayerAttraction(CBoid& boid, const CVector3f& pos, const CPlayer& player,
                                           CVector3f& ahead, float radius, float magnitude) {
  CVector3f delta = pos - boid.GetTranslation();
  if (player.GetPlayerState()->HasPowerUp(CPlayerState::kIT_LightSuit)) {
    delta = boid.GetTranslation() - pos;
  }
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    const float factor = distance > radiusSquared ? 0.f : 1.f - distance / radiusSquared;
    ahead += magnitude * (factor * delta.AsNormalized());
    if (factor > 0.f) {
      boid.mPursuingPlayer = true;
    }
  }
}

void CBacteriaSwarm::ApplyBoundsAvoidance(CBoid& boid,
                                          const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                          CVector3f& ahead) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f future = boid.GetTranslation() + 1.f * boid.mVelocity;
  if (!bounds.PointInside(future)) {
    const float priority = mContainmentPriority;
    ahead += priority * (bounds.GetCenterPoint() - future).AsNormalized();
  }
}

void CBacteriaSwarm::ApplySafeZoneAvoidance(CStateManager& mgr, CBoid& boid,
                                            const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                            CVector3f& ahead) {
  const CVector3f pos = boid.GetTranslation();
  const TUniqueId zoneId =
      mgr.GetSafeZoneManager()->SphereTouchingWhichSafeZone(mgr, CSphere(pos, mTouchRadius));
  if (zoneId != kInvalidUniqueId) {
    if (const CScriptSafeZone* zone =
            static_cast< const CScriptSafeZone* >(mgr.GetObjectById(zoneId))) {
      boid.mTouchingSafeZone = true;
      const float radius = zone->GetScale().GetX();
      const CVector3f delta = pos - zone->GetTranslation();
      if (delta.MagSquared() < radius * radius) {
        boid.mInSafeZone = true;
      }
      ahead += mSafeZoneAvoidance * delta.AsNormalized();
    }
  }
}

TUniqueId CBacteriaSwarm::GetWaypointForState(EScriptObjectState state, CStateManager& mgr) {
  rstl::vector< TUniqueId > waypoints;
  waypoints.reserve(8);
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == state && it->msg == kSM_Follow) {
      TUniqueId uid = mgr.GetIdForScript(it->objId);
      const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(uid));
      if (waypoint && waypoint->GetActive()) {
        waypoints.push_back_unsafe(uid);
      }
    }
  }
  int count = waypoints.size();
  if (count > 1) {
    return waypoints[mgr.Random()->Next() % count];
  }
  if (count > 0) {
    return waypoints[0];
  }
  return kInvalidUniqueId;
}

void CBacteriaSwarm::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    float radius = mPlayerTouchRadius;
    if (close_enough(radius, 0.f)) {
      radius = mTouchRadius;
    }
    const CAABox playerBounds = *player->GetTouchBounds();
    for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
      if (it->mActive) {
        const CVector3f extent(radius, radius, radius);
        const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
        if (playerBounds.DoBoundsOverlap(bounds) && mDamageCooldownTimer <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mDamage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mDamageCooldownTimer = mDamageCooldown;
          KillBoid(*it, mgr, 1.f, 1.f);
          break;
        }
      }
    }
  }
}

CVector3f CBacteriaSwarm::FindClosestCell(const CVector3f& pos) const {
  float minDistance = FLT_MAX;
  CVector3f result = CVector3f::Zero();
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        if (mPartitionedBoidLists[rowIndex + z * 25] != nullptr) {
          const CAABox bounds = BoxForPosition(x, y, z, 0.1f);
          const float distance = (bounds.GetCenterPoint() - pos).MagSquared();
          if (distance < minDistance) {
            result = bounds.GetCenterPoint();
            minDistance = distance;
          }
        }
      }
    }
  }
  return result;
}

void CBacteriaSwarm::UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume) {
  int count;
  const CSoundPOINode* nodes = animData.GetSoundPOIList(count);
  if (count > 0 && nodes != nullptr) {
    for (int i = 0; i < count; ++i) {
      const CSoundPOINode& node = nodes[i];
      const int character = node.GetCharacterIndex();
      if (node.GetPoiType() == kPT_Sound &&
          (character == -1 || character == animData.GetCharacterIndex())) {
        const uint soundId = node.GetSoundId();
        const ushort sfx = soundId;
        const int area = GetCurrentAreaId().Value();
        if ((soundId & 0x80000000) == 0) {
          const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
          const CVector3f pos = FindClosestCell(playerPos);
          static float maxDistance = node.GetMaxDistance();
          static float falloff = node.GetFallOff();
          CAudioSys::C3DEmitterParmData params(maxDistance, falloff, 1,
                                               volume < 0 ? 0 : (volume > 127 ? 127 : volume), 20);
          params.mPos = pos;
          params.mDir = CVector3f::Zero();
          params.mSfxId = sfx;
          CSfxManager::AddEmitter(params, area, true, false);
        }
      }
    }
  }
}

CAreaCollisionCache CBacteriaSwarm::GetAreaCollisionCacheForPartition(int x, int y, int z) const {
  return CAreaCollisionCache(BoxForPosition(x, y, z, mBoidRadius + 0.5f));
}

bool CBacteriaSwarm::ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const {
  return true;
}

void CBacteriaSwarm::KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance,
                              float deadChance) {
  AddParticle(boid.GetTransform());
  boid.mActive = false;
  const float deadRoll = mgr.Random()->Float();
  const float deathRattleRoll = mgr.Random()->Float();
  if (deathRattleRoll < deathRattleChance) {
    SendScriptMsgs(kSS_DeathRattle, mgr, kInvalidUniqueId, kSM_None);
  }
  if (deadRoll < deadChance) {
    SendScriptMsgs(kSS_Dead, mgr, kInvalidUniqueId, kSM_None);
  }
  CreateBoid(mgr, boid.mIndex, false);
}

void CBacteriaSwarm::AddParticle(const CTransform4f& xf) {
  if (mDeathParticleGenerator.get()) {
    mDeathParticleGenerator->SetParticleEmission(true);
    mDeathParticleGenerator->SetTranslation(xf.GetTranslation());
    mDeathParticleGenerator->ForceParticleCreation(mNumDeathParticles);
    mDeathParticleGenerator->SetParticleEmission(false);
  }
}

void CBacteriaSwarm::UpdateParticles(float dt) {
  if (mDeathParticleGenerator.get()) {
    mDeathParticleGenerator->Update(dt);
  }
}

void CBacteriaSwarm::RenderParticles() const {
  if (mDeathParticleGenerator.get()) {
    gpRender->AddParticleGen(*mDeathParticleGenerator);
  }
}

void CBacteriaSwarm::PlayBoidSound(ushort sfx, const CVector3f& pos, float distanceSquared) {
  if (distanceSquared < mMaxAudibleDistance * mMaxAudibleDistance) {
    CAudioSys::C3DEmitterParmData parms(mMaxAudibleDistance, mSoundFallOff, 1, mMaxVolume,
                                        mMinVolume);
    parms.mPos = pos;
    parms.mDir = CVector3f::Zero();
    parms.mSfxId = sfx;
    CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), true, false);
  }
}

CVector3f CBacteriaSwarm::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mLockOnIndex == -1) {
    return mLastOrbitPosition;
  }
  if (mBlendingLockOn && dt == 0.f) {
    return (1.f - mLockOnBlend) * mLockOnBlendStart + mLockOnBlend * mLastOrbitPosition;
  }
  return mLastOrbitPosition + dt * mBoids[mLockOnIndex].mVelocity;
}

CVector3f CBacteriaSwarm::GetOrbitPosition(const CStateManager& mgr) const {
  return mLastOrbitPosition;
}

bool CBacteriaSwarm::IsBoidVisibleForLockOn(const CStateManager& mgr, const CBoid& boid,
                                            const CVector3f& cameraPos,
                                            const CVector3f& cameraForward) const {
  const CVector3f delta = boid.GetTranslation() - cameraPos;
  const float distance = delta.Magnitude();
  const float inv = 1.f / distance;
  const CVector3f dir = inv * delta;
  if (CVector3f::Dot(cameraForward, dir) > 0.9238795f) {
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    const CRayCastResult result = mgr.RayStaticIntersection(cameraPos, dir, distance, filter);
    if (!result.IsValid()) {
      return true;
    }
  }
  return false;
}

int CBacteriaSwarm::GetLockOnIndex(CStateManager& mgr) const {
  const CTransform4f cameraXf = mgr.GetCameraManager(0)->GetFirstPersonCamera()->GetTransform();
  const CVector3f cameraPos = cameraXf.GetTranslation();
  const CVector3f cameraForward = cameraXf.GetForward();
  const bool playerOrbiting = mgr.GetPlayer(0)->GetOrbitTargetId() == GetUniqueId();
  if (mLockOnIndex != -1) {
    int result = -1;
    if (mBoids[mLockOnIndex].mActive &&
        IsBoidVisibleForLockOn(mgr, mBoids[mLockOnIndex], cameraPos, cameraForward)) {
      result = mLockOnIndex;
    }
    if (result != -1 && !playerOrbiting) {
      result = FindBestLockOnIndex(mgr);
    }
    return result;
  }
  return FindBestLockOnIndex(mgr);
}

int CBacteriaSwarm::FindBestLockOnIndex(CStateManager& mgr) const {
  float maxDot = 0.5f;
  int index = 0;
  int result = -1;
  float maxDistanceSq = mgr.GetPlayer(0)->GetOrbitMaxTargetDistance();
  maxDistanceSq *= maxDistanceSq;
  const CTransform4f cameraXf = mgr.GetCameraManager(0)->GetFirstPersonCamera()->GetTransform();
  const CVector3f cameraPos = cameraXf.GetTranslation();
  const CVector3f cameraForward = cameraXf.GetForward();
  for (rstl::vector< CBoid >::const_iterator it = mBoids.begin(); it != mBoids.end();
       ++it, ++index) {
    if (it->mActive) {
      const CVector3f delta = it->GetTranslation() - cameraPos;
      if (delta.MagSquared() > maxDistanceSq) {
        continue;
      }
      if (delta.CanBeNormalized()) {
        const float dot = CVector3f::Dot(cameraForward, delta.AsNormalized());
        if (dot > maxDot) {
          result = index;
          maxDot = dot;
        }
      }
    }
  }
  return result;
}

void CBacteriaSwarm::UpdateLockOnBlend(int prevIndex, int newIndex, float dt) {
  if (newIndex >= 0) {
    if (prevIndex >= 0 && prevIndex != newIndex) {
      if (!mBlendingLockOn) {
        mLockOnBlendStart = mLastOrbitPosition;
      }
      mBlendingLockOn = true;
      mLockOnBlend = 0.f;
    }
    if (mBlendingLockOn) {
      mLockOnBlend += 3.f * dt;
      if (mLockOnBlend >= 1.f) {
        mBlendingLockOn = false;
      }
    }
  } else {
    mBlendingLockOn = false;
  }
}

CEntity* LoadBacteriaSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrBacteriaSwarm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrBacteriaSwarm.inc"

  sldrThis.editorProperties.active = sldrThis.active;
  return rs_new CBacteriaSwarm(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.editorProperties.transform.scale,
      LdrToTransform4f(sldrThis.editorProperties), LdrToActorParameters(sldrThis.actorInformation),
      LdrToBasicSwarmData(sldrThis.basicSwarmProperties), sldrThis.unknown_0x4a85a2da,
      sldrThis.containmentPriority, sldrThis.patrolTurnSpeed, sldrThis.avoidSafeZoneTurnSpeed,
      sldrThis.bacteriaPatrolSpeed, sldrThis.bacteriaSafeZoneEscapeSpeed,
      sldrThis.bacteriaPlayerPursuitSpeed, sldrThis.bacteriaAcceleration,
      sldrThis.bacteriaDeceleration, sldrThis.bacteriaParticleEffect, sldrThis.bacteriaPatrolColor,
      sldrThis.bacteriaPlayerPursuitColor, sldrThis.colorChangeTime, sldrThis.minPatrolSoundTime,
      sldrThis.maxPursuitSoundTime, sldrThis.patrolSoundWeight, sldrThis.minPursuitSoundTime,
      sldrThis.maxPursuitSoundTime, sldrThis.pursuitSoundWeight, sldrThis.patrolSound & 0xffff,
      sldrThis.pursuitSound & 0xffff, sldrThis.soundFallOff, sldrThis.maxAudibleDistance,
      sldrThis.minVolume & 0xff, sldrThis.maxVolume & 0xff,
      CStaticRes(sldrThis.bacteriaScanModel, CVector3f(0.f, 0.f, 0.f)), sldrThis.spawnInstantly,
      false);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SBacteriaSwarm_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadBacteriaSwarm;
  SetSBacteriaSwarm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSBacteriaSwarm_FuncPtrs(nullptr); }
#endif
