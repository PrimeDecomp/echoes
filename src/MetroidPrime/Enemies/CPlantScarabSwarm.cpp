#include "MetroidPrime/Enemies/CPlantScarabSwarm.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrPlantScarabSwarm.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/SwarmRenderHelpers.hpp"
#include "MetroidPrime/TGameTypes.hpp"

CPlantScarabSwarm::CPlantScarabSwarm(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
    const CVector3f& boundingBoxExtent, const CTransform4f& xf, const CAnimRes& animRes,
    CActorParameters actorParameters, const CBasicSwarmData& data, int intoAttackAnimation,
    int attackAnimation, float maxAttackAngle, float intoAttackSpeed, float attackSpeed,
    float grenadeMass, float grenadeLaunchSpeed, float grenadeSpeed, CDamageInfo grenadeDamage,
    float grenadeExplodePlayerDistance, uint grenadeBounces, CAssetId grenadeExplosionEffect,
    CAssetId grenadeExplosionXRayEffect, CAssetId grenadeTrailEffect, CAssetId grenadeEffect,
    ushort grenadeBounceSound, ushort grenadeExplosionSound, float grenadeBounceSoundFallOff,
    float grenadeBounceMaxAudibleDistance, float grenadeExplosionSoundFallOff,
    float grenadeExplosionMaxAudibleDistance)
: CSwarmBasics(uid, name, info, boundingBoxExtent, xf, animRes, actorParameters, data, true)
, mAttackerCount(data.mAttackerCount)
, mAttackProximity(data.mAttackProximity)
, mAttackTimer(data.mAttackTimer)
, mMaxAttackAngle(maxAttackAngle * (M_PIF / 180.f))
, mSharedAttackBit(0)
, mAttackModelBitBase(0)
, mIntoAttackSpeed(intoAttackSpeed)
, mAttackSpeed(attackSpeed)
, mGrenadeData(grenadeMass, grenadeSpeed, grenadeDamage, grenadeBounces, grenadeExplosionEffect,
               grenadeExplosionXRayEffect, grenadeTrailEffect, grenadeEffect, grenadeBounceSound,
               grenadeExplosionSound, grenadeBounceSoundFallOff, grenadeBounceMaxAudibleDistance,
               grenadeExplosionSoundFallOff, grenadeExplosionMaxAudibleDistance, true)
, mGrenadeLaunchSpeed(grenadeLaunchSpeed)
, mGrenadeExplodePlayerDistance(grenadeExplodePlayerDistance) {
  if (animRes.GetId() != kInvalidAssetId && intoAttackAnimation != -1) {
    mAttackModels.reserve(mAttackerCount);
    mAttackDeltas.reserve(mAttackerCount);
    mAttackSlotBoids.reserve(mAttackerCount);
    const CAnimRes intoAttackRes(animRes.GetId(), animRes.GetCharacterNodeId(), animRes.GetScale(),
                                 intoAttackAnimation, true);
    mAttackModelBitBase = mModelDatas.size();
    for (int i = 0; i < mAttackerCount; ++i) {
      mAttackModels.push_back_unsafe(CModelData(intoAttackRes));
      mAttackDeltas.push_back_unsafe(
          CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
      mAttackModels[i].AnimationData()->EnableLooping(false);
      mAttackModels[i].AnimationData()->SetIsAnimating(true);
      mAttackSlotBoids.push_back_unsafe(-1);
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

CPlantScarabSwarm::~CPlantScarabSwarm() {}

void CPlantScarabSwarm::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CSwarmBasics::AcceptScriptMsg(mgr, msg);
}

void CPlantScarabSwarm::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CSwarmBasics::Think(dt, mgr);
  if (mAttackerCount != 0) {
    const CVector3f playerPos = mgr.GetPlayer(0)->GetTranslation();
    static CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
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
            mTimeSinceLastAttack >= mAttackTimer && IsSpaceAboveBoidClear(mgr, *it)) {
          const CVector3f forward = it->GetTransform().GetForward();
          if (CMath::AbsF(CVector3f::GetAngleDiff(forward, toPlayer.AsNormalized())) <=
              mMaxAttackAngle) {
            for (uint i = 0; i < mAttackerCount; ++i) {
              if (mAttackSlotBoids[i] == -1) {
                it->mAttacking = true;
                it->mLaunched = true;
                it->mAttackSlot = i;
                mAttackSlotBoids[i] = it->mIndex;
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
  }
  const uint count = mAttackModels.size();
  for (uint i = 0; i < count; ++i) {
    SpawnGrenades(mgr, *mAttackModels[i].AnimationData(), i);
  }
}

void CPlantScarabSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
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

void CPlantScarabSwarm::UpdateSwarmAnimations(CStateManager& mgr, float dt) {
  CSwarmBasics::UpdateSwarmAnimations(mgr, dt);
  const uint count = mAttackModels.size();
  for (uint i = 0; i < count; ++i) {
    if (mAttackSlotBoids[i] != -1) {
      CAnimData* animData = mAttackModels[i].AnimationData();
      animData->SetPlaybackRate(1.f);
      mAttackDeltas[i] = mAttackModels[i].AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
      UpdateEffects(mgr, *animData, mMaxVolume);
    }
  }
  if (mSharedAttackModel.get() != nullptr) {
    CAnimData* animData = mSharedAttackModel->AnimationData();
    animData->SetPlaybackRate(mAttackSpeed);
    *mSharedAttackDeltas = mSharedAttackModel->AdvanceAnimation(dt, mgr, GetCurrentAreaId(), true);
    UpdateEffects(mgr, *animData, mMaxVolume);
  }
}

void CPlantScarabSwarm::UpdateAllBoidMovement(CStateManager& mgr, float dt) {
  const uint count = mBoids.size();
  if (mAnimated) {
    const uint modelCount = mModelDatas.size();
    for (uint i = 0; i < count; ++i) {
      CBoid& boid = mBoids[i];
      if (boid.mAttacking) {
        if (boid.mAttackSlot == -1) {
          MoveBoid(mgr, boid, mSharedAttackDeltas->GetOffsetDelta(), dt);
        } else {
          MoveBoid(mgr, boid, mAttackDeltas[boid.mAttackSlot].GetOffsetDelta(), dt);
        }
      } else {
        MoveBoid(mgr, boid, mAdvancementDeltas[i & (modelCount - 1)].GetOffsetDelta(), dt);
      }
    }
  }
}

void CPlantScarabSwarm::UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt,
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
    if (!mAttackModels[slot].AnimationData()->IsAnimTimeRemaining(dt,
                                                                  rstl::string_l("Whole Body"))) {
      mAttackSlotBoids[slot] = -1;
      boid.mAttackSlot = -1;
    }
  } else if (boid.mAttacking) {
    KillBoid(boid, mgr, CWeaponMode());
  } else {
    CSwarmBasics::UpdateBoid(cache, mgr, dt, boid, partitionIndex);
  }
}

void CPlantScarabSwarm::PreRenderBoid(CBoid* boid, uint* drawMask) {
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

void CPlantScarabSwarm::RenderBoid(CBoid* boid) const {
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

void CPlantScarabSwarm::ApplySteeringBehaviors(
    CStateManager& mgr, CBoid& boid, CVector3f& ahead,
    const rstl::reserved_vector< CBoid*, 50 >& nearList) {
  if (!boid.mAttacking) {
    CSwarmBasics::ApplySteeringBehaviors(mgr, boid, ahead, nearList);
  }
}

void CPlantScarabSwarm::BoidCollidedCallback(CStateManager& mgr, CBoid& boid) {
  KillBoid(boid, mgr, CWeaponMode());
}

void CPlantScarabSwarm::BoidCollidedWithPlayerCallback(CStateManager& mgr, CBoid& boid) {
  if (boid.mLaunched) {
    KillBoid(boid, mgr, CWeaponMode());
  } else {
    CSwarmBasics::BoidCollidedWithPlayerCallback(mgr, boid);
  }
}

void CPlantScarabSwarm::KillBoid(CBoid& boid, CStateManager& mgr, const CWeaponMode& weapon) {
  CSwarmBasics::KillBoid(boid, mgr, weapon);
  if (boid.mAttackSlot != -1) {
    mAttackSlotBoids[boid.mAttackSlot] = -1;
    boid.mAttackSlot = -1;
    boid.mAttacking = false;
    boid.mLaunched = false;
  }
}

void CPlantScarabSwarm::SpawnGrenades(CStateManager& mgr, CAnimData& animData, int slot) {
  const int boidIndex = mAttackSlotBoids[slot];
  if (boidIndex == -1) {
    return;
  }
  int count;
  const CInt32POINode* nodes = animData.GetInt32POIList(count);
  if (count > 0 && nodes != nullptr) {
    for (int i = 0; i < count; ++i) {
      const CInt32POINode& node = nodes[i];
      if (mgr.Random()->Float() <= node.GetWeight()) {
        const int character = node.GetCharacterIndex();
        if ((character == -1 || character == animData.GetCharacterIndex()) &&
            node.GetValue() == kUE_ObjectDrop) {
          const CBoid& boid = mBoids[boidIndex];
          const CVector3f boidPos = boid.GetTranslation();
          const CVector3f aimPos = mgr.GetPlayer(0)->GetAimPosition(mgr, 0.f);
          const CVector3f toBoid = boidPos - aimPos;
          CTransform4f xf = CTransform4f::LookAt(toBoid.AsNormalized(), boid.GetTransform().GetUp(),
                                                 CVector3f::Up());
          xf.SetTranslation(GetBoidTopPosition(boid));
          CBouncyGrenade* grenade = rs_new CBouncyGrenade(
              mgr.AllocateUniqueId(), rstl::string_l("PlantScarabSwarm Grenade"),
              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, true, kInvalidEditorId),
              xf, CModelData::CModelDataNull(), CActorParameters::None(), GetUniqueId(),
              mGrenadeLaunchSpeed, mGrenadeData, mGrenadeExplodePlayerDistance, GetGrenadeBounds(),
              kInvalidUniqueId, 0.f, 0, nullptr, nullptr);
          if (grenade != nullptr) {
            mgr.AddObject(grenade);
            grenade->RemoveMaterial(kMT_Projectile, mgr);
          }
        }
      }
    }
  }
}

CVector3f CPlantScarabSwarm::GetBoidTopPosition(const CBoid& boid) const {
  return boid.GetTranslation() + 0.1f * boid.GetTransform().GetUp();
}

CAABox CPlantScarabSwarm::GetGrenadeBounds() const {
  return CAABox(CVector3f(-0.1f, -0.1f, -0.1f), CVector3f(0.1f, 0.1f, 0.1f));
}

bool CPlantScarabSwarm::IsSpaceAboveBoidClear(const CStateManager& mgr, const CBoid& boid) const {
  const CVector3f top = GetBoidTopPosition(boid);
  const CAABox grenadeBounds = GetGrenadeBounds();
  const CAABox bounds = grenadeBounds.GetTransformedAABox(CTransform4f::Translate(top));
  const CMaterialList materials(kMT_Solid);
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(materials);
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, filter, this);
  const CCollidableAABox primitive(bounds, materials);
  return !CGameCollision::DetectCollisionBoolean(mgr, primitive, CTransform4f::Identity(), filter,
                                                 nearList);
}

CEntity* LoadPlantScarabSwarm(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrPlantScarabSwarm sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrPlantScarabSwarm.inc"

  const CAnimRes animRes(sldrThis.animationInformation.ancs,
                         sldrThis.animationInformation.character_index, CVector3f::One(),
                         sldrThis.animationInformation.initial_anim, true);
  sldrThis.editorProperties.active = sldrThis.active;
  return rs_new CPlantScarabSwarm(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.editorProperties.transform.scale,
      LdrToTransform4f(sldrThis.editorProperties), animRes,
      LdrToActorParameters(sldrThis.actorInformation),
      LdrToBasicSwarmData(sldrThis.basicSwarmProperties), sldrThis.intoAttackAnimation,
      sldrThis.attackAnimation, sldrThis.maxAttackAngle, sldrThis.intoAttackSpeed,
      sldrThis.attackSpeed, sldrThis.grenadeMass, sldrThis.grenadeLaunchSpeed,
      sldrThis.unknown_0xed086ce0, LdrToDamageInfo(sldrThis.grenadeDamage),
      sldrThis.grenadeExplosionProximity, sldrThis.unknown_0x454f16b1,
      sldrThis.grenadeExplosionEffect, sldrThis.grenadeExplosionXRayEffect,
      sldrThis.grenadeTrailEffect, sldrThis.grenadeEffect, sldrThis.grenadeBounceSound,
      sldrThis.grenadeExplosionSound, sldrThis.grenadeBounceSoundFallOff,
      sldrThis.grenadeBounceMaxAudibleDistance, sldrThis.grenadeExplosionSoundFallOff,
      sldrThis.grenadeExplosionMaxAudibleDistance);
}

#ifndef MONOLITHIC
static void SetFuncPtrs() {
  static SPlantScarabSwarm_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &LoadPlantScarabSwarm;
  SetSPlantScarabSwarm_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSPlantScarabSwarm_FuncPtrs(nullptr); }
#endif
