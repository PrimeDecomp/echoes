#include "MetroidPrime/CCollisionActor.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CCollidableOBBTreeGroup.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"

static const CMaterialList kCollisionActorMaterials(kMT_Unknown59, kMT_CollisionActor,
                                                    kMT_ScanPassthrough, kMT_CameraPassthrough);
static const EScriptObjectMessage kSM_XDMG = static_cast< EScriptObjectMessage >(0x58444d47);
static const EScriptObjectMessage kSM_XRDG = static_cast< EScriptObjectMessage >(0x58524447);

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                 const CVector3f& extent, const CVector3f& center, bool active,
                                 float mass)
: CPhysicsActor(uid, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList, active), 0, CTransform4f::Identity(),
                CModelData::CModelDataNull(), kCollisionActorMaterials, CAABox::Identity(),
                SMoverData(mass), CActorParameters(), StepData(0.f, 0.f, 0))
, mPrimitiveType(kPT_OBBTreeGroup)
, mOwner(owner)
, mBoxSize(extent)
, mCenter(center)
, mObbContainer(rs_new COBBTreeGroup(extent, center))
, mObbTreeGroupPrimitive(rs_new CCollidableOBBTreeGroup(mObbContainer.get(), GetMaterialList()))
, mAaboxPrimitive(nullptr)
, mSpherePrimitive(nullptr)
, mSphereRadius(0.f)
, mHealthInfo(0.f, 0.f)
, mDamageVulnerability(CDamageVulnerability::NormalVulnerabilty())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner,
                                 const CVector3f& boxSize, bool active, float mass)
: CPhysicsActor(uid, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList, active), 0, CTransform4f::Identity(),
                CModelData::CModelDataNull(), kCollisionActorMaterials, CAABox::Identity(),
                SMoverData(mass), CActorParameters(), StepData(0.f, 0.f, 0))
, mPrimitiveType(kPT_AABox)
, mOwner(owner)
, mBoxSize(boxSize)
, mCenter(CVector3f::Zero())
, mObbContainer(nullptr)
, mObbTreeGroupPrimitive(nullptr)
, mAaboxPrimitive(rs_new CCollidableAABox(CAABox(-0.5f * mBoxSize, 0.5f * mBoxSize),
                                          CMaterialList(kMT_Unknown59, kMT_NoStaticCollision)))
, mSpherePrimitive(nullptr)
, mSphereRadius(0.f)
, mHealthInfo(0.f, 0.f)
, mDamageVulnerability(CDamageVulnerability::NormalVulnerabilty())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

CCollisionActor::CCollisionActor(TUniqueId uid, TAreaId areaId, TUniqueId owner, bool active,
                                 float radius, float mass)
: CPhysicsActor(uid, rstl::string_l("CollisionActor"),
                CEntityInfo(areaId, NullConnectionList, active), 0, CTransform4f::Identity(),
                CModelData::CModelDataNull(), kCollisionActorMaterials, CAABox::Identity(),
                SMoverData(mass), CActorParameters(), StepData(0.f, 0.f, 0))
, mPrimitiveType(kPT_Sphere)
, mOwner(owner)
, mBoxSize(CVector3f::Zero())
, mCenter(CVector3f::Zero())
, mObbContainer(nullptr)
, mObbTreeGroupPrimitive(nullptr)
, mAaboxPrimitive(nullptr)
, mSpherePrimitive(rs_new CCollidableSphere(CSphere(CVector3f::Zero(), radius),
                                            CMaterialList(kMT_Unknown59, kMT_NoStaticCollision)))
, mSphereRadius(radius)
, mHealthInfo(0.f, 0.f)
, mDamageVulnerability(CDamageVulnerability::NormalVulnerabilty())
, mLastTouched(kInvalidUniqueId)
, mResponseType(kWCR_EnemyNormal)
, mExtendedTouchBounds(CVector3f::Zero()) {
  SetCoefficientOfRestitutionModifier(0.5f);
  SetCallTouch(false);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Unknown59), CMaterialList(kMT_CollisionActor, kMT_NoStaticCollision)));
}

rstl::optional_object< CAABox > CCollisionActor::GetTouchBounds() const {
  rstl::optional_object< CAABox > bounds;
  if (mPrimitiveType == kPT_OBBTreeGroup) {
    bounds = mObbTreeGroupPrimitive->CalculateAABox(GetTransform());
  } else if (mPrimitiveType == kPT_AABox) {
    bounds = mAaboxPrimitive->CalculateAABox(GetTransform());
  } else {
    bounds = mSpherePrimitive->CalculateAABox(GetTransform());
  }

  bounds->AccumulateBounds(bounds->GetMaxPoint() + mExtendedTouchBounds);
  bounds->AccumulateBounds(bounds->GetMinPoint() - mExtendedTouchBounds);
  return bounds;
}

CVector3f CCollisionActor::GetOrbitPosition(const CStateManager&) const {
  return GetTouchBounds()->GetCenterPoint();
}

void CCollisionActor::Touch(CActor& actor, CStateManager& mgr) {
  mLastTouched = actor.GetUniqueId();
  mgr.DeliverScriptMsg(
      CScriptMsg(GetUniqueId(), actor.GetUniqueId(), mOwner, kSM_XHIT, kSS_InvalidState));
}

const CCollisionPrimitive* CCollisionActor::GetCollisionPrimitive() const {
  if (mPrimitiveType == kPT_OBBTreeGroup)
    return mObbTreeGroupPrimitive.get();
  if (mPrimitiveType == kPT_AABox)
    return mAaboxPrimitive.get();
  return mSpherePrimitive.get();
}

CTransform4f CCollisionActor::GetPrimitiveTransform() const {
  CTransform4f xf = GetTransform();
  xf.SetTranslation(CPhysicsActor::GetPrimitiveTransform().GetTranslation());
  return xf;
}

void CCollisionActor::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  const EScriptObjectMessage message = msg.GetMessage();
  const TUniqueId sender = msg.GetUnk();
  switch (message) {
  case kSM_XDelete:
    if (mNonUniformVulnerability)
      mNonUniformVulnerability.reset();
    break;
  case kSM_XHIT:
  case kSM_XXDG:
  case kSM_XDMG:
  case kSM_XRDG:
    if (CEntity* owner = mgr.ObjectById(mOwner)) {
      mLastTouched = sender;
      mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), kInvalidUniqueId, owner->GetUniqueId(),
                                      message, kSS_InvalidState));
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CCollisionActor::OnScanStateChange(EScanState state, CStateManager& mgr) {
  if (CActor* owner = TCastToPtr< CActor >(mgr.ObjectById(mOwner)))
    owner->OnScanStateChange(state, mgr);
  CActor::OnScanStateChange(state, mgr);
}

CHealthInfo* CCollisionActor::HealthInfo() { return &mHealthInfo; }

const CDamageVulnerability* CCollisionActor::GetDamageVulnerability() const {
  return &mDamageVulnerability;
}

const CDamageVulnerability*
CCollisionActor::GetDamageVulnerability(const CVector3f& point, const CVector3f& direction,
                                        const CDamageInfo& damage) const {
  if (mNonUniformVulnerability)
    return mNonUniformVulnerability->GetDamageVulnerability(GetDamageVulnerability(), point,
                                                            direction, damage);
  return GetDamageVulnerability();
}

EWeaponCollisionResponseTypes CCollisionActor::GetCollisionResponseType(const CVector3f& point,
                                                                        const CVector3f& direction,
                                                                        const CWeaponMode& mode,
                                                                        int attributes) const {
  if (mNonUniformVulnerability) {
    EWeaponCollisionResponseTypes response = mResponseType;
    if (mNonUniformVulnerability->GetCollisionResponseType(point, direction, mode, attributes,
                                                           response) == true)
      return response;
  }
  return mResponseType;
}

void CCollisionActor::SetDamageVulnerability(const CDamageVulnerability& vulnerability) {
  mDamageVulnerability = vulnerability;
}

void CCollisionActor::SetNonUniformVulnerability(
    rstl::ncrc_ptr< CNonUniformVulnerability >& vulnerability) {
  mNonUniformVulnerability = vulnerability;
}

void CCollisionActor::ResetNonUniformVulnerability() {
  if (mNonUniformVulnerability)
    mNonUniformVulnerability.reset();
}

TUniqueId CCollisionActor::GetLastTouchedObject() const { return mLastTouched; }

float CCollisionActor::GetSphereRadius() const { return mSphereRadius; }

void CCollisionActor::SetSphereRadius(float radius) {
  if (mPrimitiveType == kPT_Sphere) {
    mSphereRadius = radius;
    mSpherePrimitive->SetSphere(CSphere(mSpherePrimitive->GetSphere().GetCenter(), radius));
  }
}
