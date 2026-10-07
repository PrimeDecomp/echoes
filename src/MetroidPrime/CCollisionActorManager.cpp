#include "MetroidPrime/CCollisionActorManager.hpp"

#include "Collision/CMaterialList.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Basics/CCast.hpp"

#include <float.h>
#include <math.h>

// Unreferenced in the GC build; only its dynamic initializer survives.
static TAreaId sUnknownAreaId = kInvalidAreaId;

CJointCollisionDescription
CJointCollisionDescription::SphereCollision(CSegId pivotId, const CVector3f& pivotPoint,
                                            float radius, const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_Sphere, pivotId, CSegId::Invalid(), CVector3f::Zero(),
                                    CMatrix3f::Identity(), pivotPoint, radius, 0.f, kOT_Pivot, name,
                                    mass);
}

CJointCollisionDescription CJointCollisionDescription::SphereSubdivideCollision(
    CSegId pivotId, CSegId nextId, float radius, float maxSeparation,
    EOrientationType orientationType, const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_SphereSubdivide, pivotId, nextId, CVector3f::Zero(),
                                    CMatrix3f::Identity(), CVector3f::Zero(), radius, maxSeparation,
                                    orientationType, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::AABoxCollision(CSegId pivotId,
                                                                      const CVector3f& bounds,
                                                                      const rstl::string& name,
                                                                      float mass) {
  return CJointCollisionDescription(kCT_AABox, pivotId, CSegId::Invalid(), bounds,
                                    CMatrix3f::Identity(), CVector3f::Zero(), 0.f, 0.f, kOT_Pivot,
                                    name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBAutoSizeCollision(
    CSegId pivotId, CSegId nextId, const CVector3f& bounds, EOrientationType orientationType,
    const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_OBBAutoSize, pivotId, nextId, bounds, CMatrix3f::Identity(),
                                    CVector3f::Zero(), 0.f, 0.f, orientationType, name, mass);
}

CJointCollisionDescription CJointCollisionDescription::OBBCollision(CSegId pivotId,
                                                                    const CVector3f& bounds,
                                                                    const CVector3f& pivotPoint,
                                                                    const rstl::string& name,
                                                                    float mass) {
  return CJointCollisionDescription(kCT_OBB, pivotId, CSegId::Invalid(), bounds,
                                    CMatrix3f::Identity(), pivotPoint, 0.f, 0.f, kOT_Pivot, name,
                                    mass);
}

CJointCollisionDescription::CJointCollisionDescription(
    ECollisionType type, CSegId pivotId, CSegId nextId, const CVector3f& bounds,
    const CMatrix3f& orientation, const CVector3f& pivotPoint, float radius, float maxSeparation,
    EOrientationType orientationType, const rstl::string& name, float mass)
: mType(type)
, mOrientationType(orientationType)
, mPivotId(pivotId)
, mNextId(nextId)
, mBounds(bounds)
, mPivotPoint(pivotPoint)
, mRadius(radius)
, mMaxSeparation(maxSeparation)
, mName(name)
, mActorId(kInvalidUniqueId)
, mMass(mass)
, mOrientation(orientation) {}

CJointCollisionDescription CJointCollisionDescription::OBBFromMayaPlugInCollision(
    CSegId pivotId, const CVector3f& bounds, const CMatrix3f& orientation,
    const CVector3f& pivotPoint, const rstl::string& name, float mass) {
  return CJointCollisionDescription(kCT_OBBFromMayaPlugIn, pivotId, CSegId::Invalid(), bounds,
                                    orientation, pivotPoint, 0.f, 0.f, kOT_Pivot, name, mass);
}

void CJointCollisionDescription::ScaleAllBounds(const CVector3f& scale) {
  mBounds = CVector3f::ByElementMultiply(scale, mBounds);
  mRadius *= scale.GetX();
  mMaxSeparation *= scale.GetX();
  mPivotPoint = CVector3f::ByElementMultiply(scale, mPivotPoint);
}

CCollisionActorManager::CCollisionActorManager(
    CStateManager& mgr, TUniqueId owner, TAreaId areaId,
    const rstl::vector< CJointCollisionDescription >& descriptions, bool active)
: mOwnerId(owner), mActive(active), mDestroyed(false), mPhysicsActive(true) {
  const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwnerId));
  if (actor != nullptr) {
    const CAnimData* animData = actor->GetAnimationData();
    const CTransform4f worldXf = actor->GetTransform();
    const CVector3f scale = actor->GetModelData()->GetScale();
    const CTransform4f scaleXf = CTransform4f::Scale(scale);
    mJointDescriptions.reserve(descriptions.size());
    for (rstl::vector< CJointCollisionDescription >::const_iterator it = descriptions.begin();
         it != descriptions.end(); ++it) {
      CJointCollisionDescription desc = *it;
      desc.ScaleAllBounds(scale);
      const CTransform4f pivotXf =
          GetWRLocatorTransform(*animData, desc.GetPivotId(), worldXf, scaleXf);
      if (desc.GetNextId() != CSegId::Invalid()) {
        const CTransform4f nextXf =
            GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
        const float distance = (nextXf.GetTranslation() - pivotXf.GetTranslation()).Magnitude();
        if (desc.GetType() == CJointCollisionDescription::kCT_OBBAutoSize) {
          if (distance <= FLT_EPSILON) {
            continue;
          }
          CVector3f bounds(desc.GetBounds()[kDX], distance + desc.GetBounds()[kDY],
                           desc.GetBounds()[kDZ]);
          TUniqueId uid = mgr.AllocateUniqueId();
          CCollisionActor* const colAct =
              rs_new CCollisionActor(uid, areaId, mOwnerId, bounds,
                                     CVector3f(0.f, 0.5f * distance, 0.f), active, desc.GetMass());
          if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Pivot) {
            colAct->SetTransform(pivotXf);
          } else {
            CVector3f up = pivotXf.GetUp();
            const CVector3f delta =
                (nextXf.GetTranslation() - pivotXf.GetTranslation()).AsNormalized();
            if (CMath::AbsF(1.f - CMath::AbsF(CVector3f::Dot(delta, up))) < 100.f * FLT_EPSILON) {
              up = pivotXf.GetForward();
            }
            const CTransform4f xf = CTransform4f::LookAt(pivotXf.GetTranslation(),
                                                         pivotXf.GetTranslation() + delta, up);
            colAct->SetTransform(xf);
          }
          mgr.AddObject(*colAct);
          mJointDescriptions.push_back_unsafe(*it);
          (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
        } else {
          TUniqueId uid = mgr.AllocateUniqueId();
          CCollisionActor* const colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, active,
                                                                 desc.GetRadius(), desc.GetMass());
          colAct->SetTransform(pivotXf);
          mgr.AddObject(*colAct);
          mJointDescriptions.push_back_unsafe(CJointCollisionDescription::SphereCollision(
              desc.GetPivotId(), CVector3f::Zero(), desc.GetRadius(), desc.GetName(), 0.001f));
          (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
          const uint numSeps = CCast::ToUint32(distance / desc.GetMaxSeparation());
          if (numSeps != 0) {
            mJointDescriptions.reserve(mJointDescriptions.capacity() + numSeps);
            const float pitch = distance / float(numSeps + 1);
            for (uint i = 0; i < numSeps; ++i) {
              const float separation = pitch * float(i + 1);
              mJointDescriptions.push_back_unsafe(
                  CJointCollisionDescription::SphereSubdivideCollision(
                      desc.GetPivotId(), desc.GetNextId(), desc.GetRadius(), separation,
                      CJointCollisionDescription::kOT_BetweenJoints, desc.GetName(), 0.001f));
              TUniqueId newId = mgr.AllocateUniqueId();
              CCollisionActor* const newAct = rs_new CCollisionActor(
                  newId, areaId, mOwnerId, active, desc.GetRadius(), desc.GetMass());
              if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Pivot) {
                const CTransform4f xf = CTransform4f::Translate(pivotXf.GetTranslation() +
                                                                separation * pivotXf.GetForward());
                newAct->SetTransform(xf);
              } else {
                CVector3f up = pivotXf.GetUp();
                const CVector3f delta =
                    (nextXf.GetTranslation() - pivotXf.GetTranslation()).AsNormalized();
                if (CMath::AbsF(1.f - CMath::AbsF(CVector3f::Dot(delta, up))) <
                    100.f * FLT_EPSILON) {
                  up = pivotXf.GetForward();
                }
                const CTransform4f xf = CTransform4f::Translate(
                    pivotXf.GetTranslation() +
                    separation * CTransform4f::LookAt(CVector3f::Zero(), delta, up).GetForward());
                newAct->SetTransform(xf);
              }
              mgr.AddObject(*newAct);
              (mJointDescriptions.end() - 1)->SetCollisionActorId(newId);
            }
          }
        }
      } else {
        TUniqueId uid = mgr.AllocateUniqueId();
        CCollisionActor* colAct = nullptr;
        if (desc.GetType() == CJointCollisionDescription::kCT_Sphere) {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, active, desc.GetRadius(),
                                          desc.GetMass());
        } else if (desc.GetType() == CJointCollisionDescription::kCT_OBB) {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, desc.GetBounds(),
                                          desc.GetPivotPoint(), active, desc.GetMass());
        } else if (desc.GetType() == CJointCollisionDescription::kCT_OBBFromMayaPlugIn) {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, 0.5f * desc.GetBounds(),
                                          CVector3f::Zero(), active, desc.GetMass());
        } else {
          colAct = rs_new CCollisionActor(uid, areaId, mOwnerId, desc.GetBounds(), active,
                                          desc.GetMass());
        }
        colAct->SetTransform(pivotXf);
        if (desc.GetType() == CJointCollisionDescription::kCT_Sphere) {
          colAct->SetTranslation(pivotXf.GetTranslation() +
                                 pivotXf.BuildMatrix3f() * desc.GetPivotPoint());
        } else if (desc.GetType() == CJointCollisionDescription::kCT_OBBFromMayaPlugIn) {
          CTransform4f locatorXf = animData->GetLocatorTransform(desc.GetPivotId(), nullptr);
          locatorXf.SetTranslation(CVector3f::ByElementMultiply(actor->GetModelData()->GetScale(),
                                                                locatorXf.GetTranslation()));
          const CTransform4f orientXf = CTransform4f(desc.GetOrientation(), desc.GetPivotPoint());
          colAct->SetTransform(worldXf * locatorXf * orientXf);
        } else {
          colAct->SetTranslation(pivotXf.GetTranslation());
        }
        mgr.AddObject(*colAct);
        mJointDescriptions.push_back_unsafe(*it);
        (mJointDescriptions.end() - 1)->SetCollisionActorId(uid);
      }
    }
  }
}

CCollisionActorManager::~CCollisionActorManager() {}

void CCollisionActorManager::Update(float dt, CStateManager& mgr, EUpdateOptions options) {
  if (!mPhysicsActive) {
    SetPhysicsActive(mgr, true);
  }
  if (mActive) {
    const CActor* const act = TCastToConstPtr< CActor >(mgr.GetObjectById(mOwnerId));
    if (act != nullptr) {
      const CAnimData* animData = act->GetAnimationData();
      const CTransform4f worldXf = act->GetTransform();
      const CTransform4f scaleXf = CTransform4f::Scale(act->GetModelData()->GetScale());
      for (int i = 0; i < mJointDescriptions.size(); ++i) {
        const CJointCollisionDescription& desc = mJointDescriptions[i];
        CCollisionActor* const colAct =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()));
        if (colAct != nullptr) {
          const CTransform4f pivotXf =
              GetWRLocatorTransform(*animData, desc.GetPivotId(), worldXf, scaleXf);
          CVector3f origin = pivotXf.GetTranslation();
          if (desc.GetType() == CJointCollisionDescription::kCT_OBB ||
              desc.GetType() == CJointCollisionDescription::kCT_OBBAutoSize) {
            if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Pivot) {
              colAct->SetRotation(CQuaternion::FromMatrix(pivotXf));
            } else {
              const CTransform4f nextXf =
                  GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
              colAct->SetRotation(CQuaternion::FromMatrix(CTransform4f::LookAt(
                  pivotXf.GetTranslation(), nextXf.GetTranslation(), pivotXf.GetUp())));
            }
          } else if (desc.GetType() == CJointCollisionDescription::kCT_SphereSubdivide) {
            if (desc.GetOrientationType() == CJointCollisionDescription::kOT_Pivot) {
              origin += desc.GetMaxSeparation() * pivotXf.GetForward();
            } else {
              const CTransform4f nextXf =
                  GetWRLocatorTransform(*animData, desc.GetNextId(), worldXf, scaleXf);
              origin += CTransform4f::LookAt(origin, nextXf.GetTranslation(), pivotXf.GetUp())
                            .GetForward() *
                        desc.GetMaxSeparation();
            }
          }
          if (options == kUO_ObjectSpace) {
            const CVector3f movement = colAct->GetTransform().TransposeMultiply(origin);
            colAct->MoveToOR(movement, dt);
          } else if (desc.GetType() == CJointCollisionDescription::kCT_Sphere) {
            colAct->SetTranslation(pivotXf.GetTranslation() +
                                   pivotXf.BuildMatrix3f() * desc.GetPivotPoint());
          } else if (desc.GetType() == CJointCollisionDescription::kCT_OBBFromMayaPlugIn) {
            CTransform4f locatorXf = animData->GetLocatorTransform(desc.GetPivotId(), nullptr);
            locatorXf.SetTranslation(CVector3f::ByElementMultiply(act->GetModelData()->GetScale(),
                                                                  locatorXf.GetTranslation()));
            const CTransform4f orientXf = CTransform4f(desc.GetOrientation(), desc.GetPivotPoint());
            const CTransform4f xf = worldXf * locatorXf * orientXf;
            colAct->SetTransform(xf);
          } else {
            colAct->SetTranslation(pivotXf.GetTranslation());
          }
        }
      }
    }
  }
}

void CCollisionActorManager::Destroy(CStateManager& mgr) const {
  for (int i = 0; i < mJointDescriptions.size(); ++i)
    mgr.DeleteObjectRequest(mJointDescriptions[i].GetCollisionActorId());
  mDestroyed = true;
}

void CCollisionActorManager::SetActive(CStateManager& mgr, bool active) {
  mActive = active;
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CEntity* entity = mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId());
    if (entity != nullptr && entity->GetActive() != active) {
      entity->SetActive(active);
      if (active)
        Update(0.f, mgr, kUO_WorldSpace);
    }
  }
}

void CCollisionActorManager::AddMaterialList(CStateManager& mgr, const CMaterialList& materials) {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CActor* actor =
        TCastToPtr< CActor >(mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()));
    if (actor != nullptr)
      actor->AddMaterial(materials);
  }
}

void CCollisionActorManager::RemoveMaterialList(CStateManager& mgr,
                                                const CMaterialList& materials) {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CActor* actor =
        TCastToPtr< CActor >(mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()));
    if (actor != nullptr)
      actor->MaterialList().Remove(materials);
  }
}

uint CCollisionActorManager::GetNumCollisionActors() const { return mJointDescriptions.size(); }

const CJointCollisionDescription&
CCollisionActorManager::GetCollisionDescFromIndex(uint index) const {
  return mJointDescriptions[index];
}

int CCollisionActorManager::GetCollisionDescIndexFromUniqueId(TUniqueId id) const {
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    if (mJointDescriptions[i].GetCollisionActorId() == id)
      return i;
  }
  return -1;
}

CTransform4f CCollisionActorManager::GetWRLocatorTransform(const CAnimData& animData, CSegId id,
                                                           const CTransform4f& worldXf,
                                                           const CTransform4f& scaleXf) {
  CTransform4f locatorXf = animData.GetLocatorTransform(id, nullptr);
  const CVector3f origin = worldXf * (scaleXf * locatorXf.GetTranslation());
  locatorXf = worldXf.MultiplyIgnoreTranslation(locatorXf);
  locatorXf.SetTranslation(origin);
  return locatorXf;
}

void CCollisionActorManager::SetPhysicsActive(CStateManager& mgr, bool active) {
  if (active == mPhysicsActive)
    return;
  mPhysicsActive = active;
  for (int i = 0; i < mJointDescriptions.size(); ++i) {
    CCollisionActor* actor =
        TCastToPtr< CCollisionActor >(mgr.ObjectById(mJointDescriptions[i].GetCollisionActorId()));
    if (actor != nullptr) {
      actor->SetMovable(mPhysicsActive);
      actor->SetUseInSortedLists(mPhysicsActive);
    }
  }
}
