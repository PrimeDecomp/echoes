#include "MetroidPrime/Enemies/CSwarmBasics.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/SwarmRenderHelpers.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Weapons/CIceImpact.hpp"
#include "MetroidPrime/Weapons/CLightComboProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "rstl/algorithm.hpp"

#include <float.h>
#include <stdlib.h>

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CAdvancementDeltas.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "WorldFormat/CAreaOctTree.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "rstl/math.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"

#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "REL/REL_Setup.h"

// The native record holds a single callback that always returns null; its signature is unknown.
struct SSwarmBasics_FuncPtrs {
  void* (*mFactory)();
};

CTransform4f CSwarmBasics::ShortestRotationArcWrapped(const CVector3f& a, const CVector3f& b,
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

// Guessed names: qsort comparators over the listener distance cached in each boid.
static int CompareBoidsByListenerDistance(const void* a, const void* b) {
  const CSwarmBasics::CBoid* boidA = *static_cast< CSwarmBasics::CBoid* const* >(a);
  const CSwarmBasics::CBoid* boidB = *static_cast< CSwarmBasics::CBoid* const* >(b);
  if (boidA->GetDistanceSquaredToSoundListener() < boidB->GetDistanceSquaredToSoundListener()) {
    return 1;
  }
  if (boidA->GetDistanceSquaredToSoundListener() > boidB->GetDistanceSquaredToSoundListener()) {
    return -1;
  }
  return 0;
}

static int CompareBoidsByListenerDistanceReverse(const void* a, const void* b) {
  const CSwarmBasics::CBoid* boidA = *static_cast< CSwarmBasics::CBoid* const* >(a);
  const CSwarmBasics::CBoid* boidB = *static_cast< CSwarmBasics::CBoid* const* >(b);
  if (boidA->GetDistanceSquaredToSoundListener() > boidB->GetDistanceSquaredToSoundListener()) {
    return 1;
  }
  if (boidA->GetDistanceSquaredToSoundListener() < boidB->GetDistanceSquaredToSoundListener()) {
    return -1;
  }
  return 0;
}

static int CompareBoidRefsByListenerDistance(const void* a, const void* b) {
  const CSwarmBasics::CBoid* boidA = static_cast< const CSwarmBasics::CBoid* >(a);
  const CSwarmBasics::CBoid* boidB = static_cast< const CSwarmBasics::CBoid* >(b);
  if (boidA->GetDistanceSquaredToSoundListener() > boidB->GetDistanceSquaredToSoundListener()) {
    return 1;
  }
  if (boidA->GetDistanceSquaredToSoundListener() < boidB->GetDistanceSquaredToSoundListener()) {
    return -1;
  }
  return 0;
}

CSwarmBasics::CBoid::CBoid(const CTransform4f& xf, uint index)
: mTransform(xf)
, mVelocity(0.f, 0.f, 0.f)
, mTargetWaypoint(kInvalidUniqueId)
, mSurfacePlane(xf.GetTranslation(), CUnitVector3f(xf.GetForward()))
, mAmbientLighting(0.3f, 0.3f, 0.3f, 1.f)
, mNext(nullptr)
, mFreezeTimer(0.f)
, mTimeToExplode(0.f)
, mSurface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f), CVector3f(0.f, 0.f, 1.f), ~0)
, mAttackSlot(-1)
, mDistanceSquaredToSoundListener(0.f)
, mSpeedScale(1.f)
, xa8_(kInvalidUniqueId)
, xaa_(kInvalidUniqueId)
, mFramesNotOnSurface(0)
, mIndex(index)
, mPartitionIndex(-1)
, xb1_(0)
, mActive(false)
, mInFrustum(false)
, mLaunched(false)
, mExplodeTimerEnabled(false)
, mAttacking(false)
, mHasLoopedSound(false) {}

static CModelData GetModelDataForAnimRes(const CAnimRes& animRes) {
  return animRes.GetId() != kInvalidAssetId ? CModelData(animRes) : CModelData::CModelDataNull();
}

CSwarmBasics::CSwarmBasics(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& boundingBoxExtent, const CTransform4f& xf,
                           const CAnimRes& animRes, CActorParameters actorParameters,
                           const CBasicSwarmData& data, bool animated)
: CActor(uid, name, info, 0, xf, GetModelDataForAnimRes(animRes),
         CMaterialList(kMT_Scannable, kMT_Trigger, kMT_NonSolidDamageable, kMT_RadarObject),
         actorParameters, kInvalidUniqueId)
, mAabox(CVector3f(0.f, 0.f, 0.f), CVector3f(0.f, 0.f, 0.f))
, mOccludedTimer(5.f)
, mBoundingBoxExtent(boundingBoxExtent)
, mLastOrbitPosition(0.f, 0.f, 0.f)
, mLastKilledOffset(CVector3f::Zero())
, mSeparationRadius(data.mInfluenceRadius)
, mCohesionMagnitude(data.mCohesionPriority)
, mAlignmentWeight(data.mAlignmentPriority)
, mSeparationMagnitude(data.mSeparationPriority)
, mMoveToWaypointWeight(data.mPathFollowingPriority)
, mAttractionMagnitude(data.mPlayerAttractPriority)
, mAttractionRadius(data.mPlayerAttractDistance)
, mTimeSinceLastAttack(0.f)
, mAnimPlaybackSpeed(data.mSpeed)
, mWaypointGoalRadius(3.f)
, mPartitionedBoidLists(125, nullptr)
, mOutlierBoidList(nullptr)
, mBoidGenRate(data.mSpawnSpeed)
, mBoidGenCooldownTimer(0.f)
, mDamageCooldownTimer(0.f)
, mDamageCooldown(data.mDamageWaitTime)
, mBoidRadius(data.mCollisionRadius)
, mTouchRadius(data.mTouchRadius)
, mTurnRate(data.mTurnRate)
, mPlayerTouchRadius(data.mDamageRadius)
, mDamage(data.mContactDamage)
, mRadiusDamage(data.mContactDamage)
, mHealthInfo(data.mHealth)
, mDamageVulnerability(data.mDamageVulnerability)
, mLockOnIndex(-1)
, mWhichModel(CModelData::kWM_Normal)
, mNumDeathParticles(data.mNumDeathParticles)
, mNumBoids(data.mCount)
, mMaxCreatedBoids(data.mMaxCount)
, mCreatedBoids(0)
, mEnableLighting(true)
, mUseSoftwareLight(true)
, x4f0_26_(true)
, mAnimated(animated)
, mVulnerableToSafeZone(data.mIsVulnerableToSafeZone)
, x4f0_29_(data.xdc_1)
, x4f0_30_(true)
, mBoidUpdatedThisFrame(false)
, x4f1_24_(false)
, x4f1_25_(data.mIsOrbitable)
, mSurfaceProbeScale(1.5f)
, mLocomotionLoopedSound(data.mLocomotionLoopedSound)
, mAttackLoopedSound(data.mAttackLoopedSound)
, mSoundFallOff(data.mSoundFallOff)
, mMaxAudibleDistance(data.mMaxAudibleDistance)
, mMinVolume(data.mMinVolume)
, mMaxVolume(data.mMaxVolume)
, mMaxLocomotionEmitters(4)
, mMaxAttackEmitters(4)
, x52c_(0)
, x530_(0)
, x534_(0)
, mFreezeDuration(data.mFreezeDuration)
, x544_(0)
, mLifeTime(data.mLifeTime)
, x54c_24_(data.mIndividuallyTargetable)
, x54c_25_(false)
, x550_(CVector3f::Zero())
, x55c_(0.f)
, x560_(15)
, mSeekerBoidIndices(5, -1) {
  if (mAnimated) {
    mModelDatas.reserve(4);
    mAdvancementDeltas.reserve(4);
    if (animRes.GetId() != kInvalidAssetId) {
      for (uint i = 0; i < 4; ++i) {
        mModelDatas.push_back_unsafe(CModelData(animRes));
        mAdvancementDeltas.push_back_unsafe(
            CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
      }
      mModelData = rs_new CModelData(animRes);
      mDisplayList = rs_new SwarmRenderHelpers::CSwarmDisplayList(
          **mModelData->GetAnimationData()->GetModelData());
    }
  }
  if (data.mDeathParticleEffect != kInvalidAssetId) {
    mParticleDescription =
        rstl::optional_object< TLockedToken< CGenDescription > >(TLockedToken< CGenDescription >(
            gpSimplePool->GetObj(SObjectTag('PART', data.mDeathParticleEffect))));
    mParticleGenerator = rs_new CElementGen(*mParticleDescription);
    mParticleGenerator->SetParticleEmission(false);
  }
  FinishConstruction();
}

CSwarmBasics::~CSwarmBasics() {}

CAABox CSwarmBasics::GetBoundingBox() const {
  const CVector3f extent(0.5f * mBoundingBoxExtent.GetX(), 0.5f * mBoundingBoxExtent.GetY(),
                         0.5f * mBoundingBoxExtent.GetZ());
  const CAABox bounds(-extent, extent);
  return bounds.GetTransformedAABox(GetTransform());
}

rstl::optional_object< CAABox > CSwarmBasics::GetTouchBounds() const { return mAabox; }

void CSwarmBasics::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Activate:
    break;
  case kSM_AreaLoaded:
    AddDoorRepulsors(mgr);
    break;
  case kSM_Deactivate: {
    StopLocomotionSounds();
    uint count = mSeekerTargets.size();
    for (uint i = 0; i < count; ++i) {
      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mSeekerTargets[i]))) {
        act->SetActive(false);
      }
    }
    break;
  }
  case kSM_Create: {
    mBoids.reserve(mNumBoids);
    mActiveBoidIndices.reserve(mNumBoids);
    for (int i = 0; i < mBoids.capacity(); ++i) {
      mBoids.push_back_unsafe(CBoid(CTransform4f::Identity(), i));
    }
    AllocateSkinnedModels(mgr, CModelData::kWM_Normal);
    SetDrawShadow(false);
    if (mLocomotionLoopedSound != CSfxManager::kInternalInvalidSfxId) {
      mLocomotionSounds.reserve(mMaxLocomotionEmitters);
      for (uint i = 0; i < mMaxLocomotionEmitters; ++i) {
        mLocomotionSounds.push_back_unsafe(TLoopedSound(CSfxHandle(), 0));
      }
    }
    if (mAttackLoopedSound != CSfxManager::kInternalInvalidSfxId) {
      mAttackSounds.reserve(mMaxAttackEmitters);
      for (uint i = 0; i < mMaxAttackEmitters; ++i) {
        mAttackSounds.push_back_unsafe(TLoopedSound(CSfxHandle(), 0));
      }
    }
    AddMaterial(kMT_Character, mgr);
    mSeekerTargets.reserve(5);
    mSeekerBoidIndices.reserve(5);
    for (uint i = 0; i < 5; ++i) {
      TAreaId area = GetCurrentAreaId();
      TUniqueId uid = mgr.AllocateUniqueId();
      CPhysicsActor* act = rs_new CPhysicsActor(
          uid, rstl::string_l(""),
          CEntityInfo(area, rstl::vector< SConnection >(), true, kInvalidEditorId), 0,
          CTransform4f::Identity(), CModelData::CModelDataNull(), CMaterialList(kMT_SeekerTarget),
          CAABox(-mBoidRadius, -mBoidRadius, -mBoidRadius, mBoidRadius, mBoidRadius, mBoidRadius),
          SMoverData(1.f), CActorParameters::None(), CPhysicsActor::skDefaultStepData);
      act->AddMaterial(kMT_SeekerTarget, mgr);
      act->RemoveMaterial(kMT_Solid, mgr);
      if (act) {
        mgr.AddObject(*act);
        mSeekerTargets.push_back_unsafe(uid);
      }
    }
    break;
  }
  case kSM_Delete: {
    StopLocomotionSounds();
    uint count = mSeekerTargets.size();
    for (uint i = 0; i < count; ++i) {
      mgr.DeleteObjectRequest(mSeekerTargets[i]);
    }
    break;
  }
  case kSM_Decrement:
    x4f0_30_ = false;
    break;
  case kSM_Increment:
    x4f0_30_ = true;
    break;
  case kSM_InternalMessage0:
    ++x52c_;
    break;
  }
}

void CSwarmBasics::StopLocomotionSounds() {
  uint count = mLocomotionSounds.size();
  if (count != 0) {
    for (uint i = 0; i < count; ++i) {
      if (mLocomotionSounds[i].first) {
        CSfxManager::SfxStop(mLocomotionSounds[i].first);
        mLocomotionSounds[i].first = CSfxHandle();
      }
    }
  }
}

void CSwarmBasics::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  mSkinnedModelStates.clear();
  if (mAnimated) {
    uint count = mModelDatas.size();
    mSkinnedModelStates.reserve(count);
    for (uint i = 0; i < count; ++i) {
      const CSkinnedModel& skinnedModel = mModelDatas[i].PickAnimatedModel(which);
      mSkinnedModelStates.push_back_unsafe(
          SwarmRenderHelpers::CSwarmSkinnedModelState(skinnedModel));
      mModelDatas[i].EnableLooping(true);
      mModelDatas[i].AdvanceAnimation(
          mModelDatas[i].GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")) *
              (0.75f * (float(i) / float(count))),
          mgr, GetCurrentAreaId(), true);
    }
    const CSkinnedModel& model = mModelData->PickAnimatedModel(which);
    mSkinnedModelState = rs_new SwarmRenderHelpers::CSwarmSkinnedModelState(model);
    const CAnimData* animData = mModelData->GetAnimationData();
    animData->BuildPose();
    SwarmRenderHelpers::CSwarmSkinnedModelState& state = *mSkinnedModelState;
    model.StoreCalculation(state.State(), &animData->Pose());
    state.StateToArrays();
  }
  mWhichModel = which;
}

void CSwarmBasics::CreateBoid(CStateManager& mgr, int index) {
  const CAABox bounds = GetBoundingBox();
  const TUniqueId waypointId = GetWaypointForState(kSS_Patrol, mgr);
  if (const CScriptWaypoint* waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypointId))) {
    const TUniqueId nextId = waypoint->NextWaypoint(mgr);
    if (const CScriptWaypoint* next =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(nextId))) {
      const CVector3f pos = waypoint->GetTranslation();
      const CCollisionSurface surface(FindBestCollisionInBox(mgr, pos));
      const CVector3f projected = ProjectPointToPlane(pos, surface.GetVert(0), surface.GetNormal());
      const CVector3f translation = projected + surface.GetNormal() * mBoidRadius;
      mBoids[index].mTransform = CTransform4f::Translate(translation);
      if (close_enough(CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), surface.GetNormal()), -1.f)) {
        mBoids[index].mTransform.SetRotation(
            CTransform4f::FromColumns(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, -1.f, 0.f),
                                      CVector3f(0.f, 0.f, -1.f), CVector3f::Zero()));
      } else {
        mBoids[index].mTransform.SetRotation(ShortestRotationArcWrapped(
            CVector3f(0.f, 0.f, 1.f), surface.GetNormal(), CRelAngle::FromRadians(M_PIF)));
      }
      mBoids[index].mActive = true;
      mBoids[index].mVelocity = CVector3f::Zero();
      mBoids[index].mTargetWaypoint = nextId;
      mBoids[index].mSurfacePlane = CPlane(
          next->GetTranslation(),
          CUnitVector3f((next->GetTranslation() - waypoint->GetTranslation()).AsNormalized()));
      mBoids[index].mFramesNotOnSurface = 0;
      mBoids[index].mFreezeTimer = 0.f;
      mBoids[index].mExplodeTimerEnabled = false;
      mBoids[index].mHealth = mHealthInfo.GetHP();
      mBoids[index].mIndex = index;
      mBoids[index].mHasLoopedSound = false;
      mBoids[index].mLifeTime = mLifeTime;
      mBoids[index].xa8_ = kInvalidUniqueId;
      mBoids[index].xaa_ = kInvalidUniqueId;
      mBoids[index].mPartitionIndex = -1;
      mBoids[index].xb1_ = 0;
    }
  }
}

void CSwarmBasics::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox bounds = GetBoundingBox();
  SetOtherBounds(bounds);
  SetRenderBounds(bounds);
  UpdatePortalSystemState(mgr);
}

CVector3f CSwarmBasics::ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                            const CVector3f& normal) {
  return point - CVector3f::Dot(point - planePoint, normal) * normal;
}

CVector3f CSwarmBasics::ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal) {
  return point - CVector3f::Dot(point, normal) * normal;
}

bool CSwarmBasics::PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos,
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

CCollisionSurface CSwarmBasics::FindBestCollisionInBox(CStateManager& mgr, const CVector3f& pos) {
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

bool CSwarmBasics::FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos, float radius,
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

void CSwarmBasics::UpdateLightComboBeam(CBoid& boid, CStateManager& mgr) {
  if (boid.xaa_ != kInvalidUniqueId) {
    CLightComboProjectile* proj = TCastToPtr< CLightComboProjectile >(mgr.ObjectById(boid.xa8_));
    const CPlasmaProjectile* plasma =
        TCastToConstPtr< CPlasmaProjectile >(mgr.GetObjectById(boid.xaa_));
    if (plasma) {
      if (proj) {
        proj->UpdateRayTarget(mgr, boid.xaa_, boid.GetTranslation());
        boid.mHealth -= plasma->GetCurrentDamageInfo().GetDamage(*GetDamageVulnerability());
        if (boid.mHealth <= 0.f) {
          KillBoid(boid, mgr, CWeaponMode(kWT_Light));
          proj->RequestRayReset(mgr, boid.xaa_, false);
        }
      } else {
        boid.xaa_ = kInvalidUniqueId;
        boid.xa8_ = kInvalidUniqueId;
      }
    } else {
      boid.xaa_ = kInvalidUniqueId;
      boid.xa8_ = kInvalidUniqueId;
    }
  }
}

void CSwarmBasics::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                              int partitionIndex) {
  boid.mPartitionIndex = partitionIndex;
  if (mLifeTime > 0.f) {
    boid.mLifeTime -= dt;
    if (boid.mLifeTime <= 0.f) {
      KillBoid(boid, mgr, CWeaponMode());
      return;
    }
  }
  mBoidUpdatedThisFrame = true;
  if (mVulnerableToSafeZone) {
    if (mgr.GetSafeZoneManager()->PointIsInSafeZone(mgr, boid.GetTranslation())) {
      KillBoid(boid, mgr, CWeaponMode());
      return;
    }
  }
  UpdateLightComboBeam(boid, mgr);
  if (boid.mLaunched) {
    const float radius = 2.f * mBoidRadius;
    const float boidRadius = mBoidRadius;
    const float speed = boid.mVelocity.Magnitude();
    float distance = speed * dt;
    CVector3f pos = boid.GetTranslation();
    const CVector3f step = boidRadius * ((1.f / speed) * -boid.mVelocity);
    bool found = false;
    while (distance >= 0.f && !found) {
      CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                                CVector3f(0.f, 0.f, 1.f), ~0);
      const CVector3f predicted = pos + mSurfaceProbeScale * (dt * boid.mVelocity);
      if (FindBestSurface(cache, predicted, radius, surface) &&
          boid.mRemainingLaunchNotOnSurfaceFrames == 0) {
        boid.mTransform =
            ShortestRotationArcWrapped(boid.GetTransform().GetUp(), surface.GetNormal(),
                                       CRelAngle::FromRadians(M_PIF))
                .MultiplyIgnoreTranslation(boid.GetTransform());
        const CPlane plane = surface.GetPlane();
        found = true;
        boid.mTransform.AddTranslation(
            -(plane.GetHeight(boid.GetTranslation()) - boidRadius - 0.01f) * plane.GetNormal());
        boid.mFramesNotOnSurface = 0;
        boid.mLaunched = false;
        BoidCollidedCallback(mgr, boid);
      }
      distance -= boidRadius;
      pos += step;
    }
    if (!found) {
      boid.mVelocity += dt * CVector3f(0.f, 0.f, -CPhysicsActor::GravityConstant());
      if (boid.mRemainingLaunchNotOnSurfaceFrames != 0) {
        boid.mRemainingLaunchNotOnSurfaceFrames--;
      }
    }
  } else if (boid.mFramesNotOnSurface >= 30) {
    boid.mActive = false;
    if (boid.mHasLoopedSound) {
      StopLoopedSound(boid, mLocomotionSounds);
    }
  } else {
    const float radius = 2.f * mBoidRadius;
    const float boidRadius = mBoidRadius;
    const CVector3f pos = boid.GetTranslation();
    bool found = false;
    CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                              CVector3f(0.f, 0.f, 1.f), ~0);
    const CVector3f predicted = pos + mSurfaceProbeScale * (dt * boid.mVelocity);
    if (FindBestSurface(cache, predicted, radius, surface)) {
      boid.mSurface = surface;
      const CPlane plane = surface.GetPlane();
      const float distance = plane.GetHeight(boid.GetTranslation());
      if (distance <= mBoidRadius * mSurfaceProbeScale) {
        boid.mTransform =
            ShortestRotationArcWrapped(boid.GetTransform().GetUp(), surface.GetNormal(),
                                       CRelAngle::FromDegrees(180.f * dt))
                .MultiplyIgnoreTranslation(boid.GetTransform());
        found = true;
        boid.mTransform.AddTranslation(-(distance - boidRadius - 0.01f) * plane.GetNormal());
        boid.mFramesNotOnSurface = 0;
      }
    }
    if (!found) {
      const float angularSpeed = boid.mVelocity.Magnitude() / boidRadius;
      boid.mTransform =
          ShortestRotationArcWrapped(boid.GetTransform().GetUp(), boid.GetTransform().GetForward(),
                                     CRelAngle::FromRadians(angularSpeed * dt))
              .MultiplyIgnoreTranslation(boid.GetTransform());
      ++boid.mFramesNotOnSurface;
    }
    rstl::reserved_vector< CBoid*, 50 > nearList;
    BuildBoidNearList(boid, mSeparationRadius, nearList);
    CVector3f ahead = 0.3f * boid.GetTransform().GetForward();
    ApplySteeringBehaviors(mgr, boid, ahead, nearList);
    const CVector3f projected = ProjectVectorToPlane(ahead, boid.GetTransform().GetUp());
    const CVector3f forward = boid.GetTransform().GetForward();
    const CVector3f direction = projected.AsNormalized();
    boid.mTransform =
        ShortestRotationArcWrapped(forward, direction, CRelAngle::FromDegrees(mTurnRate * dt))
            .MultiplyIgnoreTranslation(boid.GetTransform());
  }
}

void CSwarmBasics::ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                          const rstl::reserved_vector< CBoid*, 50 >& nearList) {
  if (boid.mFreezeTimer <= 0.f) {
    for (int i = 0; i < 8; ++i) {
      switch (i) {
      case 0:
        for (rstl::vector< CRepulsor >::iterator it = mDoorRepulsors.begin();
             it != mDoorRepulsors.end(); ++it) {
          if ((it->mCenter - boid.GetTranslation()).MagSquared() <
              it->mMagnitude * it->mMagnitude) {
            ApplySeparation(boid, it->mCenter, it->mMagnitude, 4.5f, ahead);
          }
        }
        break;
      case 4:
        ApplySeparation(boid, nearList, ahead);
        break;
      case 5:
        MoveToWayPoint(boid, mgr, ahead);
        break;
      case 6:
        ApplyCohesion(boid, nearList, ahead);
        break;
      case 7:
        ApplyAlignment(boid, nearList, ahead);
        break;
      case 3:
        ApplyAttraction(boid, mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f), mAttractionRadius,
                        mAttractionMagnitude, ahead);
        break;
      default:
        break;
      }
      if (ahead.MagSquared() >= 9.f) {
        return;
      }
    }
  }
}

void CSwarmBasics::AddDoorRepulsors(CStateManager& mgr) {
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

void CSwarmBasics::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mBoidUpdatedThisFrame = false;
  if (!GetActive()) {
    return;
  }
  SetTransformDirty();
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
  for (uint i = 0; i < mgr.GetNumPlayers(); ++i) {
    SetValidTarget(i, mLockOnIndex != -1);
  }
  if (mLockOnIndex == -1 || !x4f1_25_) {
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  } else {
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
    mLastOrbitPosition = GetLockOnLocation(mLockOnIndex);
  }
  if (x4f0_30_) {
    mBoidGenCooldownTimer -= dt;
    while ((mMaxCreatedBoids == 0 || mCreatedBoids < mMaxCreatedBoids) &&
           mBoidGenCooldownTimer <= 0.f) {
      bool created = false;
      for (int i = 0; i < mBoids.size(); ++i) {
        if (!mBoids[i].mActive) {
          CreateBoid(mgr, i);
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
  }
  if (x52c_ != 0) {
    for (int i = 0; i < mBoids.size(); ++i) {
      if (!mBoids[i].mActive) {
        CreateBoid(mgr, i);
        ++mCreatedBoids;
        if (--x52c_ <= 0) {
          break;
        }
      }
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
            if (((mThinkCounter & 1) == (count & 1) && boid->mActive &&
                 boid->mFreezeTimer < 0.1f) ||
                boid->mLaunched) {
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
    if (((mThinkCounter & 1) == (count & 1) && boid->mActive && boid->mFreezeTimer < 0.1f) ||
        boid->mLaunched) {
      const float margin = 1.5f + (0.5f + mBoidRadius);
      const CVector3f extent(margin, margin, margin);
      const CAABox boidBounds(boid->GetTranslation() - extent, boid->GetTranslation() + extent);
      CAreaCollisionCache cache(boidBounds);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      UpdateBoid(cache, mgr, dt, *boid, -1);
    }
  }
  UpdateSwarmAnimations(mgr, dt);
  UpdateAllBoidMovement(mgr, dt);
  UpdateClosestPartitionLoopedSounds(mgr.GetPlayer(0)->GetTranslation(), mLocomotionSounds,
                                     mMaxLocomotionEmitters, mLocomotionLoopedSound,
                                     kLST_Locomotion);
  FlushDeathMessages(mgr);
  if (x4f1_24_) {
    SendScriptMsgs(kSS_InternalState0, mgr);
    x4f1_24_ = false;
  }
  UpdateSeekerTargets(mgr);
}

void CSwarmBasics::UpdateAllBoidMovement(CStateManager& mgr, float dt) {
  uint count = mBoids.size();
  if (mAnimated) {
    uint mask = mModelDatas.size() - 1;
    for (uint i = 0; i < count; ++i) {
      MoveBoid(mgr, mBoids[i], mAdvancementDeltas[i & mask].GetOffsetDelta(), dt);
    }
  }
}

void CSwarmBasics::UpdateSwarmAnimations(CStateManager& mgr, float dt) {
  if (mAnimated && mBoidUpdatedThisFrame) {
    uint count = mModelDatas.size();
    for (uint i = 0; i < count; ++i) {
      mModelDatas[i].AnimationData()->SetPlaybackRate(mAnimPlaybackSpeed);
      mAdvancementDeltas[i] = mModelDatas[i].AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
      UpdateEffects(mgr, *mModelDatas[i].AnimationData(), mMaxVolume);
    }
  }
}

void CSwarmBasics::MoveBoid(CStateManager& mgr, CBoid& boid, const CVector3f& offsetDelta,
                            float dt) {
  if (boid.mActive) {
    if (boid.mFreezeTimer > 0.f) {
      boid.mFreezeTimer -= dt;
      if (boid.mFreezeTimer < 0.7f * mgr.Random()->Float()) {
        KillBoid(boid, mgr, CWeaponMode(kWT_Dark));
      }
    } else {
      float speed = boid.mSpeedScale / dt;
      boid.mVelocity = speed * boid.GetTransform().Rotate(offsetDelta);
      boid.mTransform.AddTranslation(dt * boid.mVelocity);
    }
  }
}

void CSwarmBasics::UpdatePartition() {
  mActiveBoidIndices.clear();
  mPartitionedBoidLists.clear();
  for (int i = 0; i < 125; ++i) {
    mPartitionedBoidLists.push_back(nullptr);
  }
  mOutlierBoidList = nullptr;
  const CAABox bounds = GetBoundingBox();
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f size = extent / 5.f;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (!it->mActive) {
      if (it->mHasLoopedSound) {
        StopLoopedSound(*it, mLocomotionSounds);
      }
    } else {
      mActiveBoidIndices.push_back_unsafe(uint(it->mIndex));
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

CSwarmBasics::CBoid* CSwarmBasics::GetListAt(const CVector3f& pos) {
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

CAABox CSwarmBasics::BoxForPosition(int x, int y, int z, float margin) const {
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

void CSwarmBasics::HardwareLight(const CStateManager& mgr, const CAABox& bounds) const {
  CActorLights lights(8, CVector3f::Zero(), 4, 4);
  lights.SetNeedsRelight(true);
  lights.SetCastShadows(false);
  lights.BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()), bounds);
  lights.BuildDynamicLightList(mgr, bounds);
  lights.ActivateLights();
}

CColor CSwarmBasics::SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const {
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
    result = CColor::Add(result,
                         CColor::Lerp(CColor::Black(), light.GetColor(),
                                      rstl::max_val(0.f, rstl::min_val(0.8f * attenuation, 1.f))));
  }
  return result;
}

void CSwarmBasics::PreRender(CStateManager& mgr) {
  bool active = false;
  if (mAnimated) {
    uint count = mModelDatas.size();
    for (uint i = 0; i < count; ++i) {
      mModelDatas[i].AnimationData()->PreRender();
    }
  }
  uint drawMask = -1;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      it->mInFrustum = mgr.GetFrustumPlanes().SphereInFrustumPlanes(
          CSphere(it->GetTranslation(), 2.f * mBoidRadius));
      PreRenderBoid(it.get_pointer(), &drawMask);
      active = true;
    } else {
      it->mInFrustum = false;
    }
  }
  SetPreRenderClipped(!active);
}

bool CSwarmBasics::CanRenderUnsorted(const CStateManager& mgr) const { return true; }

void CSwarmBasics::CachePose(CModelData& modelData,
                             SwarmRenderHelpers::CSwarmSkinnedModelState& state) const {
  const CSkinnedModel& model = **mModelData->GetAnimationData()->GetModelData();
  const CAnimData* animData = modelData.GetAnimationData();
  animData->BuildPose();
  model.StoreCalculation(state.State(), &animData->Pose());
  state.StateToArrays();
}

void CSwarmBasics::PreRenderBoid(CBoid* boid, uint* drawMask) {
  if (!(boid->mFreezeTimer > 0.f)) {
    int idx = boid->mIndex & (mModelDatas.size() - 1);
    uint bit = 1 << idx;
    if (*drawMask & bit) {
      *drawMask &= ~bit;
      CachePose(mModelDatas[idx], mSkinnedModelStates[idx]);
    }
  }
}

void CSwarmBasics::RenderBoid(CBoid* boid) const {
  if (mAnimated) {
    if (boid->mFreezeTimer > 0.f) {
      DrawBoidSkinnedModel(boid, *mSkinnedModelState);
    } else {
      DrawBoidSkinnedModel(boid, mSkinnedModelStates[boid->mIndex & (mModelDatas.size() - 1)]);
    }
  }
}

void CSwarmBasics::DrawBoidSkinnedModel(
    const CBoid* boid, const SwarmRenderHelpers::CSwarmSkinnedModelState& state) const {
  CColor color = boid->mAmbientLighting;
  if (boid->mFreezeTimer > 0.f) {
    color = CColor::Lerp(color, CPatterned::skFrozenColor,
                         rstl::min_val(1.f, rstl::max_val(boid->mFreezeTimer, 0.f)));
  }
  if (mEnableLighting) {
    CGX::SetChanMatColor(CGX::Channel0, color.GetGXColor());
  }
  gpRender->SetModelMatrix(boid->GetTransform());
  mDisplayList->DrawFromState(state);
}

void CSwarmBasics::AddToRenderer(const CStateManager& mgr) const {
  if (GetActive()) {
    RenderParticles();
    if (!GetPreRenderClipped()) {
      if (CanRenderUnsorted(mgr)) {
        Render(mgr);
      } else {
        EnsureRendered(mgr);
      }
    }
  }
}

void CSwarmBasics::Render(const CStateManager& mgr) const {
  const int alpha = GetRenderAlphaBufferAlpha(mgr);
  if (alpha != -1) {
    gpRender->SetDestinationAlpha(alpha);
  }
  const bool enableLighting = mEnableLighting;
  const bool useSoftwareLight = mUseSoftwareLight;
  CGraphics::DisableAllLights();
  if (!enableLighting) {
    gpRender->SetAmbientColor(CColor(0.5f, 0.5f, 0.5f, 1.f));
  }
  if (mDisplayList.get()) {
    mDisplayList->SetMaterialCurrent(CModelFlags::Normal());
  }
  const uint lights = CGraphics::GetLightMask();
  CGX::SetChanCtrl(CGX::Channel0,
                   (lights && enableLighting && !useSoftwareLight) ? GX_TRUE : GX_FALSE, GX_SRC_REG,
                   GX_SRC_REG, static_cast< GXLightID >(lights), lights ? GX_DF_CLAMP : GX_DF_NONE,
                   lights ? GX_AF_SPOT : GX_AF_NONE);
  CGX::SetChanAmbColor(CGX::Channel0, CColor::White().GetGXColor());
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        const int index = rowIndex + z * 25;
        CBoid* boid = mPartitionedBoidLists[index];
        if (boid != nullptr) {
          if (enableLighting) {
            const CAABox bounds = BoxForPosition(x, y, z, 0.f);
            if (useSoftwareLight) {
              if ((index & 3) == (mThinkCounter & 3)) {
                const CColor color = SoftwareLight(mgr, bounds);
                for (CBoid* it = boid; it != nullptr; it = it->mNext) {
                  if (it->mActive) {
                    it->mAmbientLighting = CColor::Lerp(it->mAmbientLighting, color, 0.3f);
                  }
                }
              }
            } else {
              HardwareLight(mgr, bounds);
            }
          }
          for (; boid != nullptr; boid = boid->mNext) {
            if (boid->mInFrustum && boid->mActive) {
              RenderBoid(boid);
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
      if (enableLighting) {
        const CVector3f pos = boid->GetTranslation();
        const CVector3f extent(mBoidRadius, mBoidRadius, mBoidRadius);
        const CAABox bounds = CAABox(pos - extent, pos + extent);
        if (useSoftwareLight) {
          if ((index & 3) == (mThinkCounter & 3)) {
            const CColor color = SoftwareLight(mgr, bounds);
            if (boid->mActive) {
              boid->mAmbientLighting = CColor::Lerp(boid->mAmbientLighting, color, 0.3f);
            }
          }
        } else {
          HardwareLight(mgr, bounds);
        }
      }
      RenderBoid(boid);
    }
  }
  CGraphics::DisableAllLights();
  if (alpha != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

void CSwarmBasics::BuildBoidNearList(const CBoid& boid, float radius,
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

void CSwarmBasics::ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
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

void CSwarmBasics::ApplySeparation(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                                   CVector3f& ahead) {
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

void CSwarmBasics::ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
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

void CSwarmBasics::ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                                 CVector3f& ahead) {
  const CVector3f delta = pos - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distance = delta.MagSquared();
    const float radiusSquared = radius * radius;
    const float factor = distance > radiusSquared ? 1.f : distance / radiusSquared;
    ahead += factor * delta.AsNormalized() * magnitude;
  }
}

void CSwarmBasics::ApplyAttraction(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                                   CVector3f& ahead) {
  const float radiusSquared = radius * radius;
  const CVector3f delta = pos - boid.GetTranslation();
  const float distance = delta.MagSquared();
  if (distance < radiusSquared && delta.CanBeNormalized()) {
    const float factor = 1.f - distance / radiusSquared;
    ahead += factor * delta.AsNormalized() * magnitude;
  }
}

void CSwarmBasics::ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                  CVector3f& ahead) {
  if (nearList.size() > 0) {
    CVector3f direction(0.f, 0.f, 0.f);
    for (rstl::reserved_vector< CBoid*, 50 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      direction += (*it)->GetTransform().GetForward();
    }
    direction = (1.f / nearList.size()) * direction;
    const float angle =
        CVector3f::GetAngleDiff(boid.GetTransform().GetForward(), direction) / M_PIF;
    ahead += angle * (mAlignmentWeight * direction);
  }
}

static CPlane GetClosestBoxFacePlane(const CAABox& box, const CVector3f& point) {
  float minDistance = FLT_MAX;
  int bestFace = 0;
  for (int i = 0; i < 6; ++i) {
    const CTri tri = box.GetTri(CAABox::EBoxFaceId(i), 0);
    const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
    const float distance = plane.GetHeight(point);
    if (distance >= 0.f && distance < minDistance) {
      bestFace = i;
      minDistance = distance;
    }
  }
  const CTri tri = box.GetTri(CAABox::EBoxFaceId(bestFace), 0);
  return CPlane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
}

void CSwarmBasics::ApplyBoundsAvoidance(CBoid& boid,
                                        const rstl::reserved_vector< CBoid*, 50 >& nearList,
                                        CVector3f& ahead) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f future = boid.GetTranslation() + 1.5f * boid.mVelocity;
  if (!bounds.PointInside(future)) {
    const CPlane plane = GetClosestBoxFacePlane(bounds, future);
    const float distance = plane.GetHeight(future);
    const float factor = distance > 5.f ? 1.f : 5.f / (0.00001f + distance);
    ahead = ahead - factor * plane.GetNormal();
  }
}

void CSwarmBasics::MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead) {
  CScriptWaypoint* wp = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.mTargetWaypoint));
  if (wp) {
    if (!wp->GetActive() ||
        boid.mSurfacePlane.GetHeight(boid.GetTranslation()) > -mWaypointGoalRadius) {
      rstl::vector< TUniqueId > nextWaypoints;
      nextWaypoints.reserve(8);
      for (rstl::vector< SConnection >::const_iterator it = wp->GetConnectionList().begin();
           it != wp->GetConnectionList().end(); ++it) {
        if (it->msg == kSM_Next) {
          TUniqueId uid = mgr.GetIdForScript(it->objId);
          const CScriptWaypoint* next = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(uid));
          if (next && next->GetActive()) {
            nextWaypoints.push_back_unsafe(uid);
          }
        }
      }
      boid.mTargetWaypoint = kInvalidUniqueId;
      uint count = nextWaypoints.size();
      if (count != 0) {
        if (count > 1) {
          boid.mTargetWaypoint = nextWaypoints[mgr.Random()->Next() % count];
        } else {
          boid.mTargetWaypoint = nextWaypoints[0];
        }
      }
      if (CScriptWaypoint* next =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(boid.mTargetWaypoint))) {
        CUnitVector3f normal((next->GetTranslation() - wp->GetTranslation()).AsNormalized());
        boid.mSurfacePlane = CPlane(next->GetTranslation(), normal);
        wp = next;
      } else {
        boid.mActive = false;
        if (boid.mHasLoopedSound) {
          StopLoopedSound(boid, mLocomotionSounds);
        }
        return;
      }
    }
    const float weight = mMoveToWaypointWeight;
    ahead += weight * (wp->GetTranslation() - boid.GetTranslation()).AsNormalized();
  }
}

TUniqueId CSwarmBasics::GetWaypointForState(EScriptObjectState state, CStateManager& mgr) {
  rstl::vector< TUniqueId > waypoints;
  waypoints.reserve(8);
  for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
       it != GetConnectionList().end(); ++it) {
    if (it->state == state && it->msg == kSM_Follow) {
      TUniqueId uid = mgr.GetIdForScript(it->objId);
      if (TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(uid)) && waypoints.size() < 8u) {
        waypoints.push_back_unsafe(uid);
      }
    }
  }
  uint count = waypoints.size();
  if (count != 0) {
    if (count > 1) {
      return waypoints[mgr.Random()->Next() % count];
    }
    return waypoints[0];
  }
  return kInvalidUniqueId;
}

void CSwarmBasics::ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info, CStateManager& mgr) {
  const float radiusSquared = info.GetRadius() * info.GetRadius();
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      const CVector3f boidPos = it->GetTranslation();
      if ((boidPos - pos).MagSquared() < radiusSquared) {
        it->mHealth -= info.GetRadiusDamage(mDamageVulnerability);
        if (it->mHealth <= 0.f) {
          KillBoid(*it, mgr, info.GetWeaponMode());
        }
      }
    }
  }
}

void CSwarmBasics::Touch(CActor& actor, CStateManager& mgr) {
  CActor::Touch(actor, mgr);
  if (CGameProjectile* projectile = TCastToPtr< CGameProjectile >(actor)) {
    const CDamageInfo& damage = projectile->GetCurrentDamageInfo();
    if (mDamageVulnerability.WeaponHits(damage.GetWeaponMode(), 0)) {
      const rstl::optional_object< CAABox > touchBounds = projectile->GetTouchBounds();
      if (touchBounds) {
        const CAABox projectileBounds = *touchBounds;
        const CVector3f extent = mTouchRadius * CVector3f::One();
        if (CLightComboProjectile* light = TCastToPtr< CLightComboProjectile >(projectile)) {
          if (light->CanCreateRay()) {
            for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
              if (it->mActive && it->xaa_ == kInvalidUniqueId) {
                const CVector3f pos = it->GetTranslation();
                const CAABox bounds(pos - extent, pos + extent);
                if (bounds.DoBoundsOverlap(projectileBounds)) {
                  const TUniqueId rayId = light->CreateRay(0.3f, mgr, pos);
                  if (rayId != kInvalidUniqueId) {
                    it->xa8_ = light->GetUniqueId();
                    it->xaa_ = rayId;
                    break;
                  }
                }
              }
            }
          }
        } else {
          for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
            if (it->mActive) {
              const CVector3f boidPos = it->GetTranslation();
              const CAABox bounds(boidPos - extent, boidPos + extent);
              if (bounds.DoBoundsOverlap(projectileBounds)) {
                CEnergyProjectile* energy = TCastToPtr< CEnergyProjectile >(projectile);
                if (energy && !TCastToPtr< CLightComboProjectile >(energy)) {
                  const CVector3f pos = it->GetTranslation();
                  if (!energy->Explode(pos, -1.f * energy->GetTransform().GetForward(),
                                       kWCR_EnemyNormal, mgr, mDamageVulnerability,
                                       GetUniqueId())) {
                    mgr.SendScriptMsg(this, energy->GetUniqueId(), kSM_HitObject);
                    mgr.SendScriptMsg(this, energy->GetUniqueId(), kSM_ReflectedDamage);
                    SendScriptMsgs(kSS_ReflectedDamage, mgr);
                  } else {
                    mgr.ApplyDamageToWorld(energy->GetOwnerId(), *energy, pos,
                                           energy->GetCurrentDamageInfo(), energy->GetFilter());
                  }
                  break;
                }
                it->mHealth -= damage.GetDamage(mDamageVulnerability);
                if (it->mHealth <= 0.f) {
                  KillBoid(*it, mgr, damage.GetWeaponMode());
                }
              }
            }
          }
        }
      }
    }
  }
  if (CPlayer* player = TCastToPtr< CPlayer >(actor)) {
    float radius = mPlayerTouchRadius;
    const CVector3f playerPos = player->GetTranslation();
    if (close_enough(radius, 0.f)) {
      radius = mTouchRadius;
    }
    const CAABox playerBounds = *player->GetTouchBounds();
    bool ballDamage = true;
    bool cannonBall = false;
    if ((player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
             ? player->GetMorphballTransitionState()
             : CPlayer::kMS_Unmorphed) == CPlayer::kMS_Morphed &&
        player->GetPlayerState()->GetItemAmount(CPlayerState::kIT_CannonBall, true) != 0) {
      cannonBall = true;
    }
    if (!cannonBall && !player->GetMorphBall()->InScrewAttackMode()) {
      ballDamage = false;
    }
    for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
      if (it->mActive && it->mFreezeTimer <= 0.f) {
        const CVector3f extent(radius, radius, radius);
        const CVector3f boidPos = it->GetTranslation();
        const CAABox bounds = CAABox(boidPos - extent, boidPos + extent);
        if (playerBounds.DoBoundsOverlap(bounds) && mDamageCooldownTimer <= 0.f) {
          if (!ballDamage) {
            mgr.ApplyDamage(
                GetUniqueId(), player->GetUniqueId(), GetUniqueId(), mDamage,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                CVector3f::Zero());
            mDamageCooldownTimer = mDamageCooldown;
          }
          BoidCollidedWithPlayerCallback(mgr, *it);
          break;
        }
      }
    }
    const bool boosting = player->GetMorphBall()->GetBallState() == CMorphBall::kBS_Boost;
    if (ballDamage || boosting) {
      const CDamageInfo ballInfo =
          ballDamage ? gpTweakBall->GetCannonBallDamage() : gpTweakBall->GetBoostBallDamage();
      ApplyRadiusDamage(playerPos, ballInfo, mgr);
    }
  }
}

void CSwarmBasics::SetExplodeTimers(const CVector3f& pos, float radius, float minTime,
                                    float maxTime) {
  const float radiusSquared = radius * radius;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive && it->mFreezeTimer <= 0.f) {
      const float distanceSquared = (it->GetTranslation() - pos).MagSquared();
      if (distanceSquared < radiusSquared) {
        const float time = (distanceSquared / radiusSquared) * (maxTime - minTime) + minTime;
        if (it->mTimeToExplode > time || it->mTimeToExplode == 0.f) {
          it->mTimeToExplode = time;
        }
      }
    }
  }
}

bool CSwarmBasics::IsBoidVisibleForLockOn(const CStateManager& mgr, const CBoid& boid,
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

int CSwarmBasics::GetLockOnIndex(CStateManager& mgr) const {
  if (!x4f0_26_) {
    return -1;
  }
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
    if (result != -1 && x54c_24_ && !playerOrbiting) {
      result = FindBestLockOnIndex(mgr);
    }
    return result;
  }
  return FindBestLockOnIndex(mgr);
}

int CSwarmBasics::FindBestLockOnIndex(CStateManager& mgr) const {
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

// Guessed name: orders seeker candidates by descending view alignment.
struct SSeekerCandidateSorter {
  SSeekerCandidateSorter() {}
  bool operator()(const rstl::pair< uint, float >& a, const rstl::pair< uint, float >& b) const {
    return a.second > b.second;
  }
};

void CSwarmBasics::AssignSeekerBoids(CStateManager& mgr, const rstl::vector< uint >& taken,
                                     uint numNeeded, rstl::vector< uint >& out) {
  float maxDistSq = mgr.GetPlayer(0)->GetOrbitMaxTargetDistance();
  maxDistSq *= maxDistSq;
  CTransform4f camXf = mgr.GetCameraManager(0)->GetFirstPersonCamera()->GetTransform();
  CVector3f camPos = camXf.GetTranslation();
  CVector3f camFwd = camXf.GetForward();
  rstl::vector< rstl::pair< uint, float > > candidates;
  candidates.reserve(mBoids.size());
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      CVector3f delta = it->GetTranslation() - camPos;
      if (delta.MagSquared() > maxDistSq) {
        continue;
      }
      if (delta.CanBeNormalized()) {
        float dot = CVector3f::Dot(camFwd, delta.AsNormalized());
        if (dot > 0.5f) {
          candidates.push_back_unsafe(rstl::pair< uint, float >(uint(it->mIndex), dot));
        }
      }
    }
  }
  rstl::sort(candidates.begin(), candidates.end(), SSeekerCandidateSorter());
  for (uint i = 0; out.size() < numNeeded && i < candidates.size(); ++i) {
    uint idx = candidates[i].first;
    bool found = false;
    for (uint j = 0; j < taken.size(); ++j) {
      if (idx == taken[j]) {
        found = true;
        break;
      }
    }
    if (!found) {
      out.push_back_unsafe(idx);
    }
  }
}

void CSwarmBasics::UpdateLockOnBlend(int prevIndex, int newIndex, float dt) {
  if (x54c_24_) {
    if (newIndex >= 0) {
      if (prevIndex >= 0 && prevIndex != newIndex) {
        if (!x54c_25_) {
          x550_ = mLastOrbitPosition;
        }
        x54c_25_ = true;
        x55c_ = 0.f;
      }
      if (x54c_25_) {
        x55c_ += 3.f * dt;
        if (x55c_ >= 1.f) {
          x54c_25_ = false;
        }
      }
    } else {
      x54c_25_ = false;
    }
  }
}

CVector3f CSwarmBasics::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mLockOnIndex == -1) {
    return mLastOrbitPosition;
  }
  if (x54c_24_ && x54c_25_ && dt == 0.f) {
    return (1.f - x55c_) * x550_ + x55c_ * mLastOrbitPosition;
  }
  return mLastOrbitPosition + dt * mBoids[mLockOnIndex].mVelocity;
}

CVector3f CSwarmBasics::GetOrbitPosition(const CStateManager& mgr) const {
  return mLastOrbitPosition;
}

void CSwarmBasics::KillBoid(CBoid& boid, CStateManager& mgr, const CWeaponMode& weapon) {
  mHealthInfo.SetCauseOfDeathWeapon(weapon, kInvalidUniqueId, kInvalidUniqueId, false, false);
  mLastKilledOffset = boid.GetTranslation();
  AddParticle(boid.GetTransform());
  boid.mActive = false;
  if (boid.mHasLoopedSound && boid.mAttacking) {
    StopLoopedSound(boid, mAttackSounds);
  } else if (boid.mHasLoopedSound) {
    StopLoopedSound(boid, mLocomotionSounds);
  }
  QueueDeathMessage(mgr);
  x4f1_24_ = true;
  if (boid.xaa_ != kInvalidUniqueId) {
    CLightComboProjectile* proj = TCastToPtr< CLightComboProjectile >(mgr.ObjectById(boid.xa8_));
    if (TCastToConstPtr< CPlasmaProjectile >(mgr.GetObjectById(boid.xaa_))) {
      if (proj) {
        proj->RequestRayReset(mgr, boid.xaa_, false);
        boid.xaa_ = kInvalidUniqueId;
        boid.xa8_ = kInvalidUniqueId;
      } else {
        boid.xaa_ = kInvalidUniqueId;
        boid.xa8_ = kInvalidUniqueId;
      }
    } else {
      boid.xaa_ = kInvalidUniqueId;
      boid.xa8_ = kInvalidUniqueId;
    }
  }
}

void CSwarmBasics::StopLoopedSound(CBoid& boid, rstl::vector< TLoopedSound >& sounds) {
  for (uint i = 0; i < sounds.size(); ++i) {
    if (boid.mIndex == sounds[i].second) {
      CSfxManager::SfxStop(sounds[i].first);
      sounds[i].first = CSfxHandle();
      boid.mHasLoopedSound = false;
      return;
    }
  }
}

void CSwarmBasics::AddParticle(const CTransform4f& xf) {
  if (mParticleGenerator.get()) {
    mParticleGenerator->SetParticleEmission(true);
    mParticleGenerator->SetTranslation(xf.GetTranslation());
    mParticleGenerator->ForceParticleCreation(mNumDeathParticles);
    mParticleGenerator->SetParticleEmission(false);
  }
}

void CSwarmBasics::UpdateParticles(float dt) {
  if (mParticleGenerator.get()) {
    mParticleGenerator->Update(dt);
  }
}

void CSwarmBasics::RenderParticles() const {
  if (mParticleGenerator.get()) {
    gpRender->AddParticleGen(*mParticleGenerator);
  }
}

void CSwarmBasics::FreezeCollision(const CMarkerGrid& grid) {
  const float radius = mTouchRadius * mTouchRadius;
  const float xy = radius + 0.3f;
  const float z = radius + 0.5f;
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      const CVector3f extent(xy, xy, z);
      const CAABox bounds = CAABox(it->GetTranslation() - extent, it->GetTranslation() + extent);
      if (grid.AABoxTouchesData(bounds, 1)) {
        it->mFreezeTimer = 1.f;
      }
    }
  }
}

int CSwarmBasics::EvaluateActiveBoidCount() const {
  int count = 0;
  for (int i = 0; i < mBoids.size(); ++i) {
    if (mBoids[i].mActive) {
      ++count;
    }
  }
  return count;
}

CVector3f CSwarmBasics::FindClosestCell(const CVector3f& pos) const {
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

CSwarmBasics::CBoid* CSwarmBasics::GetClosestPartitionList(const CVector3f& pos) const {
  float minDistance = FLT_MAX;
  CBoid* result = nullptr;
  for (int x = 0; x < 5; ++x) {
    int rowIndex = x;
    for (int y = 0; y < 5; ++y, rowIndex += 5) {
      for (int z = 0; z < 5; ++z) {
        CBoid* list = mPartitionedBoidLists[rowIndex + z * 25];
        if (list != nullptr) {
          const CAABox bounds = BoxForPosition(x, y, z, 0.1f);
          const float distance = (bounds.GetCenterPoint() - pos).MagSquared();
          if (distance < minDistance) {
            result = list;
            minDistance = distance;
          }
        }
      }
    }
  }
  return result;
}

uint CSwarmBasics::UpdateLoopedSounds(uint maxEmitters, int partitionIndex,
                                      rstl::vector< TLoopedSound >& sounds) {
  uint active = 0;
  for (uint i = 0; i < maxEmitters; ++i) {
    if (sounds[i].first) {
      CBoid& boid = mBoids[sounds[i].second];
      if (boid.mPartitionIndex != partitionIndex || partitionIndex == -1) {
        ++boid.xb1_;
        if (boid.xb1_ > x560_) {
          CSfxManager::SfxStop(sounds[i].first);
          sounds[i].first = CSfxHandle();
          boid.mHasLoopedSound = false;
        }
      } else {
        CSfxManager::UpdateEmitter(sounds[i].first, boid.GetTranslation(), CVector3f::Zero(), 127);
        ++active;
        boid.xb1_ = 0;
      }
    }
  }
  return active;
}

void CSwarmBasics::UpdateClosestPartitionLoopedSounds(const CVector3f& listener,
                                                      rstl::vector< TLoopedSound >& sounds,
                                                      uint maxEmitters, ushort sfx,
                                                      ELoopedSoundType type) {
  if (sounds.size() > 0) {
    CBoid* list = GetClosestPartitionList(listener);
    if (list) {
      uint active = UpdateLoopedSounds(maxEmitters, list->mPartitionIndex, sounds);
      if (active < maxEmitters) {
        CBoid* candidates[64];
        uint count = 0;
        for (CBoid* boid = list; boid && count < 64; boid = boid->mNext) {
          if (CanStartLoopedSound(*boid, type)) {
            ++count;
            const CVector3f delta = listener - boid->GetTranslation();
            boid->mDistanceSquaredToSoundListener = delta.MagSquared();
            candidates[count - 1] = boid;
          }
        }
        if (count != 0) {
          if (count > maxEmitters - active) {
            qsort(candidates, count, sizeof(CBoid*), CompareBoidsByListenerDistance);
          }
          CBoid** it = candidates;
          uint used = 0;
          for (uint i = 0; i < maxEmitters && used < count; ++i) {
            if (!sounds[i].first) {
              StartLoopedSound(**it, sounds, sfx, i);
              ++it;
              ++used;
            }
          }
        }
      }
    } else {
      UpdateLoopedSounds(maxEmitters, -1, sounds);
    }
  }
}

bool CSwarmBasics::AddLoopedSoundToHandlesList(CBoid& boid, rstl::vector< TLoopedSound >& sounds,
                                               ushort sfx) {
  for (uint i = 0; i < sounds.size(); ++i) {
    if (!sounds[i].first) {
      StartLoopedSound(boid, sounds, sfx, i);
      return true;
    }
  }
  return false;
}

void CSwarmBasics::StartLoopedSound(CBoid& boid, rstl::vector< TLoopedSound >& sounds, ushort sfx,
                                    uint slot) {
  sounds[slot].first = AddLoopedEmitter(boid.GetTranslation(), sfx);
  sounds[slot].second = boid.mIndex;
  boid.mHasLoopedSound = true;
}

void CSwarmBasics::UpdateLoopedSoundPositions(const rstl::vector< TLoopedSound >& sounds) const {
  uint count = sounds.size();
  for (uint i = 0; i < count; ++i) {
    if (sounds[i].first) {
      CSfxManager::UpdateEmitter(sounds[i].first, mBoids[sounds[i].second].GetTranslation(),
                                 CVector3f::Zero(), 127);
    }
  }
}

bool CSwarmBasics::CanStartLoopedSound(const CBoid& boid, ELoopedSoundType type) const {
  switch (type) {
  case kLST_Locomotion:
    return !boid.mHasLoopedSound && !boid.mAttacking && boid.mActive;
  case kLST_Attack:
    return !boid.mHasLoopedSound && boid.mAttacking && boid.mActive;
  default:
    return false;
  }
}

CSfxHandle CSwarmBasics::AddLoopedEmitter(const CVector3f& pos, ushort sfx) {
  const uchar maxVol = mMaxVolume;
  const uchar minVol = mMinVolume;
  CAudioSys::C3DEmitterParmData parms(mMaxAudibleDistance, mSoundFallOff, 1, maxVol, minVol);
  parms.mPos = pos;
  parms.mDir = CVector3f::Zero();
  parms.mSfxId = sfx;
  return CSfxManager::AddEmitter(parms, GetCurrentAreaId().Value(), true, true);
}

void CSwarmBasics::UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume) {
  int count;
  const CSoundPOINode* nodes = animData.GetSoundPOIList(count);
  if (count > 0 && nodes != nullptr) {
    for (int i = 0; i < count; ++i) {
      const CSoundPOINode& node = nodes[i];
      const float roll = mgr.Random()->Float();
      if (roll <= node.GetWeight()) {
        const int character = node.GetCharacterIndex();
        if (node.GetPoiType() == kPT_Sound &&
            (character == -1 || character == animData.GetCharacterIndex())) {
          const uint soundId = node.GetSoundId();
          const int area = GetCurrentAreaId().Value();
          const ushort sfx = soundId;
          if ((soundId & 0x80000000) == 0) {
            const CBoid& boid =
                mBoids[mActiveBoidIndices[mgr.Random()->Next() % mActiveBoidIndices.size()]];
            const CVector3f pos = boid.GetTranslation();
            static float maxDistance = node.GetMaxDistance();
            static float falloff = node.GetFallOff();
            CAudioSys::C3DEmitterParmData params(maxDistance, falloff, 1, mMaxVolume, mMinVolume);
            params.mPos = pos;
            params.mDir = CVector3f::Zero();
            params.mSfxId = sfx;
            CSfxManager::AddEmitter(params, area, true, false);
          }
        }
      }
    }
  }
}

CAreaCollisionCache CSwarmBasics::GetAreaCollisionCacheForPartition(int x, int y, int z) const {
  return CAreaCollisionCache(BoxForPosition(x, y, z, mBoidRadius + 0.5f));
}

void CSwarmBasics::BoidCollidedWithPlayerCallback(CStateManager& mgr, CBoid& boid) {
  if (x4f0_29_) {
    KillBoid(boid, mgr, CWeaponMode());
  }
}

bool CSwarmBasics::ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const {
  return true;
}

void CSwarmBasics::BoidCollidedCallback(CStateManager& mgr, CBoid& boid) {}

void CSwarmBasics::QueueDeathMessage(CStateManager& mgr) {
  if (x530_ < 10) {
    SendScriptMsgs(kSS_DeathRattle, mgr);
    SendScriptMsgs(kSS_Dead, mgr);
    ++x530_;
  } else {
    ++x534_;
  }
}

void CSwarmBasics::FlushDeathMessages(CStateManager& mgr) {
  int count = x534_;
  int avail = 10 - x530_;
  if (avail < count) {
    count = avail;
  }
  for (int i = 0; i < count; ++i) {
    SendScriptMsgs(kSS_DeathRattle, mgr);
    SendScriptMsgs(kSS_Dead, mgr);
  }
  x534_ -= count;
  x530_ = 0;
}

void CSwarmBasics::FreezeBoids(const CVector3f& position, float radius) {
  for (rstl::vector< CBoid >::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      CVector3f delta = position - it->GetTranslation();
      if (delta.MagSquared() < radius * radius) {
        it->mFreezeTimer = mFreezeDuration;
      }
    }
  }
}

void CSwarmBasics::UpdateSeekerTargets(CStateManager& mgr) {
  uint numTargets = mSeekerTargets.size();
  const rstl::reserved_vector< rstl::pair< TUniqueId, float >, 5 >& gunTargets =
      mgr.GetPlayer(0)->GetGun()->GetSeekerTargets();
  uint numGunTargets = gunTargets.size();
  rstl::vector< TUniqueId > lostTargets;
  rstl::vector< uint > keptBoids;
  lostTargets.reserve(numTargets);
  keptBoids.reserve(numTargets);
  for (uint i = 0; i < numTargets; ++i) {
    bool found = false;
    TUniqueId uid = mSeekerTargets[i];
    for (uint j = 0; j < numGunTargets; ++j) {
      if (gunTargets[j].first == uid) {
        found = true;
        keptBoids.push_back_unsafe(mSeekerBoidIndices[i]);
        break;
      }
    }
    if (!found) {
      lostTargets.push_back_unsafe(uid);
    }
  }
  int numLost = lostTargets.size();
  rstl::vector< uint > newBoids;
  newBoids.reserve(numLost);
  AssignSeekerBoids(mgr, keptBoids, numLost, newBoids);
  uint numNew = newBoids.size();
  for (uint i = 0; i < numNew; ++i) {
    TUniqueId uid = lostTargets[i];
    for (uint j = 0; j < numTargets; ++j) {
      if (uid == mSeekerTargets[j]) {
        mSeekerBoidIndices[j] = newBoids[i];
        break;
      }
    }
  }
  for (uint i = numNew; i < numLost; ++i) {
    TUniqueId uid = lostTargets[i];
    for (uint j = 0; j < numTargets; ++j) {
      if (uid == mSeekerTargets[j]) {
        if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mSeekerTargets[j]))) {
          act->SetActive(false);
        }
        mSeekerBoidIndices[j] = -1;
        break;
      }
    }
  }
  for (uint i = 0; i < numTargets; ++i) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mSeekerTargets[i]))) {
      int idx = mSeekerBoidIndices[i];
      if (idx != -1) {
        act->SetActive(true);
        act->SetTransform(mBoids[idx].GetTransform());
      } else {
        act->SetActive(false);
      }
    }
  }
}

TUniqueId CSwarmBasics::GetSeekerTargetLockedOn() const {
  int lockOn = mLockOnIndex;
  if (lockOn == -1) {
    return kInvalidUniqueId;
  }
  for (uint i = 0; i < mSeekerTargets.size(); ++i) {
    if (lockOn == mSeekerBoidIndices[i]) {
      return mSeekerTargets[i];
    }
  }
  return kInvalidUniqueId;
}

const CHealthInfo* CSwarmBasics::GetHealthInfo() const { return &mHealthInfo; }

CHealthInfo* CSwarmBasics::HealthInfo() { return &mHealthInfo; }

void CSwarmBasics::FinishConstruction() {}

static void* NullSwarmBasicsFactory() { return nullptr; }

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SSwarmBasics_FuncPtrs funcPtrs;
  funcPtrs.mFactory = &NullSwarmBasicsFactory;
  SetSSwarmBasics_FuncPtrs(&funcPtrs);
}

void RELMain() { SetFuncPtrs(); }

void RELExit() { SetSSwarmBasics_FuncPtrs(nullptr); }
#endif
