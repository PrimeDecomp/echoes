#include "MetroidPrime/Enemies/CSurfaceParticleEffect.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTri.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/COBBTree.hpp"

CSurfaceParticleEffect::CSurfaceParticleEffect(const TLockedToken< CGenDescription >& desc,
                                               TUniqueId uid, TAreaId area, bool active,
                                               const rstl::string& name, const CTransform4f& xf,
                                               TUniqueId ignoredCollisionId, int flags)
: CEffect(uid, CEntityInfo(area, CEntity::NullConnectionList, active, kInvalidEditorId), name, xf)
, mRandom(CStopwatch::GetGlobalMicros())
, mParticleGen(rs_new CElementGen(desc, CElementGen::kMOT_One, CElementGen::kOSF_One))
, mLightId(kInvalidUniqueId)
, mParticleAssetId(CToken(desc).GetTag().GetId())
, mGeneratorRate(1.f)
, mSpawnRemainder(0.f)
, mFrameCounter(0)
, mIgnoredCollisionId(ignoredCollisionId)
, mHasRenderBounds(false)
, mFlag(false) {
  SetHighlightedInDarkVisor(true);
}

CSurfaceParticleEffect::~CSurfaceParticleEffect() {}

// Guessed name: per-particle record stored in the particle's additional (ADV) data.
static const uint kNoSurface = 0xFFFFFFFF; // Guessed name

struct SParticleSurfaceState {
  uint mSurfaceId; // kNoSurface when the particle is not attached to a surface
  CQuaternion mCurrentOrientation;
  CQuaternion mTargetOrientation;
};

static EMaterialTypes sObbTreeExcludeMaterial = kMT_ProjectilePassthrough;
static EMaterialTypes sNearListExcludeMaterial0 = kMT_Character;
static EMaterialTypes sNearListExcludeMaterial1 = kMT_Player;
static EMaterialTypes sNearListExcludeMaterial2 = kMT_Projectile;
static EMaterialTypes sNearListExcludeMaterial3 = kMT_ProjectilePassthrough;
static EMaterialTypes sNearListExcludeMaterial4 = kMT_AIJoint;

static CVector3f ProjectOntoPlane(const CVector3f& vector, float normalDot, float magnitude,
                                  const CVector3f& normal) {
  CVector3f result = vector - normal * normalDot;
  result = result.AsNormalized();
  result = result * magnitude;
  return result;
}

// Guessed name: unreferenced in the retail REL, but kept as a global symbol.
uint FindNearestSurfaceAlongDirection(
    const rstl::reserved_vector< CCollisionSurface, 256 >& surfaces, const CVector3f& point,
    const CVector3f& direction) {
  float best = FLT_MAX;
  uint bestIndex = kNoSurface;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCollisionSurface& surface = surfaces[i];
    const float sqrDist = CollisionUtil::TriPointSqrDist_Float(
        point, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2), nullptr, nullptr);
    if (sqrDist < best) {
      const float score =
          sqrDist * (1.f / CMath::AbsF(CVector3f::Dot(direction, surface.GetNormal())));
      if (score < best) {
        best = score;
        bestIndex = i;
        if (score < FLT_EPSILON) {
          break;
        }
      }
    }
  }
  return bestIndex;
}

static uint FindNearestSurface(const rstl::reserved_vector< CCollisionSurface, 256 >& surfaces,
                               const CVector3f& point) {
  float best = FLT_MAX;
  uint bestIndex = kNoSurface;
  for (int i = 0; i < surfaces.size(); ++i) {
    const CCollisionSurface& surface = surfaces[i];
    const float sqrDist = CollisionUtil::TriPointSqrDist_Float(
        point, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2), nullptr, nullptr);
    if (sqrDist < best) {
      best = sqrDist;
      bestIndex = i;
      if (best < FLT_EPSILON) {
        break;
      }
    }
  }
  return bestIndex;
}

static void BuildOrientationFromNormal(const CVector3f& normal, CMatrix3f& orientation) {
  float dot = CMath::Limit(CVector3f::Dot(CVector3f::Right(), normal), 1.f);
  CVector3f tangent = CVector3f::Right() - normal * dot;
  if (tangent.Magnitude() <= FLT_EPSILON) {
    dot = CMath::Limit(CVector3f::Dot(CVector3f::Up(), normal), 1.f);
    tangent = CVector3f::Up() - normal * dot;
  }
  tangent = tangent * (1.f / tangent.Magnitude());
  const CVector3f cross = CVector3f::Cross(normal, tangent);
  orientation = CMatrix3f(cross.GetX(), normal.GetX(), tangent.GetX(), cross.GetY(), normal.GetY(),
                          tangent.GetY(), cross.GetZ(), normal.GetZ(), tangent.GetZ());
}

static void GatherFromBox(rstl::reserved_vector< CCollisionSurface, 256 >& surfaces,
                          rstl::reserved_vector< uint, 256 >& surfaceIds, const CAABox& box,
                          const CVector3f& center, const CVector3f& halfExtent, uint idBits) {
  for (int i = 0; i < 12; ++i) {
    const CTri tri = box.GetTri(static_cast< CAABox::EBoxFaceId >(i / 2), (i & 1) * 2);
    const CCollisionSurface surface(tri.GetPointA(), tri.GetPointB(), tri.GetPointC(), -1);
    if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2))) {
      if (surfaces.size() == surfaces.capacity()) {
        break;
      }
      surfaces.push_back(surface);
      surfaceIds.push_back(idBits | i);
    }
  }
}

static void GatherFromObbTree(rstl::reserved_vector< CCollisionSurface, 256 >& surfaces,
                              rstl::reserved_vector< uint, 256 >& surfaceIds, const COBBTree& tree,
                              const CTransform4f& xf, const CVector3f& center,
                              const CVector3f& halfExtent, uint idBits) {
  const CMaterialFilter filter =
      CMaterialFilter::MakeExclude(CMaterialList(sObbTreeExcludeMaterial));
  const int triangleCount = tree.GetTriangleCount();
  for (short i = 0; i < triangleCount; ++i) {
    const CCollisionSurface surface(tree.GetTriangle(i, &xf));
    if (filter.Passes(CMaterialList(surface.GetSurfaceFlags())) &&
        CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2))) {
      if (surfaces.size() == surfaces.capacity()) {
        break;
      }
      surfaces.push_back(surface);
      surfaceIds.push_back(idBits | i);
    }
  }
}

void CSurfaceParticleEffect::GatherCollisionSurfaces(
    CStateManager& mgr, const CAABox& bounds,
    rstl::reserved_vector< CCollisionSurface, 256 >& surfaces,
    rstl::reserved_vector< uint, 256 >& surfaceIds) {
  CAreaCollisionCache cache(bounds);
  CGameCollision::BuildAreaCollisionCache(mgr, cache);
  const CVector3f center = bounds.GetCenterPoint();
  const CVector3f halfExtent((bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX()) * 0.5f,
                             (bounds.GetMaxPoint().GetY() - bounds.GetMinPoint().GetY()) * 0.5f,
                             (bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ()) * 0.5f);

  CMetroidAreaCollider::ResetInternalCounters();
  for (int j = 0; j < static_cast< int >(cache.GetNumCaches()); ++j) {
    const CMetroidAreaCollider::COctreeLeafCache& leafCache = cache.GetOctreeLeafCache(j);
    const uint areaBits = leafCache.GetAreaId().value << 16;
    for (int n = 0; n < leafCache.GetNumLeaves(); ++n) {
      const CAreaOctTree::Node& leaf = leafCache.GetLeaf(n);
      const CAreaOctTree::TriListReference triangles = leaf.GetTriangleArray();
      const CAreaOctTree& owner = leaf.GetOwner();
      const int triangleCount = triangles.GetSize();
      for (int i = 0; i < triangleCount; ++i) {
        const int index = triangles.GetAt(i);
        if (CMetroidAreaCollider::DupTriangleListValue(index) !=
            CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
          CMetroidAreaCollider::DupTriangleListValue(index) =
              CMetroidAreaCollider::GetDupPrimitiveCheckCount();
          const CCollisionSurface surface(owner.GetTriangle(index));
          if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0),
                                           surface.GetVert(1), surface.GetVert(2))) {
            if (surfaces.size() == surfaces.capacity()) {
              n = leafCache.GetNumLeaves();
              break;
            }
            surfaces.push_back(surface);
            surfaceIds.push_back(areaBits | index);
          }
        }
      }
    }
  }

  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds,
                    CMaterialFilter::MakeExclude(
                        CMaterialList(sNearListExcludeMaterial0, sNearListExcludeMaterial1,
                                      sNearListExcludeMaterial2, sNearListExcludeMaterial3,
                                      sNearListExcludeMaterial4)),
                    this);
  for (rstl::reserved_vector< TUniqueId, 1024 >::const_iterator it = nearList.begin();
       it != nearList.end(); ++it) {
    if (*it == mIgnoredCollisionId) {
      continue;
    }
    CActor* actor = static_cast< CActor* >(mgr.ObjectById(*it));
    if (mFlag && TCastToPtr< CScriptPlatform >(actor) == nullptr) {
      continue;
    }
    const uint idBits = it->value << 22;
    CPhysicsActor* physActor = TCastToPtr< CPhysicsActor >(actor);
    if (physActor != nullptr && physActor->GetCollisionPrimitive()->GetPrimType() == 'OBTG') {
      const CCollidableOBBTreeGroup* group =
          static_cast< const CCollidableOBBTreeGroup* >(physActor->GetCollisionPrimitive());
      for (int i = 0; i < group->GetContainer()->NumTrees(); ++i) {
        GatherFromObbTree(surfaces, surfaceIds, *group->GetOBBTree(i),
                          physActor->GetPrimitiveTransform(), center, halfExtent, idBits);
      }
    } else if (actor != nullptr) {
      if (actor->GetMaterialList().HasMaterial(kMT_Solid) ||
          TCastToPtr< CScriptWater >(actor) != nullptr) {
        const rstl::optional_object< CAABox > touchBounds = actor->GetTouchBounds();
        if (touchBounds) {
          GatherFromBox(surfaces, surfaceIds, *touchBounds, center, halfExtent, idBits);
        }
      }
    }
  }
}

void CSurfaceParticleEffect::Think(float dt, CStateManager& mgr) {
  mParticleGen->SetGeneratorRate(mGeneratorRate);
  mSpawnRemainder += mParticleGen->GetGenerationRate();
  const int wanted = static_cast< int >(CMath::FloorF(mSpawnRemainder));
  const int existing = mParticleGen->GetParticleCount();
  int spawnCount = mParticleGen->mMAXP - existing;
  if (wanted < spawnCount) {
    spawnCount = wanted;
  }
  if (existing == 0 && spawnCount == 0) {
    return;
  }
  mSpawnRemainder -= static_cast< float >(spawnCount);
  mParticleGen->SetGeneratorRate(0.f);
  mParticleGen->SetTranslation(GetTranslation());
  mParticleGen->SetOrientation(CQuaternion::FromMatrix(GetTransform()).BuildTransform4f());
  mParticleGen->ForceParticleCreation(spawnCount);

  CAABox bounds(GetTranslation() - CVector3f(1.f, 1.f, 1.f),
                GetTranslation() + CVector3f(1.f, 1.f, 1.f));
  const rstl::optional_object< CAABox > particleBounds = mParticleGen->GetBounds();
  if (particleBounds) {
    bounds.AccumulateBounds(particleBounds->GetMinPoint());
    bounds.AccumulateBounds(particleBounds->GetMaxPoint());
  }

  rstl::reserved_vector< CCollisionSurface, 256 > surfaces;
  rstl::reserved_vector< uint, 256 > surfaceIds;
  GatherCollisionSurfaces(mgr, bounds, surfaces, surfaceIds);

  if (spawnCount != 0) {
    const int first = mParticleGen->GetParticleCount() - spawnCount;
    CElementGen::CParticle* particle = &mParticleGen->mParticles[first];
    SParticleSurfaceState* state =
        reinterpret_cast< SParticleSurfaceState* >(const_cast< CElementGen::CAdvancedValues* >(
            mParticleGen->GetParticleAdditionalData(first)));
    CMatrix3f* orientation = &mParticleGen->mParentMatrices[first];
    for (int i = 0; i < spawnCount; ++i) {
      const uint slot = FindNearestSurface(surfaces, particle->mPos);
      if (slot != kNoSurface) {
        state->mSurfaceId = surfaceIds[slot];
        const CCollisionSurface& surface = surfaces[slot];
        float baryX, baryY;
        const float distance = CMath::SqrtF(CollisionUtil::TriPointSqrDist_Float(
            particle->mPos, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2), &baryX,
            &baryY));
        const CVector3f point =
            CMath::BaryToWorld(surface.GetVert(0), surface.GetVert(1), surface.GetVert(2),
                               CVector3f(1.f - (baryX + baryY), baryY, baryX));
        const CVector3f normal = surface.GetNormal();
        particle->mPos = point;
        const float normalDot = CVector3f::Dot(particle->mVel, normal);
        particle->mVel =
            ProjectOntoPlane(particle->mVel, normalDot, particle->mVel.Magnitude(), normal);
        BuildOrientationFromNormal(normal, *orientation);
        state->mCurrentOrientation = state->mTargetOrientation =
            CQuaternion::FromMatrix(*orientation);
      } else {
        particle->mEndFrame = -1;
      }
      ++orientation;
      ++particle;
      ++state;
    }
  }

  mParticleGen->Update(dt);

  if (mParticleGen->GetParticleCount() != 0) {
    const int currentFrame = mParticleGen->GetEmitterTime();
    CElementGen::CParticle* particle = &mParticleGen->mParticles[0];
    SParticleSurfaceState* state = reinterpret_cast< SParticleSurfaceState* >(
        const_cast< CElementGen::CAdvancedValues* >(mParticleGen->GetParticleAdditionalData(0)));
    CMatrix3f* orientation = &mParticleGen->mParentMatrices[0];
    for (int i = 0; i < mParticleGen->GetParticleCount(); ++i) {
      const uint phase = particle->mStartFrame & 7;
      if (phase == mFrameCounter) {
        if (!(state->mTargetOrientation == state->mCurrentOrientation)) {
          state->mCurrentOrientation = state->mTargetOrientation;
          *orientation = state->mCurrentOrientation.BuildTransform();
        }
        if (state->mSurfaceId != kNoSurface) {
          const uint slot = FindNearestSurface(surfaces, particle->mPos);
          if (slot != kNoSurface) {
            if (surfaceIds[slot] != state->mSurfaceId) {
              state->mSurfaceId = surfaceIds[slot];
              const CCollisionSurface& surface = surfaces[slot];
              float baryX, baryY;
              const float distance = CMath::SqrtF(CollisionUtil::TriPointSqrDist_Float(
                  particle->mPos, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2),
                  &baryX, &baryY));
              const CVector3f point =
                  CMath::BaryToWorld(surface.GetVert(0), surface.GetVert(1), surface.GetVert(2),
                                     CVector3f(1.f - (baryX + baryY), baryY, baryX));
              const CVector3f normal = surface.GetNormal();
              particle->mPos = point;
              const float normalDot = CVector3f::Dot(particle->mVel, normal);
              const float speed = particle->mVel.Magnitude();
              const CVector3f previousNormal = orientation->GetColumn(kDY);
              const float previousDot = CVector3f::Dot(normal, previousNormal);
              if (CMath::AbsF(normalDot / speed) > 0.95f || previousDot < 0.3f) {
                particle->mVel = CVector3f::Zero();
                state->mSurfaceId = kNoSurface;
              } else {
                particle->mVel = ProjectOntoPlane(particle->mVel, normalDot, speed, normal);
                if (CVector3f::Dot(normal, previousNormal) < 0.995f) {
                  CMatrix3f matrix = CMatrix3f::Identity();
                  BuildOrientationFromNormal(normal, matrix);
                  state->mTargetOrientation = CQuaternion::FromMatrix(matrix);
                }
              }
            }
          } else {
            state->mSurfaceId = kNoSurface;
          }
        }
      } else if (!(state->mTargetOrientation == state->mCurrentOrientation)) {
        const float t = 1.f - 0.125f * static_cast< float >((phase + 8 - mFrameCounter) & 7);
        *orientation = CQuaternion::Slerp(state->mCurrentOrientation, state->mTargetOrientation, t)
                           .BuildTransform();
      }

      if (state->mSurfaceId == kNoSurface && particle->mEndFrame != -1) {
        const int fadeFrames[5] = {11, 11, 11, 5, 0};
        const int index = ((currentFrame - particle->mStartFrame - 1) * 4) /
                          (particle->mEndFrame - particle->mStartFrame);
        const int newEnd = particle->mEndFrame - fadeFrames[index];
        if (newEnd < currentFrame) {
          particle->mEndFrame = -1;
        } else {
          particle->mEndFrame = newEnd;
        }
      }
      ++orientation;
      ++particle;
      ++state;
    }
  }

  ++mFrameCounter;
  mFrameCounter &= 7;
  mParticleGen->SetOrientation(CTransform4f::Identity());

  if (mLightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId))) {
      if (GetActive()) {
        light->SetLight(mParticleGen->GetLight());
      }
    }
  }
}

void CSurfaceParticleEffect::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetSenderId();

  switch (message) {
  case kSM_Create:
    if (mParticleGen->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(), rstl::string_l(""),
                                      GetTransform(), GetUniqueId(), mParticleGen->GetLight(),
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

void CSurfaceParticleEffect::PreRender(CStateManager& mgr) {
  CActor::PreRender(mgr);
  SetPreRenderClipped(!mHasRenderBounds || GetPreRenderClipped());
}

void CSurfaceParticleEffect::AddToRenderer(const CStateManager& mgr) const {
  if (mHasRenderBounds) {
    Render(mgr);
  }
}

void CSurfaceParticleEffect::Render(const CStateManager& mgr) const {
  int alpha = GetRenderAlphaBufferAlpha(mgr);
  if (alpha != -1 && mgr.GetCurrentRenderPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Dark) {
    gpRender->SetDestinationAlpha(alpha);
  } else {
    alpha = -1;
  }
  mParticleGen->Render();
  if (alpha != -1) {
    gpRender->DisableDestinationAlpha();
  }
}

void CSurfaceParticleEffect::PreRenderAllViewports(CStateManager& mgr) {
  rstl::optional_object< CAABox > bounds = mParticleGen->GetBounds();
  if (bounds.valid()) {
    mHasRenderBounds = true;
    SetOtherBounds(*bounds);
    SetRenderBounds(*bounds);
  } else {
    mHasRenderBounds = false;
    CAABox box(GetTranslation(), GetTranslation());
    SetOtherBounds(box);
    SetRenderBounds(box);
  }
  UpdatePortalSystemState(mgr);
}

rstl::optional_object< CAABox > CSurfaceParticleEffect::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CSurfaceParticleEffect::Touch(CActor&, CStateManager&) {}

CElementGen* CSurfaceParticleEffect::GetParticleGen() { return mParticleGen.get(); }

const CElementGen* CSurfaceParticleEffect::GetParticleGen() const { return mParticleGen.get(); }

float CSurfaceParticleEffect::GetGeneratorRate() const { return mGeneratorRate; }

void CSurfaceParticleEffect::SetFlag(bool value) { mFlag = value; }

void CSurfaceParticleEffect::SetGeneratorRate(float rate) { mGeneratorRate = rate; }

static CEffect* CreateSurfaceParticleEffect(const TLockedToken< CGenDescription >& desc,
                                            TUniqueId uid, TAreaId area, bool active,
                                            const rstl::string& name, const CTransform4f& xf,
                                            TUniqueId ignoredCollisionId, uint flags) {
  return rs_new CSurfaceParticleEffect(desc, uid, area, active, name, xf, ignoredCollisionId,
                                       flags);
}

void SetSurfaceParticleEffectFuncPtrs() {
  static SSurfaceParticleEffect_FuncPtrs funcPtrs;
  funcPtrs.mFactory = &CreateSurfaceParticleEffect;
  SetSSurfaceParticleEffect_FuncPtrs(&funcPtrs);
}

void ClearSurfaceParticleEffectFuncPtrs() { SetSSurfaceParticleEffect_FuncPtrs(nullptr); }

// ---- Raw matching-decompiler output from a local tree (reference only, not cleaned up) ----

extern "C" int fn_25_5614(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

// ---- End of raw matching-decompiler output ----
