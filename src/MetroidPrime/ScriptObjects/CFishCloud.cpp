#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFishCloud.hpp"
#include "MetroidPrime/ScriptLoader/SLdrFishCloudModifier.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "rstl/algorithm.hpp"

#include <float.h>
#include <math.h>

// The original divides by multiplying with the reciprocal (Echoes' CVector3f::operator/ divides
// each component); the operator is kept as is for the DOL units that already match.
static inline float FishPow(const float x, const float y) { return pow(x, y); }

static inline CVector3f VecDiv(const CVector3f& vec, const float f) {
  const float inv = 1.f / f;
  float x = vec.GetX() * inv;
  float y = vec.GetY() * inv;
  float z = vec.GetZ() * inv;
  return CVector3f(x, y, z);
}

CFishCloudModifier::~CFishCloudModifier() {}

// Local copy of CModelData::CModelDataNull.
static CModelData CModelDataNull() { return CModelData(); }

CFishCloudModifier::CFishCloudModifier(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, const CVector3f& pos,
                                       bool isRepulsor, bool swirl, float radius, float priority)
: CActor(uid, name, info, 0, CTransform4f::Translate(pos), CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, mRadius(radius)
, mPriority(priority)
, mIsRepulsor(isRepulsor)
, mSwirl(swirl) {}

void CFishCloudModifier::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Activate:
  case kSM_AreaLoaded:
    if (GetActive()) {
      AddSelf(mgr);
    }
    break;
  case kSM_Deactivate:
  case kSM_Delete:
    RemoveSelf(mgr);
    break;
  default:
    break;
  }
}

void CFishCloudModifier::AddSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state != kSS_Modify || it->msg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->objId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (mIsRepulsor) {
        cloud->AddRepulsor(GetUniqueId(), mSwirl, mRadius, mPriority);
      } else {
        cloud->AddAttractor(GetUniqueId(), mSwirl, mRadius, mPriority);
      }
    }
  }
}

void CFishCloudModifier::RemoveSelf(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (rstl::vector< SConnection >::const_iterator it = connections.begin();
       it != connections.end(); ++it) {
    if (it->state != kSS_Modify || it->msg != kSM_Follow) {
      continue;
    }
    const TUniqueId uid = mgr.GetIdForScript(it->objId);
    if (uid == kInvalidUniqueId) {
      continue;
    }
    if (CFishCloud* cloud = TCastToPtr< CFishCloud >(mgr.ObjectById(uid))) {
      if (mIsRepulsor) {
        cloud->RemoveRepulsor(GetUniqueId());
      } else {
        cloud->RemoveAttractor(GetUniqueId());
      }
    }
  }
}

CFishCloud::CModifierSource::CModifierSource(const TUniqueId& source, bool repulsor, bool swirl,
                                             float radius, float priority)
: mSource(source), mRadius(radius), mPriority(priority), mIsRepulsor(repulsor), mIsSwirl(swirl) {}

bool CFishCloud::CModifierSource::operator<(const CModifierSource& other) const {
  if (mSource == other.mSource) {
    return mIsRepulsor < other.mIsRepulsor;
  }
  return mSource < other.mSource;
}

CFishCloud::CBoid::CBoid(const CVector3f& pos, const CVector3f& vel, float scale)
: mPos(pos), mVel(vel), mScale(scale), mNext(nullptr), mActive(true) {}

CFishCloud::CFishCloud(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CVector3f& scale, const CTransform4f& xf, const CModelData& mData,
                       const CAnimRes& aRes, int numBoids, float speed, float separationRadius,
                       float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
                       float weaponRepelMagnitude, float playerRepelMagnitude,
                       float containmentMagnitude, float scatterVel, float maxScatterAngle,
                       float weaponRepelDampingSpeed, float playerRepelDampingSpeed,
                       float playerBallPriority, float playerBallDistance, float containmentRadius,
                       int updateShift, const CColor& color, bool killable, float weaponKillRadius,
                       CAssetId part1, int partCount1, CAssetId part2, int partCount2,
                       CAssetId part3, int partCount3, CAssetId part4, int partCount4, int deathSfx,
                       bool repelFromThreats, const CActorParameters& actParms)
: CActor(uid, name, info, 0, xf, mData, CMaterialList(kMT_NoStepLogic), actParms, kInvalidUniqueId)
, mUpdateMask((1 << updateShift) - 1)
, mScale(scale)
, mRandomMovementTimer(0.f)
, mSpeed(speed)
, mNumBoids(numBoids)
, mSeparationRadius(separationRadius)
, mCohesionMagnitude(cohesionMagnitude)
, mAlignmentWeight(alignmentWeight)
, mSeparationMagnitude(separationMagnitude)
, mWeaponRepelMagnitude(weaponRepelMagnitude)
, mPlayerRepelMagnitude(playerRepelMagnitude)
, mScatterVel(scatterVel)
, mMaxScatterAngle(maxScatterAngle)
, mContainmentMagnitude(containmentMagnitude)
, mPlayerBallPriority(playerBallPriority)
, mPlayerBallDistance(playerBallDistance)
, mPlayerRepelDampingSpeed(playerRepelDampingSpeed)
, mWeaponRepelDampingSpeed(weaponRepelDampingSpeed)
, mPlayerRepelDamping(playerRepelDampingSpeed)
, mWeaponRepelDamping(weaponRepelDampingSpeed)
, mColor(color)
, mWeaponKillRadius(weaponKillRadius)
, mContainmentRadius(containmentRadius)
, mDeathSfx(deathSfx == -1 ? CSfxManager::kInternalInvalidSfxId : deathSfx)
, mPartitionPitch(CVector3f::Zero())
, mOoPartitionPitch(CVector3f::Zero())
, mRandomMovement(false)
, mWorldSpace(false)
, mEnableWeaponRepelDamping(false)
, mValidModel(false)
, mKillable(killable)
, mRepelFromThreats(repelFromThreats)
, mEnablePlayerRepelDamping(false)
, mUpdateWithoutPartitions(false) {
  mModifierSources.reserve(10);
  const CVector3f& forward = GetTransform().GetForward();
  const CVector3f& up = GetTransform().GetUp();
  const CVector3f& right = GetTransform().GetRight();
  mWorldSpace = !(close_enough(right.GetX(), 1.f) && close_enough(right.GetX(), 0.f) &&
                  close_enough(right.GetX(), 0.f) && close_enough(forward.GetX(), 0.f) &&
                  close_enough(forward.GetX(), 1.f) && close_enough(forward.GetX(), 0.f) &&
                  close_enough(up.GetX(), 0.f) && close_enough(up.GetX(), 0.f) &&
                  close_enough(up.GetX(), 1.f));
  if (aRes.GetId() != kInvalidAssetId) {
    for (int i = 0; i < 4; ++i) {
      mModels.push_back(rs_new CModelData(aRes));
    }
    mValidModel = true;
    mDisplayList = rs_new SwarmRenderHelpers::CSwarmDisplayList(
        **mModels[0]->GetAnimationData()->GetModelData());
  }
  if (part1 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part1)));
  }
  if (part2 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part2)));
  }
  if (part3 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part3)));
  }
  if (part4 != kInvalidAssetId) {
    mParticleDescs.push_back(gpSimplePool->GetObj(SObjectTag('PART', part4)));
  }
  for (int i = 0; i < mParticleDescs.size(); ++i) {
    mParticleGens.push_back(rs_new CElementGen(mParticleDescs[i]));
    mParticleGens[i]->SetParticleEmission(false);
  }
  mDeathParticleCounts.push_back(partCount1);
  mDeathParticleCounts.push_back(partCount2);
  mDeathParticleCounts.push_back(partCount3);
  mDeathParticleCounts.push_back(partCount4);
  const CAABox& aabb = GetBoundingBox();
  mPartitionPitch = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * (1.f / 7.f);
  mOoPartitionPitch = CVector3f(1.f / mPartitionPitch.GetX(), 1.f / mPartitionPitch.GetY(),
                                1.f / mPartitionPitch.GetZ());
}

CFishCloud::~CFishCloud() {}

void CFishCloud::InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which) {
  mModelStates.clear();
  for (int i = 0; i < 4; ++i) {
    mModelStates.push_back(
        SwarmRenderHelpers::CSwarmSkinnedModelState(mModels[i]->PickAnimatedModel(which)));
    mModels[i]->EnableLooping(true);
    mModels[i]->AdvanceAnimation(
        (float(i) / 4.f) *
            mModels[i]->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")),
        mgr, GetCurrentAreaId(), true);
  }
}

void CFishCloud::PreRenderAllViewports(CStateManager& mgr) {
  const CAABox aabb = GetBoundingBox();
  SetOtherBounds(aabb);
  SetRenderBounds(aabb);
  UpdatePortalSystemState(mgr);
}

CAABox CFishCloud::GetBoundingBox() const {
  const CAABox aabb = GetUntransformedBoundingBox();
  return aabb.GetTransformedAABox(GetTransform());
}

CAABox CFishCloud::GetUntransformedBoundingBox() const {
  const CVector3f extent(0.75f * mScale.GetX(), 0.75f * mScale.GetY(), 0.75f * mScale.GetZ());
  return CAABox(-extent, extent);
}

bool CFishCloud::PointInBox(const CAABox& aabb, const CVector3f& point) const {
  if (!mWorldSpace) {
    return aabb.PointInside(point);
  }
  const CVector3f localPoint = GetTransform().TransposeRotate(point - GetTranslation());
  return GetUntransformedBoundingBox().PointInside(localPoint);
}

CPlane CFishCloud::FindClosestPlane(const CAABox& aabb, const CVector3f& point) const {
  if (!mWorldSpace) {
    float minDistance = FLT_MAX;
    CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
    for (int i = 0; i < 6; ++i) {
      const CTri tri = aabb.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
      const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
      const float distance = plane.GetHeight(point);
      if (distance >= 0.f && distance < minDistance) {
        minFace = static_cast< CAABox::EBoxFaceId >(i);
        minDistance = distance;
      }
    }
    const CTri tri = aabb.GetTri(minFace, 0);
    return CPlane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
  }
  const CTransform4f& xf = GetTransform();
  const CVector3f localPoint = xf.TransposeRotate(point - GetTranslation());
  const CAABox localBounds = GetUntransformedBoundingBox();
  float minDistance = FLT_MAX;
  CAABox::EBoxFaceId minFace = CAABox::kF_YMin;
  for (int i = 0; i < 6; ++i) {
    const CTri tri = localBounds.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
    const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
    const float distance = plane.GetHeight(localPoint);
    if (distance >= 0.f && distance < minDistance) {
      minFace = static_cast< CAABox::EBoxFaceId >(i);
      minDistance = distance;
    }
  }
  const CTri tri = localBounds.GetTri(minFace, 0);
  return CPlane(xf * tri.GetPointA(), xf * tri.GetPointC(), xf * tri.GetPointB());
}

CPlane CFishCloud::FindFarthestPlane(const CAABox& aabb, const CVector3f& point) const {
  if (!mWorldSpace) {
    float maxDistance = 0.f;
    CAABox::EBoxFaceId maxFace = CAABox::kF_YMin;
    for (int i = 0; i < 6; ++i) {
      const CTri tri = aabb.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
      const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
      const float distance = plane.GetHeight(point);
      if (distance >= 0.f && distance > maxDistance) {
        maxFace = static_cast< CAABox::EBoxFaceId >(i);
        maxDistance = distance;
      }
    }
    const CTri tri = aabb.GetTri(maxFace, 0);
    return CPlane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
  }
  const CTransform4f& xf = GetTransform();
  const CVector3f localPoint = xf.TransposeRotate(point - GetTranslation());
  const CAABox localBounds = GetUntransformedBoundingBox();
  float maxDistance = 0.f;
  CAABox::EBoxFaceId maxFace = CAABox::kF_YMin;
  for (int i = 0; i < 6; ++i) {
    const CTri tri = localBounds.GetTri(static_cast< CAABox::EBoxFaceId >(i), 0);
    const CPlane plane(tri.GetPointA(), tri.GetPointC(), tri.GetPointB());
    const float distance = plane.GetHeight(localPoint);
    if (distance >= 0.f && distance > maxDistance) {
      maxFace = static_cast< CAABox::EBoxFaceId >(i);
      maxDistance = distance;
    }
  }
  const CTri tri = localBounds.GetTri(maxFace, 0);
  return CPlane(xf * tri.GetPointA(), xf * tri.GetPointC(), xf * tri.GetPointB());
}

void CFishCloud::PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb) {
  CRandom16& random = *mgr.Random();
  const CPlane plane = FindClosestPlane(aabb, boid.mPos);
  boid.mPos -= plane.GetHeight(boid.mPos) * plane.GetNormal() + 0.0001f * plane.GetNormal();
  boid.mVel = CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f);
  if (!mWorldSpace) {
    if (!aabb.PointInside(boid.mPos)) {
      const CVector3f min = aabb.GetMinPoint();
      boid.mPos = CVector3f(random.Float() * aabb.GetWidth() + min.GetX(),
                            random.Float() * aabb.GetHeight() + min.GetY(),
                            random.Float() * aabb.GetDepth() + min.GetZ());
    }
  } else if (!PointInBox(aabb, boid.mPos)) {
    const CAABox localBounds = GetUntransformedBoundingBox();
    const CVector3f min = localBounds.GetMinPoint();
    const CVector3f pos(random.Float() * localBounds.GetWidth() + min.GetX(),
                        random.Float() * localBounds.GetHeight() + min.GetY(),
                        random.Float() * (localBounds.GetMaxPoint().GetZ() - min.GetZ()) +
                            min.GetZ());
    boid.mPos = GetTransform() * pos;
  }
}

void CFishCloud::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  CActor::AcceptScriptMsg(mgr, msg);
  switch (message) {
  case kSM_Activate:
  case kSM_Deactivate:
  case kSM_Delete:
  case kSM_AreaLoaded:
    break;
  case kSM_Create: {
    mBoids.reserve(mNumBoids);
    const CAABox bounds = GetUntransformedBoundingBox();
    CRandom16& random = *mgr.Random();
    const CVector3f& min = bounds.GetMinPoint();
    for (int i = 0; i < mBoids.capacity(); ++i) {
      const CVector3f pos(random.Float() * bounds.GetWidth() + min[kDX],
                          random.Float() * bounds.GetHeight() + min[kDY],
                          random.Float() * bounds.GetDepth() + min[kDZ]);
      mBoids.push_back_unsafe(CBoid(GetTransform() * pos,
                                    CVector3f(random.Float() - 0.5f, random.Float() - 0.5f, 0.f),
                                    0.2f * FishPow(random.Float(), 7.f) + 0.9f));
    }
    CreatePartitionList();
    if (mValidModel) {
      InitAnimBoids(mgr, CModelData::kWM_Normal);
    }
    break;
  }
  default:
    break;
  }
}

bool CFishCloud::AddAttractor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, false, swirl, radius, priority);
  TModifierSourceVector::iterator it =
      rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier);
  if (it != mModifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (mModifierSources.size() < mModifierSources.capacity()) {
    TModifierSourceVector::iterator insertIt =
        rstl::lower_bound(mModifierSources.begin(), mModifierSources.end(), modifier);
    mModifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

bool CFishCloud::AddRepulsor(TUniqueId source, bool swirl, float radius, float priority) {
  const CModifierSource modifier(source, true, swirl, radius, priority);
  TModifierSourceVector::iterator it =
      rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier);
  if (it != mModifierSources.end()) {
    it->SetAffectRadius(radius);
    it->SetAffectPriority(priority);
    return true;
  }
  if (mModifierSources.size() < mModifierSources.capacity()) {
    TModifierSourceVector::iterator insertIt =
        rstl::lower_bound(mModifierSources.begin(), mModifierSources.end(), modifier);
    mModifierSources.insert(insertIt, modifier);
    return true;
  }
  return false;
}

void CFishCloud::RemoveAttractor(TUniqueId source) {
  const CModifierSource modifier(source, false, false, 0.f, 0.f);
  TModifierSourceVector::iterator it =
      rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier);
  if (it != mModifierSources.end()) {
    mModifierSources.erase(it);
  }
}

void CFishCloud::RemoveRepulsor(TUniqueId source) {
  const CModifierSource modifier(source, true, false, 0.f, 0.f);
  TModifierSourceVector::iterator it =
      rstl::binary_find(mModifierSources.begin(), mModifierSources.end(), modifier);
  if (it != mModifierSources.end()) {
    mModifierSources.erase(it);
  }
}

rstl::optional_object< CAABox > CFishCloud::GetTouchBounds() const { return GetBoundingBox(); }

void CFishCloud::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    return;
  }
  mWeaponRepelDamping =
      rstl::max_val(0.f, mWeaponRepelDamping - mWeaponRepelDampingSpeed * dt * 0.1f);
  if (mEnableWeaponRepelDamping) {
    mWeaponRepelDamping =
        rstl::min_val(mWeaponRepelMagnitude, mWeaponRepelDampingSpeed * dt + mWeaponRepelDamping);
  }
  mPlayerRepelDamping =
      rstl::max_val(0.f, mPlayerRepelDamping - mPlayerRepelDampingSpeed * dt * 0.1f);
  if (mEnablePlayerRepelDamping) {
    mPlayerRepelDamping =
        rstl::min_val(mPlayerRepelMagnitude, mPlayerRepelDampingSpeed * dt + mPlayerRepelDamping);
  }
  mEnableWeaponRepelDamping = false;
  mEnablePlayerRepelDamping = false;
  ++mThinkCounter;
  UpdateParticles(dt);
  rstl::reserved_vector< CBoid*, 25 > nearList;
  UpdatePartitionList();
  CRandom16& random = *mgr.Random();
  const CAABox bounds = GetBoundingBox();
  int index = 0;
  for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it, ++index) {
    if (it->mActive && (index & mUpdateMask) == (mThinkCounter & mUpdateMask)) {
      nearList.clear();
      if (mUpdateWithoutPartitions) {
        OldBuildBoidNearList(it->mPos, mSeparationRadius, nearList);
      } else {
        BuildBoidNearList(it->mPos, mSeparationRadius, nearList);
      }
      for (int i = 0; i != 5; ++i) {
        switch (i) {
        case 1:
          ApplySeparation(*it, nearList);
          break;
        case 2:
          if (!mRandomMovement || random.Float() > mRandomMovementTimer) {
            ApplyCohesion(*it, nearList);
          }
          break;
        case 3:
          if (!mRandomMovement || random.Float() > mRandomMovementTimer) {
            ApplyAlignment(*it, nearList);
          }
          break;
        case 4:
          ApplyWander(mgr, *it);
          break;
        }
        if (it->mVel.MagSquared() > 3.2f) {
          break;
        }
      }
      if (!mRandomMovement && it->mVel.MagSquared() < 3.2f) {
        for (TModifierSourceVector::iterator mod = mModifierSources.begin();
             mod != mModifierSources.end(); ++mod) {
          if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(mod->GetSource()))) {
            if (mod->IsSwirl()) {
              ApplyRotation(*it, mod->GetAffectPriority(), actor->GetTranslation(),
                            mod->GetAffectRadius(), mod->mIsRepulsor);
            } else if (mod->IsRepulsor()) {
              ApplyRepulsion(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                             mod->GetAffectPriority());
            } else {
              ApplyAttraction(*it, actor->GetTranslation(), mod->GetAffectRadius(),
                              mod->GetAffectPriority());
            }
          } else {
            if (mod->IsRepulsor()) {
              RemoveRepulsor(mod->GetSource());
            } else {
              RemoveAttractor(mod->GetSource());
            }
            break;
          }
        }
      }
    }
  }
  for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      ApplyContainment(*it, bounds);
      CVector3f& velocity = it->mVel;
      const float speed = velocity.Magnitude();
      if (!close_enough(speed, 0.f)) {
        const float inverseSpeed = 1.f / speed;
        velocity *= inverseSpeed;
      }
      velocity.SetZ(0.99f * velocity.GetZ());
    }
  }
  if (mRandomMovementTimer > 0.f) {
    mRandomMovementTimer -= dt;
  } else {
    mRandomMovementTimer = 0.f;
    mRandomMovement = false;
  }
  for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive) {
      it->mPos += mSpeed * (dt * it->mVel);
      if (!PointInBox(bounds, it->mPos)) {
        PlaceBoid(mgr, *it, bounds);
      }
    }
  }
  if (mValidModel) {
    for (int i = 0; i < 4; ++i) {
      mModels[i]->AnimationData()->SetPlaybackRate(1.f);
      mModels[i]->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    }
  }
}

void CFishCloud::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  if (mValidModel) {
    for (int i = 0; i < 4; ++i) {
      mModels[i]->AnimationData()->PreRender();
    }
  }
  SetPreRenderClipped(false);
}

void CFishCloud::RenderBoid(int idx, const CBoid& boid, uint& drawMask) const {
  const uint modelIndex = idx & 3;
  uint mask = drawMask;
  CModelData& modelData = *mModels[modelIndex];
  CAnimData& animData = *modelData.AnimationData();
  CSkinnedModel& model = modelData.PickAnimatedModel(CModelData::kWM_Normal);
  const uint bit = 1 << modelIndex;
  if (mask & bit) {
    mask &= ~bit;
    animData.BuildPose();
    SwarmRenderHelpers::CSwarmSkinnedModelState& state =
        const_cast< SwarmRenderHelpers::CSwarmSkinnedModelState& >(mModelStates[modelIndex]);
    model.StoreCalculation(state.State(), &animData.Pose());
    state.StateToArrays();
  }
  gpRender->SetModelMatrix(CTransform4f::LookAt(boid.mPos, boid.mPos + boid.mVel));
  mDisplayList->DrawFromState(mModelStates[modelIndex]);
  drawMask = mask;
}

void CFishCloud::Render(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }
  const int alpha = GetRenderAlphaBufferAlpha(mgr);
  if (alpha != -1) {
    gpRender->SetDestinationAlpha(alpha);
  }
  const CModelFlags flags = CModelFlags::ColorModulate(mColor);
  RenderParticles();
  if (mValidModel) {
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    mDisplayList->SetMaterialCurrent(flags);
    uint drawMask = ~0;
    int index = 0;
    for (TBoidVector::const_iterator it = mBoids.begin(); it != mBoids.end(); ++it, ++index) {
      if (it->mActive) {
        RenderBoid(index, *it, drawMask);
      }
    }
  } else {
    const CModelData* modelData = GetModelData();
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
    const CModel& model = **modelData->PickStaticModel(CModelData::kWM_Normal);
    const CCubeModel& cubeModel = *model.GetModelInstance();
    cubeModel.TryLockTextures();
    model.PreDrawModel(flags);
    cubeModel.GetMaterialByIndex(0).SetCurrent(
        flags, CCubeSurface(cubeModel.GetModelInstance().Surfaces()[0]), cubeModel);
    gpRender->SetModelMatrix(CTransform4f::Identity());
    for (TBoidVector::const_iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
      if (it->mActive) {
        const float scale = it->mScale;
        const CTransform4f xf = CTransform4f::LookAt(it->mPos, it->mPos + it->mVel);
        gpRender->SetModelMatrix(xf * CTransform4f::Scale(scale));
        model.DolphinDrawFlat(CModel::kDF_All);
      }
    }
  }
  if (alpha != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

void CFishCloud::KillBoid(CBoid& boid) {
  boid.mActive = false;
  AddParticles(boid.mPos);
  const int areaId = GetCurrentAreaId().Value();
  CAudioSys::C3DEmitterParmData parms(250.f, 0.1f, 1, 127, 20);
  parms.mPos = boid.mPos;
  parms.mDir = CVector3f::Up();
  parms.mSfxId = mDeathSfx;
  CSfxManager::AddEmitter(parms, areaId, true, false);
}

void CFishCloud::Touch(CActor& other, CStateManager& mgr) {
  CActor::Touch(other, mgr);
  if (CWeapon* weapon = TCastToPtr< CWeapon >(other)) {
    if (!mEnableWeaponRepelDamping && mRepelFromThreats) {
      int index = 0;
      for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it, ++index) {
        if ((index & 3) == (mThinkCounter & 3)) {
          ApplyRepulsion(*it, weapon->GetTranslation(), 8.f,
                         mWeaponRepelMagnitude - mWeaponRepelDamping);
        }
      }
    }
    mEnableWeaponRepelDamping = true;
    if (mKillable) {
      const rstl::optional_object< CAABox > touchBounds = weapon->GetTouchBounds();
      if (touchBounds.valid()) {
        const CAABox bounds = *touchBounds;
        const float radius = mWeaponKillRadius;
        for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
          if (it->mActive) {
            const CVector3f extent(radius, radius, radius);
            const CAABox boidBounds(it->mPos - extent, it->mPos + extent);
            if (bounds.DoBoundsOverlap(boidBounds)) {
              KillBoid(*it);
            }
          }
        }
      }
    }
  }
  if (mRepelFromThreats) {
    if (CPlayer* player = TCastToPtr< CPlayer >(other)) {
      const CPlayer::EPlayerMorphBallState state =
          player->GetSpawnedMorphballState() == CPlayer::kMS_Unmorphed
              ? player->GetMorphballTransitionState()
              : CPlayer::kMS_Unmorphed;
      if (state == CPlayer::kMS_Morphed && !close_enough(mPlayerBallPriority, 0.f)) {
        const float ballRadius = 2.f * player->GetMorphBall()->GetBallRadius() + 0.25f;
        const float ballRadiusSquared = ballRadius * ballRadius;
        const CVector3f ballPos =
            player->GetTranslation() + CVector3f(mgr.Random()->Range(-0.1f, 0.1f), 0.f, 0.f);
        const float distanceSquared = mPlayerBallDistance * mPlayerBallDistance;
        int index = 0;
        for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it, ++index) {
          const CVector3f delta = ballPos - it->GetTranslation();
          const float magSquared = delta.MagSquared();
          if (magSquared > 0.8f + ballRadiusSquared) {
            if ((index & mUpdateMask) != (mThinkCounter & mUpdateMask) &&
                magSquared < distanceSquared) {
              const float priority = mPlayerBallPriority;
              const float weight = 1.f - magSquared / distanceSquared;
              it->mVel += priority * (weight * delta.AsNormalized());
            }
          } else if (magSquared < ballRadiusSquared) {
            if (delta.GetY() > 0.f) {
              const CVector3f flat(delta.GetX(), 0.f, delta.GetZ());
              CVector3f& vel = it->mVel;
              if (flat.CanBeNormalized() && vel.CanBeNormalized()) {
                const float mag = flat.Magnitude();
                const CVector3f velocity = vel.AsNormalized();
                const CVector3f normal = VecDiv(-flat, mag);
                const float dot = CVector3f::Dot(velocity, normal);
                vel += 0.25f * (velocity - 2.f * (dot * normal));
                it->mPos = it->mPos + (0.1f + (ballRadius - mag)) * normal;
              }
            } else {
              CVector3f& vel = it->mVel;
              if (delta.CanBeNormalized() && vel.CanBeNormalized()) {
                const float mag = delta.Magnitude();
                const CVector3f velocity = vel.AsNormalized();
                const CVector3f normal = VecDiv(-delta, mag);
                const float dot = CVector3f::Dot(velocity, normal);
                vel += 0.25f * (velocity - 2.f * (dot * normal));
                it->mPos = it->mPos + (0.1f + (ballRadius - mag)) * normal;
              }
            }
          }
        }
      } else {
        CRandom16& random = *mgr.Random();
        const CVector3f playerPos = player->GetTranslation();
        for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
          CVector3f adjustedPos = playerPos;
          const CVector3f diff = it->GetTranslation() - adjustedPos;
          const float dz = diff.GetZ();
          if (dz > 0.f && dz < 2.3) {
            adjustedPos.SetZ(it->GetTranslation().GetZ());
          }
          adjustedPos[kDX] += 0.2f * random.Float() - 0.1f;
          adjustedPos[kDY] += 0.2f * random.Float() - 0.1f;
          ApplyRepulsion(*it, adjustedPos, 8.f, mPlayerRepelMagnitude - mPlayerRepelDamping);
        }
        mEnablePlayerRepelDamping = true;
      }
    }
  }
}

void CFishCloud::CreatePartitionList() {
  const CAABox bounds = GetBoundingBox();
  mBoidPartitionLists.reserve(343);
}

void CFishCloud::UpdatePartitionList() {
  mBoidPartitionLists.clear();
  for (int i = 0; i < mBoidPartitionLists.capacity(); ++i) {
    mBoidPartitionLists.push_back_unsafe(nullptr);
  }
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  for (TBoidVector::iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    const CVector3f indices =
        CVector3f::ByElementMultiply(mOoPartitionPitch, it->GetTranslation() - min);
    const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
    if (index >= 0 && index < 343) {
      it->mNext = mBoidPartitionLists[index];
      mBoidPartitionLists[index] = &*it;
    }
  }
}

CFishCloud::CBoid* CFishCloud::GetListAt(const CVector3f& pos) {
  const CAABox bounds = GetBoundingBox();
  const CVector3f indices =
      CVector3f::ByElementMultiply(mOoPartitionPitch, pos - bounds.GetMinPoint());
  const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
  if (index < 0 || index >= 343) {
    return nullptr;
  }
  return mBoidPartitionLists[index];
}

void CFishCloud::BuildBoidNearList(const CVector3f& pos, float radius,
                                   rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  const CAABox bounds = GetBoundingBox();
  const CVector3f& min = bounds.GetMinPoint();
  const CVector3f& max = bounds.GetMaxPoint();
  const float x = rstl::max_val(radius * mOoPartitionPitch.GetX(), mPartitionPitch.GetX());
  const float y = rstl::max_val(radius * mOoPartitionPitch.GetY(), mPartitionPitch.GetY());
  const float z = rstl::max_val(radius * mOoPartitionPitch.GetZ(), mPartitionPitch.GetZ());
  int remaining = 25;
  for (float ox = 0.01f - x; ox < x; ox += mPartitionPitch.GetX()) {
    const float px = ox + pos.GetX();
    if (px < min.GetX()) {
      continue;
    }
    if (px >= max.GetX()) {
      break;
    }
    for (float oy = 0.01f - y; oy < y; oy += mPartitionPitch.GetY()) {
      const float py = oy + pos.GetY();
      if (py < min.GetY()) {
        continue;
      }
      if (py >= max.GetY()) {
        break;
      }
      for (float oz = 0.01f - z; oz < z; oz += mPartitionPitch.GetZ()) {
        const float pz = oz + pos.GetZ();
        if (pz < min.GetZ()) {
          continue;
        }
        if (pz >= max.GetZ()) {
          break;
        }
        const CVector3f indices =
            CVector3f::ByElementMultiply(mOoPartitionPitch, CVector3f(px, py, pz) - min);
        const int index = int(indices.GetX()) + int(indices.GetY()) * 7 + int(indices.GetZ()) * 49;
        if (index < 0) {
          continue;
        }
        if (index >= 343) {
          break;
        }
        for (CBoid* boid = mBoidPartitionLists[index]; boid != nullptr; boid = boid->mNext) {
          if (boid->mActive) {
            const CVector3f delta = boid->mPos - pos;
            const float distanceSquared = delta.MagSquared();
            if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
              nearList.push_back(boid);
              if (--remaining == 0) {
                return;
              }
            }
          }
        }
      }
    }
  }
}

void CFishCloud::OldBuildBoidNearList(const CVector3f& pos, float radius,
                                      rstl::reserved_vector< CBoid*, 25 >& nearList) {
  const float radiusSquared = radius * radius;
  int remaining = 25;
  for (CBoid* boid = GetListAt(pos); boid != nullptr && remaining != 0; boid = boid->mNext) {
    if (boid->mActive) {
      const CVector3f delta = boid->GetTranslation() - pos;
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared != 0.f && distanceSquared < radiusSquared) {
        nearList.push_back(boid);
        --remaining;
      }
    }
  }
}

static CVector3f FishCloudCross(const CVector3f& lhs, const CVector3f& rhs) {
  const float lX = lhs.GetX();
  const float lY = lhs.GetY();
  const float lZ = lhs.GetZ();
  const float rX = rhs.GetX();
  const float rY = rhs.GetY();
  const float rZ = rhs.GetZ();
  return CVector3f(lY * rZ - rY * lZ, lZ * rX - rZ * lX, lX * rY - rX * lY);
}

void CFishCloud::ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                               bool clockwise) {
  CVector3f delta = boid.mPos - point;
  delta[kDZ] = 0.f;
  const float distance = delta.Magnitude();
  const CVector3f align = clockwise ? FishCloudCross(delta.AsNormalized(), CVector3f::Up())
                                    : FishCloudCross(CVector3f::Up(), VecDiv(delta, distance));
  const CVector3f velocity = boid.mVel;
  const float weight = distance > radius ? 0.f : 1.f - distance / radius;
  const float angle = CVector3f::GetAngleDiff(velocity, align) / M_PIF;
  const float weightedAngle = angle * weight;
  boid.mVel += weightedAngle * (magnitude * align);
}

void CFishCloud::ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (rstl::reserved_vector< CBoid*, 25 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      average += (*it)->mVel;
    }
    average = VecDiv(average, float(nearList.size()));
    const CVector3f velocity = boid.mVel;
    const float angle = CVector3f::GetAngleDiff(velocity, average) / M_PIF;
    boid.mVel += angle * (mAlignmentWeight * average);
  }
}

void CFishCloud::ApplyWander(CStateManager& mgr, CBoid& boid) {
  const float x = boid.mVel.GetX();
  const float y = boid.mVel.GetY();
  const float angle = mMaxScatterAngle * (M_PIF * (mgr.Random()->Float() - 0.5f));
  const CVector3f scatter(x * CMath::FastCosR(angle) - y * CMath::FastSinR(angle),
                          x * CMath::FastSinR(angle) + y * CMath::FastCosR(angle), 0.f);
  boid.mVel += mScatterVel * scatter;
}

void CFishCloud::ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f average(0.f, 0.f, 0.f);
    for (rstl::reserved_vector< CBoid*, 25 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      average += (*it)->GetTranslation();
    }
    average = VecDiv(average, float(nearList.size()));
    ApplyCohesion(boid, average, mSeparationRadius, mCohesionMagnitude);
  }
}

void CFishCloud::ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float weight = distanceSquared > radius ? 1.f : distanceSquared / radius;
    boid.mVel += magnitude * (weight * delta.AsNormalized());
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList) {
  if (nearList.size() > 0) {
    CVector3f nearest(0.f, 0.f, 0.f);
    float minDistanceSquared = FLT_MAX;
    for (rstl::reserved_vector< CBoid*, 25 >::const_iterator it = nearList.begin();
         it != nearList.end(); ++it) {
      const CVector3f delta = boid.GetTranslation() - (*it)->GetTranslation();
      const float distanceSquared = delta.MagSquared();
      if (distanceSquared < minDistanceSquared) {
        minDistanceSquared = distanceSquared;
        nearest = (*it)->GetTranslation();
      }
    }
    ApplySeparation(boid, nearest, mSeparationRadius, mSeparationMagnitude);
  }
}

void CFishCloud::ApplySeparation(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = boid.GetTranslation() - point;
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.mVel += magnitude * (weight * delta.AsNormalized());
    }
  }
}

void CFishCloud::ApplyAttraction(CBoid& boid, const CVector3f& point, float radius,
                                 float magnitude) {
  const CVector3f delta = point - boid.GetTranslation();
  if (delta.CanBeNormalized()) {
    const float distanceSquared = delta.MagSquared();
    const float radiusSquared = radius * radius;
    if (distanceSquared < radiusSquared) {
      const float weight = 1.f - distanceSquared / radiusSquared;
      boid.mVel += magnitude * (weight * delta.AsNormalized());
    }
  }
}

void CFishCloud::ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius,
                                float magnitude) {
  ApplySeparation(boid, point, radius, magnitude);
}

void CFishCloud::ApplyContainment(CBoid& boid, const CAABox& aabb) {
  const float radius = mContainmentRadius;
  if (boid.mVel.CanBeNormalized()) {
    const CVector3f futurePos = boid.mPos + radius * (mSpeed * boid.mVel.AsNormalized());
    if (!PointInBox(aabb, futurePos)) {
      ApplyAttraction(boid, aabb.GetCenterPoint(), 100000.f, mContainmentMagnitude);
    }
  }
}

void CFishCloud::AddParticles(const CVector3f& pos) {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->SetParticleEmission(true);
    mParticleGens[i]->SetTranslation(pos);
    mParticleGens[i]->ForceParticleCreation(mDeathParticleCounts[i]);
    mParticleGens[i]->SetParticleEmission(false);
  }
}

void CFishCloud::UpdateParticles(float dt) {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    mParticleGens[i]->Update(dt);
  }
}

void CFishCloud::RenderParticles() const {
  for (int i = 0; i < mParticleGens.size(); ++i) {
    gpRender->AddParticleGen(*mParticleGens[i]);
  }
}

CEntity* LoadFishCloud(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFishCloud sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFishCloud.inc"

  CActorParameters actParms = CActorParameters::None();
  actParms = actParms.HotInThermal(sldrThis.isHighlightedInDarkVisor);
  return new CFishCloud(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.editorProperties.transform.scale,
      LdrToTransform4f(sldrThis.editorProperties),
      CModelData(CStaticRes(sldrThis.fishModel, CVector3f(1.f, 1.f, 1.f))),
      CAnimRes(sldrThis.animationInformation.ancs, sldrThis.animationInformation.character_index,
               CVector3f(1.f, 1.f, 1.f), 0, true),
      CCast::FtoL(sldrThis.fishCount), sldrThis.speed, sldrThis.influenceDistance,
      sldrThis.cohesionPriority, sldrThis.alignmentPriority, sldrThis.separationPriority,
      sldrThis.projectilePriority, sldrThis.playerPriority, sldrThis.containmentPriority,
      sldrThis.wanderPriority, sldrThis.wanderAmount, sldrThis.projectileDecayRate,
      sldrThis.playerDecayRate, sldrThis.playerBallPriority, sldrThis.playerBallDistance,
      sldrThis.lookAheadTime, sldrThis.updateFrame, sldrThis.materialColor, sldrThis.canBeKilled,
      sldrThis.collisionRadius, sldrThis.deathEffect0, sldrThis.deathEffect0Count,
      sldrThis.deathEffect1, sldrThis.deathEffect1Count, sldrThis.deathEffect2,
      sldrThis.deathEffect2Count, sldrThis.deathEffect3, sldrThis.deathEffect3Count,
      sldrThis.deathSound, sldrThis.unknown_0xc320a050, actParms);
}

CEntity* LoadFishCloudModifier(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrFishCloudModifier sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrFishCloudModifier.inc"

  return new CFishCloudModifier(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                LdrToEntityInfo(info, sldrThis.editorProperties),
                                sldrThis.editorProperties.transform.position,
                                sldrThis.unknown_0xea2d4ca8, sldrThis.rotate,
                                sldrThis.influenceDistance, sldrThis.influencePriority);
}

#ifndef MONOLITHIC
SFishCloud_FuncPtrs REL_loader_FishCloud;

void SetRelLoaderFunctionToLoader() {
  REL_loader_FishCloud.mLoadFishCloud = LoadFishCloud;
  REL_loader_FishCloud.mLoadFishCloudModifier = LoadFishCloudModifier;
  SetSFishCloud_FuncPtrs(&REL_loader_FishCloud);
}

extern "C" void RELMain() { SetRelLoaderFunctionToLoader(); }

extern "C" void RELExit() { SetSFishCloud_FuncPtrs(nullptr); }
#endif
