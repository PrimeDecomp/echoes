#include "MetroidPrime/ScriptObjects/CScriptTriggerEllipsoid.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTriggerEllipsoid.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

#include <float.h>

CScriptTriggerEllipsoid::~CScriptTriggerEllipsoid() {}

CEntity* LoadTriggerEllipsoid(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrTriggerEllipsoid sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrTriggerEllipsoid.inc"

  const CVector3f forceField =
      mgr.GetWorld()->GetAreaAlways(info.GetAreaId()).GetTM().Rotate(sldrThis.trigger.forceField);
  const CVector3f scale = sldrThis.editorProperties.transform.scale;
  if (scale.GetX() > FLT_EPSILON && scale.GetY() > FLT_EPSILON && scale.GetZ() > FLT_EPSILON) {
    sldrThis.editorProperties.transform.scale = CVector3f(1.f, 1.f, 1.f);
    return rs_new CScriptTriggerEllipsoid(
        mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
        LdrToEntityInfo(info, sldrThis.editorProperties), scale,
        LdrToTransform4f(sldrThis.editorProperties), LdrToDamageInfo(sldrThis.trigger.damage),
        forceField, sldrThis.trigger.flagsTrigger, sldrThis.deactivateOnEnter,
        sldrThis.deactivateOnExit, CScriptTriggerEllipsoid::kST_Ellipsoid);
  }
  return nullptr;
}

bool CScriptTriggerEllipsoid::IsPointInside(const CVector3f& point) const {
  switch (mShape) {
  case kST_Cylinder: {
    const CVector3f diff = point - GetTranslation();
    float distanceSq = 0.f;
    distanceSq += diff.GetX() * diff.GetX() + diff.GetY() * diff.GetY();
    if (distanceSq < mScale.GetX() * mScale.GetX() && fabsf(diff.GetZ()) < mScale.GetZ()) {
      return true;
    }
    break;
  }
  case kST_Ellipsoid: {
    const CVector3f local = GetTransform().TransposeRotate(point - GetTranslation());
    const CVector3f scaled(local.GetX() * mInverseScale.GetX(), local.GetY() * mInverseScale.GetY(),
                           local.GetZ() * mInverseScale.GetZ());
    if (scaled.MagSquared() <= 1.f) {
      return true;
    }
    break;
  }
  }
  return false;
}

void CScriptTriggerEllipsoid::SetScale(const CVector3f& scale) {
  mScale = scale;
  mInverseScale = CVector3f(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
  mWorldBounds = CalculateBounds(GetTransform(), scale);
}

CAABox CScriptTriggerEllipsoid::CalculateBounds(const CTransform4f& xf, const CVector3f& scale) {
  const CAABox box(-scale, scale);
  return box.GetTransformedAABox(xf);
}

bool CScriptTriggerEllipsoid::BoundsOverlap(const CAABox& bounds) const {
  const CVector3f origin = GetTranslation();
  const CVector3f scaledMin =
      GetTransform().TransposeRotate(bounds.GetMinPoint() - origin) * mInverseScale;
  const CVector3f scaledMax =
      GetTransform().TransposeRotate(bounds.GetMaxPoint() - origin) * mInverseScale;

  float distanceSq = 0.f;
  if (scaledMin.GetX() > 0.f) {
    distanceSq += scaledMin.GetX() * scaledMin.GetX();
  } else if (scaledMax.GetX() < 0.f) {
    distanceSq += scaledMax.GetX() * scaledMax.GetX();
  }
  if (scaledMin.GetY() > 0.f) {
    distanceSq += scaledMin.GetY() * scaledMin.GetY();
  } else if (scaledMax.GetY() < 0.f) {
    distanceSq += scaledMax.GetY() * scaledMax.GetY();
  }

  if (mShape == kST_Cylinder) {
    return distanceSq < 1.f && scaledMax.GetZ() > -1.f && scaledMin.GetZ() < 1.f;
  }

  if (scaledMin.GetZ() > 0.f) {
    distanceSq += scaledMin.GetZ() * scaledMin.GetZ();
  } else if (scaledMax.GetZ() < 0.f) {
    distanceSq += scaledMax.GetZ() * scaledMax.GetZ();
  }
  return distanceSq < 1.f;
}

void CScriptTriggerEllipsoid::AddToRenderer(const CStateManager&) const {}

void CScriptTriggerEllipsoid::Render(const CStateManager&) const {}

rstl::optional_object< CAABox > CScriptTriggerEllipsoid::GetTouchBounds() const {
  return rstl::optional_object< CAABox >(mWorldBounds);
}

void CScriptTriggerEllipsoid::Touch(CActor& actor, CStateManager& mgr) {
  const rstl::optional_object< CAABox > actorBounds = actor.GetTouchBounds();
  if (actorBounds && BoundsOverlap(*actorBounds)) {
    CScriptTrigger::Touch(actor, mgr);
  }
}

void CScriptTriggerEllipsoid::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    mWorldBounds = CalculateBounds(GetTransform(), mScale);
    SetTransform(GetTransform());
    SetTransformDirtySpare(false);
  }
  CScriptTrigger::Think(dt, mgr);
}

CScriptTriggerEllipsoid::CScriptTriggerEllipsoid(TUniqueId uid, const rstl::string& name,
                                                 const CEntityInfo& info, const CVector3f& scale,
                                                 const CTransform4f& xf, const CDamageInfo& damage,
                                                 const CVector3f& forceField, uint flags,
                                                 bool deactivateOnEntered, bool deactivateOnExited,
                                                 EShapeType shape)
: CScriptTrigger(uid, name, info, xf.GetTranslation(), CAABox(CVector3f::Zero(), CVector3f::Zero()),
                 damage, forceField, flags, deactivateOnEntered, deactivateOnExited)
, mScale(scale)
, mInverseScale(CVector3f::Zero())
, mWorldBounds(CalculateBounds(xf, scale))
, x1f8_(0.f)
, mShape(shape) {
  mInverseScale = CVector3f(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
  SetTransform(xf);
}
