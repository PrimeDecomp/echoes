#include "MetroidPrime/Enemies/CSpankWeed.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/COBBox.hpp"
#include "Collision/CSpatialPrimitive.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpankWeed.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "REL/REL_Setup.h"
#include "rstl/StringExtras.hpp"
#include "rstl/map.hpp"

#include <stdio.h>

void DebugDrawOBB(const COBBox& box, float r, float g, float b, float a);

static const char* const skArmJointNames[] = {
    "Arm_2", "Arm_3", "Arm_4",  "Arm_5",  "Arm_6",  "Arm_7",
    "Arm_8", "Arm_9", "Arm_10", "Arm_11", "Arm_12", "Arm_end",
};

// Guessed class: an unused tentacle collision proxy that tracks the arm joints of the plant.
class CSpankWeedCollisionActor : public CActor {
public:
  CSpankWeedCollisionActor(TUniqueId uid, TAreaId areaId, const CMaterialList& materials,
                           const TUniqueId& ownerId);

  // CEntity
  void Think(float dt, CStateManager& mgr) override { UpdateBounds(mgr); }

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override { return mTouchBounds; }
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  void UpdateBounds(CStateManager& mgr);

  TUniqueId mOwnerId;
  rstl::optional_object< CAABox > mTouchBounds;
  rstl::vector< CVector3f > mArmPoints;
};

CSpankWeedCollisionActor::CSpankWeedCollisionActor(TUniqueId uid, TAreaId areaId,
                                                   const CMaterialList& materials,
                                                   const TUniqueId& ownerId)
: CActor(uid, rstl::string_l("Spank Weed Collision "),
         CEntityInfo(areaId, CEntity::NullConnectionList, true, kInvalidEditorId), 0,
         CTransform4f::Identity(), CModelData::None(), materials, CActorParameters(),
         kInvalidUniqueId)
, mOwnerId(ownerId)
, mTouchBounds()
, mArmPoints() {
  mArmPoints.reserve(12);
  SetCallTouch(false);
}

void CSpankWeedCollisionActor::Touch(CActor& actor, CStateManager& mgr) {
  CActor* touched = TCastToPtr< CActor >(actor);
  if (touched == nullptr) {
    return;
  }

  rstl::vector< CCollidableSphere > spheres;
  spheres.reserve(12);

  const rstl::optional_object< CAABox > bounds = touched->GetTouchBounds();
  bool intersects = false;
  CSpankWeed* owner = TCastToPtr< CSpankWeed >(mgr.ObjectById(mOwnerId));
  if (owner == nullptr) {
    return;
  }

  const CTransform4f& ownerXf = owner->GetTransform();
  for (int i = 0; i < 12; ++i) {
    const CVector3f center = ownerXf * mArmPoints[i];
    if (CollisionUtil::AABoxSphereIntersection(*bounds, CSphere(center, 1.f))) {
      intersects = true;
      break;
    }
  }

  if (intersects) {
    CPlayer* player = TCastToPtr< CPlayer >(actor);
    if (player != nullptr && owner->mCurDamageRemTime <= 0.f && !owner->mHitByPlayerProjectile) {
      mgr.ApplyDamage(
          mOwnerId, player->GetUniqueId(), GetUniqueId(), owner->GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
      owner->mCurDamageRemTime = owner->mDamageWaitTime;
    }
  }
}

void CSpankWeedCollisionActor::UpdateBounds(CStateManager& mgr) {
  mArmPoints.clear();
  CSpankWeed* owner = TCastToPtr< CSpankWeed >(mgr.ObjectById(mOwnerId));
  if (owner == nullptr) {
    return;
  }

  const CAABox ownerBox = owner->GetBoundingBox();
  const float halfWidth = 0.5f * ownerBox.GetWidth();
  const float halfDepth = 0.5f * ownerBox.GetDepth();
  float minX = 0.f;
  float maxX = 0.f;
  float minY = 0.f;
  float maxY = 0.f;
  float minZ = 0.f;
  float maxZ = 0.f;
  for (uint i = 0; i < ARRAY_SIZE(skArmJointNames); ++i) {
    const CTransform4f locator = owner->GetLocatorTransform(rstl::string_l(skArmJointNames[i]));
    const CVector3f point = locator.GetTranslation();
    mArmPoints.push_back_unsafe(point);
    if (point.GetX() < minX) {
      minX = point.GetX();
    } else if (point.GetX() > maxX) {
      maxX = point.GetX();
    }
    if (point.GetY() < minY) {
      minY = point.GetY();
    } else if (point.GetY() > maxY) {
      maxY = point.GetY();
    }
    if (point.GetZ() < minZ) {
      minZ = point.GetZ();
    } else if (point.GetZ() > maxZ) {
      maxZ = point.GetZ();
    }
  }

  const CAABox localBounds(minX - halfWidth, minY, minZ - halfDepth, maxX + halfWidth, maxY,
                           maxZ + halfDepth);
  mTouchBounds =
      rstl::optional_object< CAABox >(localBounds.GetTransformedAABox(owner->GetTransform()));
}

CSpankWeed::CSpankWeed(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& modelData,
                       const CActorParameters& actorParams, const CPatternedInfo& patternedInfo,
                       float maxDetectionRange, float maxHearingRange, float maxSightRange,
                       float hideTime)
: CPatterned(kPAI_SpankWeed, uid, name, kFT_Zero, info, xf, modelData, patternedInfo, kMT_Flyer,
             kCT_One, kBT_Restricted, actorParams)
, mMaxDetectionRange(maxDetectionRange)
, mHeightRange(patternedInfo.GetDetectionHeightRange())
, mMaxHearingRange(maxHearingRange)
, mMaxSightRange(maxSightRange)
, mHideTime(hideTime)
, mCanKnockBack(false)
, x7d8_(0.f)
, mRetreatOrigin(xf.GetTranslation())
, mCollisionActorId(kInvalidUniqueId)
, mCollisionMgr(nullptr)
, mIsHiding(true)
, mLockonOffset(CVector3f::Zero())
, mLockonTarget(CVector3f::Zero())
, mState(-1)
, mPreviousState(-1)
, mAnimPhase(-1) {
  SetCallTouch(false);
  SetDrawShadow(false);

  const CVector3f modelScale = GetModelData()->GetScale();
  if (modelScale.GetX() != modelScale.GetY() || modelScale.GetX() != modelScale.GetZ()) {
    const float scale = modelScale.Magnitude() / CMath::SqrtF(3.f);
    ModelData()->SetScale(CVector3f(scale, scale, scale));

    char buf[1024];
    sprintf(buf,
            "WARNING: Non-uniform scale (%.2f, %.2f, %.2f) applied to Spank Weed...changing scale "
            "to (%.2f, %.2f, %.2f)\n",
            modelScale.GetX(), modelScale.GetY(), modelScale.GetZ(), scale, scale, scale);
  }

  CMaterialList excludeList = GetMaterialFilter().GetExcludeList();
  excludeList.Add(CMaterialList(kMT_Character, kMT_Player));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), excludeList));

  const CSegId lockonId = GetAnimationData()->GetLocatorSegId(rstl::string_l("lockon_target_LCTR"));
  if (lockonId != 0xff) {
    const CTransform4f locatorXf = GetAnimationData()->GetLocatorTransform(lockonId, nullptr);
    const CTransform4f scaledXf =
        GetTransform() * (CTransform4f::Scale(GetModelData()->GetScale()) * locatorXf);
    mLockonTarget = scaledXf.GetTranslation();
    mLockonOffset = scaledXf.GetTranslation() - GetTranslation();
  }

  KnockBackController().EnableKnockBackPhysics(false);
}

void CSpankWeed::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const bool oldActive = GetActive();
  const TUniqueId senderId = msg.GetSenderId();
  switch (msg.GetMessage()) {
  case kSM_XCRT:
    if (!BodyController()->GetIsActive()) {
      BodyController()->Activate(mgr, pas::kAS_Invalid);
      const CAABox box = GetBoundingBox();
      const float halfWidth = 0.5f * box.GetWidth();
      const float halfDepth = 0.5f * box.GetDepth();
      const float halfHeight = 0.5f * box.GetHeight();
      SetBoundingBox(CAABox(-halfWidth, -halfHeight, -halfDepth, halfWidth, halfHeight, halfDepth));
    }
    {
      rstl::vector< CJointCollisionDescription > joints;
      if (HasAnimation() && GetAnimationData()->GetSpatialPrimitive()) {
        const rstl::vector< CSpatialPrimitive::SBox >& boxes =
            (*GetAnimationData()->GetSpatialPrimitive())->GetBoxes();
        const uint count = boxes.size();
        joints.reserve(count);
        for (uint i = 0; i < count; ++i) {
          const CSegId segId = boxes[i].mFirstSegment;
          const COBBox& obb = boxes[i].mBox;
          const rstl::map< rstl::string, CSegId > nameMap =
              GetAnimationData()->GetCharLayoutInfo()->GetNameMap();
          for (rstl::map< rstl::string, CSegId >::const_iterator it = nameMap.begin();
               it != nameMap.end(); ++it) {
          }
          const CJointCollisionDescription desc =
              CJointCollisionDescription::OBBFromMayaPlugInCollision(
                  segId, obb.GetSize(), obb.GetTransform().BuildMatrix3f(),
                  obb.GetTransform().GetTranslation(),
                  rstl::string_l("box") + CStringExtras::CreateFromInteger(i), 0.001f);
          joints.push_back_unsafe(desc);
        }
      }
      mCollisionMgr = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints,
                                                    GetActive());
      CMaterialList materials(kMT_CameraPassthrough, kMT_Immovable);
      mCollisionMgr->AddMaterialList(mgr, materials);
    }
    if (HasActorLights()) {
      ActorLights()->SetNeedsRelight(true);
    }
    {
      const CVector3f bias =
          GetTransform().BuildMatrix3f() *
          GetScaledLocatorTransform(rstl::string_l("swoosh_LCTR")).GetTranslation();
      ActorLights()->SetLightingPositionOffset(bias);
    }
    break;
  case kSM_XHIT: {
    CCollisionActor* collisionActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(senderId));
    if (collisionActor != nullptr) {
      CPlayer* player =
          TCastToPtr< CPlayer >(mgr.ObjectById(collisionActor->GetLastTouchedObject()));
      if (player != nullptr && mCurDamageRemTime <= 0.f && mState != 4 && mState != 6) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
    break;
  }
  case kSM_Delete:
    mgr.DeleteObjectRequest(mCollisionActorId);
    mCollisionMgr->Destroy(mgr);
    break;
  case kSM_Activate:
    if (HasActorLights()) {
      ActorLights()->SetNeedsRelight(true);
    }
    break;
  case kSM_Decrement:
    if (mState != 0 && mState != 5 && mState != 6 && mState != 4) {
      mHitByPlayerProjectile = true;
      mDamageCooldownTimer = mDamageWaitTime;
    }
    break;
  case kSM_AIUpdateDisabled:
    if (mCollisionMgr.get() != nullptr) {
      mCollisionMgr->SetPhysicsActive(mgr, false);
    }
    break;
  }

  CPatterned::AcceptScriptMsg(mgr, msg);
  const bool active = GetActive();
  if (oldActive != active && mCollisionMgr.get() != nullptr) {
    mCollisionMgr->SetActive(mgr, active);
  }
}

void CSpankWeed::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  HealthInfo()->SetHP(1000000.f);
  if (!mIsHiding) {
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f eyeOrigin = GetLocatorTransform(rstl::string_l("Eye")).GetTranslation();
    CVector3f offset = CVector3f::ByElementMultiply(scale, eyeOrigin);
    offset = GetTransform().Rotate(offset);
    MoveCollisionPrimitive(offset);
    SetTransformDirty();
    mCollisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  }
  CPatterned::Think(dt, mgr);
}

void CSpankWeed::KnockBack(CStateManager& mgr, const CKnockBackInfo& info) {
  if (mCanKnockBack) {
    CPatterned::KnockBack(mgr, info);
    mCanKnockBack = false;
  }
}

void CSpankWeed::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mKnockBackController.EnableFreeze(false);
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    RemoveMaterial(kMT_Solid, kMT_Scannable, mgr);
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    mCollisionMgr->SetActive(mgr, false);
    mIsHiding = true;
    mState = 0;
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, kMT_Scannable, mgr);
    SetTranslation(mRetreatOrigin);
    mCollisionMgr->SetActive(mgr, true);
    mIsHiding = false;
    mKnockBackController.EnableFreeze(true);
    mPreviousState = 0;
    break;
  }
}

void CSpankWeed::FadeIn(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimPhase = 0;
    mCanKnockBack = true;
    mState = 5;
    break;
  case kStateMsg_Update:
    switch (mAnimPhase) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mAnimPhase = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mAnimPhase = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 5;
    break;
  }
  SetWorldLightingDirty(true);
}

void CSpankWeed::FadeOut(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimPhase = 0;
    mCanKnockBack = false;
    mState = 6;
    break;
  case kStateMsg_Update:
    switch (mAnimPhase) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mAnimPhase = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mAnimPhase = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 6;
    break;
  }
}

void CSpankWeed::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mKnockBackController.EnableFreeze(true);
    mBodyController->SetLocomotionType(pas::kLT_Lurk);
    RemoveMaterial(kMT_Solid, mgr);
    mState = 1;
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 1;
    break;
  }
}

void CSpankWeed::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->SetLocomotionType(pas::kLT_Combat);
    RemoveMaterial(kMT_Solid, mgr);
    mState = 2;
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 2;
    break;
  }
}

void CSpankWeed::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    mState = 3;
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
      mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 3;
    break;
  }
}

bool CSpankWeed::IsPlayerNear(CStateManager& mgr, float range) const {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CVector3f playerPos = mgr.Player(i)->GetTranslation();
    if (mHeightRange > 0.f &&
        CMath::AbsF(playerPos.GetZ() - GetTranslation().GetZ()) >= mHeightRange) {
      continue;
    }
    if ((playerPos - mLockonTarget).MagSquared() < range * range) {
      return true;
    }
  }
  return false;
}

bool CSpankWeed::InDetectionRange(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerNear(mgr, mMaxDetectionRange);
}

bool CSpankWeed::HearPlayer(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerNear(mgr, mMaxHearingRange);
}

bool CSpankWeed::InRange(CStateManager& mgr, const CTriggerData& data) const {
  return IsPlayerNear(mgr, mMaxSightRange);
}

bool CSpankWeed::Delay(CStateManager& mgr, const CTriggerData& data) const {
  if (mHitByPlayerProjectile) {
    if (mStateMachine->GetTime() > mHideTime) {
      const_cast< CSpankWeed* >(this)->mHitByPlayerProjectile = false;
      return true;
    }
    return false;
  }
  return true;
}

void CSpankWeed::Flinch(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimPhase = 0;
    mState = 4;
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Update:
    switch (mAnimPhase) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
        mAnimPhase = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Zero(), pas::kS_Zero));
      }
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_KnockBack) {
        mAnimPhase = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mPreviousState = 4;
    break;
  }
}

CVector3f CSpankWeed::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f position = CVector3f::Zero();
  if (dt > 0.f) {
    const CMotionState motion = PredictMotion(dt);
    position = motion.GetTranslation();
  }

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CSegId lockonId = animData->GetLocatorSegId(rstl::string_l("lockon_target_LCTR"));
  if (lockonId != 0xff) {
    const CTransform4f locatorXf = animData->GetLocatorTransform(lockonId, nullptr);
    const CVector3f scaledOrigin =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), locatorXf.GetTranslation());
    position += GetTransform() * scaledOrigin;
  } else {
    position += GetBoundingBox().GetCenterPoint();
  }
  return position;
}

CVector3f CSpankWeed::GetOrbitPosition(const CStateManager& mgr) const {
  const CVector3f orbit = CPatterned::GetOrbitPosition(mgr);
  const float time = rstl::min_val(mStateMachine->GetTime(), 1.f);
  const CVector3f target = GetTranslation() + mLockonOffset;
  if (mState == 3 && mPreviousState == 2) {
    return CVector3f::Lerp(orbit, target, time);
  }
  if (mState == 2 && mPreviousState == 3) {
    return CVector3f::Lerp(target, orbit, time);
  }
  return orbit;
}

bool CSpankWeed::AnimOver(CStateManager& mgr, const CTriggerData& data) const {
  return mAnimPhase == 3;
}

bool CSpankWeed::Attacked(CStateManager& mgr, const CTriggerData& data) const {
  return CPatterned::Attacked(mgr, data);
}

void CSpankWeed::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const rstl::optional_object< TLockedToken< CSpatialPrimitive > >& primitive =
      animData->GetSpatialPrimitive();
  if (primitive) {
    const rstl::vector< CSpatialPrimitive::SBox >& boxes = (*primitive)->GetBoxes();
    const CTransform4f& xf = GetTransform();
    for (uint i = 0, count = boxes.size(); i < count; ++i) {
      const CSegId segId = boxes[i].mFirstSegment;
      const COBBox& boxObb = boxes[i].mBox;
      const CTransform4f locatorXf = GetScaledLocatorTransform(segId);
      const COBBox obb(xf * locatorXf * boxObb.GetTransform(), boxObb.GetSize());
      DebugDrawOBB(obb, 1.f, 1.f, 1.f, 1.f);
    }
  }
}

CSpankWeed::~CSpankWeed() {}

CEntity* REL_LoadSpankWeed(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrSpankWeed sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrSpankWeed.inc"

  rstl::optional_object< CModelData > modelData(
      LdrToModelData(sldrThis.editorProperties.transform.scale, kInvalidAssetId,
                     sldrThis.patterned.animationInformation, true));
  if (!modelData) {
    return nullptr;
  }

  return rs_new CSpankWeed(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                           LdrToEntityInfo(info, sldrThis.editorProperties),
                           LdrToTransform4f(sldrThis.editorProperties), *modelData,
                           LdrToActorParameters(sldrThis.actorInformation),
                           LdrToPatternedInfo(sldrThis.patterned, nullptr), sldrThis.wakeUpRadius,
                           sldrThis.searchRadius, sldrThis.attackRadius, sldrThis.hurtSleepDelay);
}

static void SetFuncPtrs() {
  static SSpankWeed_FuncPtrs funcPtrs;
  funcPtrs.mLoader = &REL_LoadSpankWeed;
  SetSSpankWeed_FuncPtrs(&funcPtrs);
}

extern "C" void RELMain() { SetFuncPtrs(); }

extern "C" void RELExit() { SetSSpankWeed_FuncPtrs(nullptr); }
