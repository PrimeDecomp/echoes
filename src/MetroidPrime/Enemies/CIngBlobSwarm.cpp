#include "MetroidPrime/Enemies/CIngBlobSwarm.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrIngBlobSwarm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/SwarmRenderHelpers.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

CIngBlobSwarm::CIngBlobSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CVector3f& boundingBoxExtent, const CTransform4f& xf,
                             const CAnimRes& animRes, CActorParameters actorParameters,
                             const CBasicSwarmData& data, int intoAttackAnimation,
                             int attackAnimation, float maxAttackAngle, float intoAttackSpeed,
                             float attackSpeed, float mass, float maxAttackHeight,
                             const CVector3f& attackAimOffset)
: CSwarmBasics(uid, name, info, boundingBoxExtent, xf, animRes, actorParameters, data, true)
, mAttackerCount(data.mAttackerCount)
, mAttackProximity(data.mAttackProximity)
, mAttackTimer(data.mAttackTimer)
, mMaxAttackAngle(maxAttackAngle * (M_PIF / 180.f))
, mSharedAttackBit(0)
, mAttackModelBitBase(0)
, mIntoAttackSpeed(intoAttackSpeed)
, mAttackSpeed(attackSpeed)
, mMass(mass)
, mMaxAttackHeight(maxAttackHeight)
, mAttackAimOffset(attackAimOffset)
, mSharedAttackActive(false) {
  if (animRes.GetId() != kInvalidAssetId && intoAttackAnimation != -1) {
    mAttackModels.reserve(mAttackerCount);
    mAttackDeltas.reserve(mAttackerCount);
    mAttackSlotInUse.reserve(mAttackerCount);
    const CAnimRes intoAttackRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                                 intoAttackAnimation, true);
    mAttackModelBitBase = mModelDatas.size();
    for (int i = 0; i < mAttackerCount; ++i) {
      mAttackModels.push_back_unsafe(CModelData(intoAttackRes));
      mAttackDeltas.push_back_unsafe(
          CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
      mAttackModels[i].AnimationData()->EnableLooping(false);
      mAttackModels[i].AnimationData()->SetIsAnimating(true);
      mAttackSlotInUse.push_back_unsafe(false);
    }
    if (attackAnimation != -1) {
      const CAnimRes attackRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                               attackAnimation, true);
      mSharedAttackModel = rs_new CModelData(attackRes);
      mSharedAttackModel->AnimationData()->SetIsAnimating(true);
      mSharedAttackModel->AnimationData()->EnableLooping(true);
      mSharedAttackDeltas = rs_new CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation());
      mSharedAttackBit = mAttackModelBitBase + mAttackerCount;
    }
  }
}

CIngBlobSwarm::~CIngBlobSwarm() {}

void CIngBlobSwarm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CSwarmBasics::AcceptScriptMsg(mgr, msg);
}

void CIngBlobSwarm::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mSharedAttackActive = false;
  CSwarmBasics::Think(dt, mgr);
  if (mAttackerCount == 0) {
    return;
  }
  static CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
  uint attackers = 0;
  for (rstl::vector< CBoid >::const_iterator it = mBoids.begin(); it != mBoids.end(); ++it) {
    if (it->mActive && it->mAttacking) {
      ++attackers;
    }
  }
  mTimeSinceLastAttack = rstl::min_val(mTimeSinceLastAttack + dt, mAttackTimer);
  if (attackers < mAttackerCount) {
    for (rstl::vector< CBoid >::iterator it = mBoids.begin();
         it != mBoids.end() && attackers < mAttackerCount; ++it) {
      if (!it->mActive || it->mAttacking) {
        continue;
      }
      const CVector3f toPlayer = playerPos - it->GetTranslation();
      if (toPlayer.MagSquared() < mAttackProximity * mAttackProximity &&
          mTimeSinceLastAttack >= mAttackTimer) {
        const CVector3f forward = it->GetTransform().GetForward();
        if (CMath::AbsF(CVector3f::GetAngleDiff(forward, toPlayer.AsNormalized())) <=
            mMaxAttackAngle) {
          for (uint i = 0; i < mAttackerCount; ++i) {
            if (!mAttackSlotInUse[i]) {
              it->mAttacking = true;
              it->mLaunched = true;
              it->mAttackSlot = i;
              mAttackSlotInUse[i] = true;
              mAttackModels[i].AnimationData()->SetPhase(0.f);
              mAttackModels[i].AnimationData()->SetIsAnimating(true);
              mTimeSinceLastAttack = 0.f;
              break;
            }
          }
        }
      }
    }
  }
  if (attackers != 0) {
    UpdateLoopedSoundPositions(mAttackSounds);
  }
}

void CIngBlobSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  const uint count = mAttackModels.size();
  mAttackModelStates.reserve(count);
  for (uint i = 0; i < count; ++i) {
    mAttackModelStates.push_back_unsafe(
        SwarmRenderHelpers::CSwarmSkinnedModelState(mAttackModels[i].PickAnimatedModel(which)));
  }
  if (mSharedAttackModel.get() != nullptr) {
    CSkinnedModel& skinnedModel = mSharedAttackModel->PickAnimatedModel(which);
    mSharedAttackState = rs_new SwarmRenderHelpers::CSwarmSkinnedModelState(skinnedModel);
  }
  CSwarmBasics::AllocateSkinnedModels(mgr, which);
}

void CIngBlobSwarm::UpdateSwarmAnimations(CStateManager& mgr, float dt) {
  CSwarmBasics::UpdateSwarmAnimations(mgr, dt);
  const uint count = mAttackModels.size();
  for (uint i = 0; i < count; ++i) {
    if (mAttackSlotInUse[i]) {
      CAnimData* animData = mAttackModels[i].AnimationData();
      animData->SetPlaybackRate(1.f);
      mAttackDeltas[i] = mAttackModels[i].AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
      UpdateEffects(mgr, *animData, mMaxVolume);
    }
  }
  if (mSharedAttackModel.get() != nullptr && mSharedAttackActive) {
    CAnimData* animData = mSharedAttackModel->AnimationData();
    animData->SetPlaybackRate(mAttackSpeed);
    *mSharedAttackDeltas = mSharedAttackModel->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    UpdateEffects(mgr, *animData, mMaxVolume);
  }
}

void CIngBlobSwarm::UpdateAllBoidMovement(CStateManager& mgr, float dt) {
  const uint count = mBoids.size();
  if (mAnimated) {
    const uint modelCount = mModelDatas.size();
    for (uint i = 0; i < count; ++i) {
      CBoid& boid = mBoids[i];
      if (boid.mAttacking) {
        if (boid.mAttackSlot == -1) {
          boid.mTransform.AddTranslation(boid.mVelocity * dt);
        } else {
          MoveBoid(mgr, boid, mAttackDeltas[boid.mAttackSlot].GetOffsetDelta(), dt);
        }
      } else {
        MoveBoid(mgr, boid, mAdvancementDeltas[i & (modelCount - 1)].GetOffsetDelta(), dt);
      }
    }
  }
}

void CIngBlobSwarm::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt,
                               CBoid& boid, int partitionIndex) {
  boid.mPartitionIndex = partitionIndex;
  if (mVulnerableToSafeZone &&
      mgr.GetSafeZoneManager()->PointIsInSafeZone(mgr, boid.GetTranslation())) {
    KillBoid(boid, mgr, CWeaponMode());
    return;
  }
  UpdateLightComboBeam(boid, mgr);
  if (boid.mAttacking && boid.mAttackSlot != -1) {
    const int slot = boid.mAttackSlot;
    const CVector3f toPlayer = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f) - boid.GetTranslation();
    const CVector3f forward = boid.GetTransform().GetForward();
    boid.mTransform = ShortestRotationArcWrapped(forward, forward + toPlayer.AsNormalized(),
                                                 CRelAngle::FromRadians(M_PIF * dt))
                          .MultiplyIgnoreTranslation(boid.mTransform);
    if (!mAttackModels[slot].AnimationData()->IsAnimTimeRemaining(dt,
                                                                  rstl::string_l("Whole Body"))) {
      mAttackSlotInUse[slot] = false;
      boid.mAttackSlot = -1;
      LaunchBoid(boid, mAttackAimOffset + mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f));
      AddLoopedSoundToHandlesList(boid, mAttackSounds, mAttackLoopedSound);
    }
  } else if (boid.mAttacking && boid.mLaunched) {
    mSharedAttackActive = true;
    const float radius = mBoidRadius;
    const float speed = boid.mVelocity.Magnitude();
    const CVector3f step = radius * (1.f / speed) * -boid.mVelocity;
    float remaining = speed * dt;
    CVector3f pos = boid.GetTranslation();
    bool hit = false;
    while (remaining >= 0.f && !hit) {
      CCollisionSurface surface(CVector3f(1.f, 0.f, 0.f), CVector3f(0.f, 1.f, 0.f),
                                CVector3f(0.f, 0.f, 1.f), u64(-1));
      if (boid.mRemainingLaunchNotOnSurfaceFrames == 0 &&
          FindBestSurface(cache, pos + (mSurfaceProbeScale * dt) * boid.mVelocity, radius,
                          surface)) {
        BoidCollidedCallback(mgr, boid);
        hit = true;
      }
      remaining -= radius;
      pos += step;
    }
    if (!hit) {
      boid.mVelocity += dt * CVector3f(0.f, 0.f, -mMass * kDefaultGravityAccel);
      if (boid.mRemainingLaunchNotOnSurfaceFrames > 0) {
        --boid.mRemainingLaunchNotOnSurfaceFrames;
      }
    }
  } else {
    CSwarmBasics::UpdateBoid(cache, mgr, dt, boid, partitionIndex);
  }
}

void CIngBlobSwarm::PreRenderBoid(CBoid* boid, uint* drawMask) {
  if (boid->mAttacking) {
    const int slot = boid->mAttackSlot;
    if (slot == -1) {
      if (*drawMask & (1 << mSharedAttackBit)) {
        CachePose(*mSharedAttackModel, *mSharedAttackState);
        *drawMask &= ~(1 << mSharedAttackBit);
      }
    } else {
      if (*drawMask & (1 << (mAttackModelBitBase + slot))) {
        CachePose(mAttackModels[slot], mAttackModelStates[slot]);
        *drawMask &= ~(1 << (mAttackModelBitBase + slot));
      }
    }
  } else {
    CSwarmBasics::PreRenderBoid(boid, drawMask);
  }
}

void CIngBlobSwarm::RenderBoid(CBoid* boid) const {
  if (boid->mAttacking) {
    if (boid->mAttackSlot == -1) {
      DrawBoidSkinnedModel(boid, *mSharedAttackState);
    } else {
      DrawBoidSkinnedModel(boid, mAttackModelStates[boid->mAttackSlot]);
    }
  } else {
    CSwarmBasics::RenderBoid(boid);
  }
}

void CIngBlobSwarm::ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                                           const rstl::reserved_vector< CBoid*, 50 >& nearList) {
  if (!boid.mAttacking) {
    CSwarmBasics::ApplySteeringBehaviors(mgr, boid, ahead, nearList);
  }
}

void CIngBlobSwarm::BoidCollidedCallback(CStateManager& mgr, CBoid& boid) {
  KillBoid(boid, mgr, CWeaponMode());
}

void CIngBlobSwarm::BoidCollidedWithPlayerCallback(CStateManager& mgr, CBoid& boid) {
  if (boid.mLaunched) {
    KillBoid(boid, mgr, CWeaponMode());
  } else {
    CSwarmBasics::BoidCollidedWithPlayerCallback(mgr, boid);
  }
}

void CIngBlobSwarm::LaunchBoid(CBoid& boid, const CVector3f& target) {
  // Ballistic arc that peaks mMaxAttackHeight above the higher of the two endpoints.
  const float gravity = kDefaultGravityAccel * mMass;
  const CVector3f start = boid.GetTranslation();
  const float drop = start.GetZ() - target.GetZ();
  const float rise = drop > 0.f ? mMaxAttackHeight : -drop + mMaxAttackHeight;
  const float fall = drop > 0.f ? drop + mMaxAttackHeight : mMaxAttackHeight;
  const float time = CMath::SqrtF(2.f * rise / gravity) + CMath::SqrtF(2.f * fall / gravity);
  const float inverseTime = 1.f / time;
  boid.mVelocity =
      CVector3f((target.GetX() - start.GetX()) * inverseTime,
                (target.GetY() - start.GetY()) * inverseTime, -drop / time + 0.5f * gravity * time);
  boid.mLaunched = true;
  boid.mRemainingLaunchNotOnSurfaceFrames = 5;
}

void CIngBlobSwarm::KillBoid(CBoid& boid, CStateManager& mgr, const CWeaponMode& weapon) {
  CSwarmBasics::KillBoid(boid, mgr, weapon);
  if (boid.mAttackSlot != -1) {
    mAttackSlotInUse[boid.mAttackSlot] = false;
    boid.mAttackSlot = -1;
    boid.mAttacking = false;
    boid.mLaunched = false;
  }
}

CEntity* LoadIngBlobSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrIngBlobSwarm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrIngBlobSwarm.inc"

  const CAnimRes animRes(sldrThis.animationInformation.ancs,
                         sldrThis.animationInformation.character_index, CVector3f::One(),
                         sldrThis.animationInformation.initial_anim, true);
  sldrThis.editorProperties.active = sldrThis.active;
  return rs_new CIngBlobSwarm(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.editorProperties.transform.scale,
      LdrToTransform4f(sldrThis.editorProperties), animRes,
      LdrToActorParameters(sldrThis.actorInformation),
      LdrToBasicSwarmData(sldrThis.basicSwarmProperties), sldrThis.intoAttackAnimation,
      sldrThis.attackAnimation, sldrThis.maxAttackAngle, sldrThis.intoAttackSpeed,
      sldrThis.attackSpeed, sldrThis.mass, sldrThis.maxAttackHeight, sldrThis.attackAimOffset);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SIngBlobSwarm_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadIngBlobSwarm;
  SetSIngBlobSwarm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSIngBlobSwarm_FuncPtrs(nullptr); }
#endif
