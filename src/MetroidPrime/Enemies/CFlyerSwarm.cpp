#include "MetroidPrime/Enemies/CFlyerSwarm.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFlyerSwarm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "Collision/CollisionUtil.hpp"

CFlyerSwarm::CFlyerSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CVector3f& boundingBoxExtent, const CTransform4f& xf,
                         const CAnimRes& animRes, CActorParameters actorParameters,
                         const CBasicSwarmData& data, bool active, float surfaceAvoidance,
                         float initialMoveSpeedModifier, float initialMoveSpeedModifierTime,
                         float spawnSpread, float rollUprightSpeed, float rollUprightMinAngle)
: CSwarmBasics(uid, name, info, boundingBoxExtent, xf, animRes, actorParameters, data, active)
, mSurfaceAvoidance(surfaceAvoidance)
, mInitialMoveSpeedModifier(initialMoveSpeedModifier)
, mInitialMoveSpeedModifierTime(initialMoveSpeedModifierTime)
, mSpawnSpread(spawnSpread)
, mRollUprightSpeed(rollUprightSpeed * (M_PIF / 180.f))
, mRollUprightMinAngle(rollUprightMinAngle * (M_PIF / 180.f))
, mPartitionHasCollision(false) {}

CFlyerSwarm::~CFlyerSwarm() {}

void CFlyerSwarm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  if (msg.GetMessage() == kSM_AreaLoaded) {
    const CAABox swarmBounds = GetBoundingBox();
    for (int x = 0; x < 5; ++x) {
      for (int y = 0; y < 5; ++y) {
        for (int z = 0; z < 5; ++z) {
          const CAABox bounds = BoxForPosition(x, y, z, 0.5f + mBoidRadius);
          CAreaCollisionCache cache = GetAreaCollisionCacheForPartition(x, y, z);
          CGameCollision::BuildAreaCollisionCache(mgr, cache);
          for (uint i = 0; i < cache.GetNumCaches(); ++i) {
            const CMetroidAreaCollider::COctreeLeafCache& leafCache = cache.GetOctreeLeafCache(i);
            for (int j = 0; j < leafCache.GetNumLeaves(); ++j) {
              const CAreaOctTree::Node& node = leafCache.GetLeaf(j);
              if (!bounds.DoBoundsOverlap(node.GetBoundingBox())) {
                continue;
              }
              const CAreaOctTree::TriListReference triangles = node.GetTriangleArray();
              for (int k = 0; k < triangles.GetSize(); ++k) {
                const CCollisionSurface surface = node.GetOwner().GetTriangle(triangles.GetAt(k));
                if (CollisionUtil::TriBoxOverlap(
                        bounds.GetCenterPoint(),
                        0.5f * (bounds.GetMaxPoint() - bounds.GetMinPoint()), surface.GetVert(0),
                        surface.GetVert(1), surface.GetVert(2))) {
                  mPartitionHasCollision[x + y * 5 + z * 25] = true;
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  CSwarmBasics::AcceptScriptMsg(mgr, msg);
}

void CFlyerSwarm::CreateBoid(CStateManager& mgr, int index) {
  const CAABox bounds = GetBoundingBox();
  const TUniqueId waypointId = GetWaypointForState(kSS_Patrol, mgr);
  if (const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId))) {
    const TUniqueId nextId = waypoint->NextWaypoint(mgr);
    if (const CScriptWaypoint* next =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(nextId))) {
      const CVector3f pos = waypoint->GetTranslation();
      const CVector3f direction = (next->GetTranslation() - pos).AsNormalized();
      const float pitch = asin(direction.GetZ());
      const float yaw = atan2(-direction.GetX(), direction.GetY());
      CBoid& boid = mBoids[index];
      if (mSpawnSpread > 0.f) {
        const float spread = mSpawnSpread * (M_PIF / 180.f);
        const float yawJitter = spread * (mgr.Random()->Float() - 0.5f);
        const float pitchJitter = pitch + spread * (mgr.Random()->Float() - 0.5f);
        boid.mTransform.SetRotation(CTransform4f::RotateZ(CRelAngle::FromRadians(yaw)) *
                                    CTransform4f::RotateX(CRelAngle::FromRadians(pitchJitter)) *
                                    CTransform4f::RotateZ(CRelAngle::FromRadians(yawJitter)));
      } else {
        boid.mTransform.SetRotation(CTransform4f::RotateZ(CRelAngle::FromRadians(yaw)) *
                                    CTransform4f::RotateX(CRelAngle::FromRadians(pitch)));
      }
      boid.mTransform.SetTranslation(pos);
      boid.mActive = true;
      boid.mVelocity = CVector3f::Zero();
      boid.mTargetWaypoint = nextId;
      boid.mSurfacePlane = CPlane(next->GetTranslation(), CUnitVector3f(direction));
      boid.mFramesNotOnSurface = 0;
      boid.mFreezeTimer = 0.f;
      boid.mExplodeTimerEnabled = false;
      boid.mHealth = mHealthInfo.GetHP();
      boid.mLaunched = true;
      boid.mLifeTime = mLifeTime;
    }
  }
}

void CFlyerSwarm::Think(float dt, CStateManager& mgr) { CSwarmBasics::Think(dt, mgr); }

void CFlyerSwarm::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                             int partitionIndex) {
  boid.mPartitionIndex = partitionIndex;
  if (mLifeTime > 0.f) {
    boid.mLifeTime -= dt;
    if (boid.mLifeTime <= 0.f) {
      KillBoid(boid, mgr, CWeaponMode());
      return;
    }
  }
  UpdateLightComboBeam(boid, mgr);
  if (mInitialMoveSpeedModifierTime > 0.f) {
    const float elapsed = mLifeTime - boid.mLifeTime;
    if (elapsed < mInitialMoveSpeedModifierTime) {
      boid.mSpeedScale =
          mInitialMoveSpeedModifier * (1.f - elapsed / mInitialMoveSpeedModifierTime);
    } else {
      boid.mSpeedScale = 1.f;
    }
  }
  mBoidUpdatedThisFrame = true;
  if (mVulnerableToSafeZone) {
    if (mgr.GetSafeZoneManager()->PointIsInSafeZone(mgr, boid.GetTranslation())) {
      KillBoid(boid, mgr, CWeaponMode());
      return;
    }
  }
  rstl::reserved_vector< CBoid*, 50 > nearList;
  BuildBoidNearList(boid, mSeparationRadius, nearList);
  CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                            CVector3f(0.f, 0.f, 1.f), ~0);
  bool collided = false;
  if (mBoidRadius > 0.f && ShouldBuildAreaCollisionCacheForPartition(partitionIndex)) {
    const float radius = mBoidRadius;
    const float probeRadius = 0.5f * radius;
    const float speed = boid.mVelocity.Magnitude();
    const CVector3f step = radius * ((1.f / speed) * -boid.mVelocity);
    CVector3f pos = boid.GetTranslation();
    for (float distance = speed * dt; distance >= 0.f; distance -= radius) {
      if (FindBestSurface(cache, pos + mSurfaceProbeScale * (dt * boid.mVelocity), probeRadius,
                          surface)) {
        boid.mTransform =
            ShortestRotationArcWrapped(boid.GetTransform().GetUp(), surface.GetNormal(),
                                       CRelAngle::FromRadians(M_PIF))
                .MultiplyIgnoreTranslation(boid.GetTransform());
        collided = true;
        break;
      }
      pos += step;
    }
  }
  CVector3f ahead = 0.4f * boid.GetTransform().GetForward();
  if (collided) {
    ahead += mSurfaceAvoidance * surface.GetPlane().GetNormal();
  }
  ApplySteeringBehaviors(mgr, boid, ahead, nearList);
  const CVector3f forward = boid.GetTransform().GetForward();
  boid.mTransform = ShortestRotationArcWrapped(forward, ahead.AsNormalized(),
                                               CRelAngle::FromDegrees(mTurnRate * dt))
                        .MultiplyIgnoreTranslation(boid.GetTransform());
  if (mRollUprightSpeed > 0.f) {
    float roll = asin(boid.GetTransform().GetRight().GetZ());
    if (boid.GetTransform().GetUp().GetZ() < 0.f) {
      roll = -roll;
    }
    const float magnitude = fabs(roll);
    if (magnitude > mRollUprightMinAngle) {
      float correction = magnitude - mRollUprightMinAngle;
      if (dt * mRollUprightSpeed < correction) {
        correction = dt * mRollUprightSpeed;
      }
      if (roll < 0.f) {
        correction = -correction;
      }
      boid.mTransform.RotateLocalY(CRelAngle::FromRadians(correction));
    }
  }
}

void CFlyerSwarm::ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                         const rstl::reserved_vector< CBoid*, 50 >& nearList) {
  CSwarmBasics::ApplySteeringBehaviors(mgr, boid, ahead, nearList);
}

bool CFlyerSwarm::ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const {
  if (partitionIndex == -1) {
    return true;
  }
  return mPartitionHasCollision[partitionIndex];
}

void CFlyerSwarm::Render(const CStateManager& mgr) const { CSwarmBasics::Render(mgr); }

CEntity* REL_LoadFlyerSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFlyerSwarm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFlyerSwarm.inc"

  const CAnimRes animRes(sldrThis.animationInformation.ancs,
                         sldrThis.animationInformation.character_index, CVector3f::One(),
                         sldrThis.animationInformation.initial_anim, true);
  return rs_new CFlyerSwarm(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                            LdrToEntityInfo(info, sldrThis.editorProperties),
                            sldrThis.editorProperties.transform.scale,
                            LdrToTransform4f(sldrThis.editorProperties), animRes,
                            LdrToActorParameters(sldrThis.actorInformation),
                            LdrToBasicSwarmData(sldrThis.basicSwarmProperties), sldrThis.active,
                            sldrThis.unknown_0x4a85a2da, sldrThis.initialMoveSpeedModifier,
                            sldrThis.initialMoveSpeedModifierTime, sldrThis.unknown_0x262e586d,
                            sldrThis.rollUprightSpeed, sldrThis.rollUprightMinAngle);
}

static void SetFuncPtrs() {
  static SFlyerSwarm_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadFlyerSwarm;
  SetSFlyerSwarm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSFlyerSwarm_FuncPtrs(nullptr); }
